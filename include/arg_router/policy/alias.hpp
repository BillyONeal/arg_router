#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/span.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/core/ignore_unused.hpp>
#include <boost/mp11/bind.hpp>

namespace arg_router
{
namespace policy
{
/** Allows the 'aliasing' of arguments, i.e. a single argument will set multiple
 * others.
 *
 * @note An aliased argument cannot be routed, it's aliased arguments are set
 * instead
 * @tparam AliasedPolicies Pack of policies to alias
 */
template <typename... AliasedPolicies>
class alias_t : public no_result_value
{
public:
    /** Tuple of policy types. */
    using aliased_policies_type = std::tuple<AliasedPolicies...>;

private:
    template <typename T>
    struct policy_checker {
        constexpr static auto value = traits::has_long_name_method_v<T> ||
                                      traits::has_short_name_method_v<T>;
    };

    static_assert((sizeof...(AliasedPolicies) > 0),
                  "At least one name needed for alias");
    static_assert(policy::is_all_policies_v<aliased_policies_type>,
                  "All parameters must be policies");
    static_assert(
        boost::mp11::mp_all_of<aliased_policies_type, policy_checker>::value,
        "All parameters must provide a long and/or short form name");

    // You could do this a single inline mp11 expression, but it would be
    // unreadable...
    template <typename Node>
    struct node_match {
        constexpr static auto value = boost::mp11::mp_any_of<
            aliased_policies_type,
            boost::mp11::mp_bind<boost::mp11::mp_contains,
                                 typename Node::policies_type,
                                 boost::mp11::_1>::template fn>::value;
    };

    template <typename ModeType>
    constexpr static auto create_aliased_node_indices()
    {
        using children_type = typename ModeType::children_type;

        // Zip together an index-sequence of ModeType's children and the
        // elements of it
        using zipped = algorithm::zip_t<
            boost::mp11::mp_iota_c<std::tuple_size_v<children_type>>,
            children_type>;

        // Filter out the elements that have a long or short name that match
        // our alias
        using filtered = boost::mp11::mp_filter<
            boost::mp11::mp_bind<
                node_match,
                boost::mp11::mp_bind<boost::mp11::mp_second,
                                     boost::mp11::_1>>::template fn,
            zipped>;

        return typename algorithm::unzip<filtered>::first_type{};
    }

public:
    /** Given a mode-like parent, iterate over its children and return the
     * indices for those that are aliased by this policy.
     *
     * @tparam ModeType Mode-like type
     */
    template <typename ModeType>
    using aliased_node_indices =
        decltype(create_aliased_node_indices<ModeType>());

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit alias_t(AliasedPolicies&&... policies)
    {
        boost::ignore_unused(policies...);
    }

protected:
    /** Duplicates any value tokens as aliases of other nodes.
     * 
     * The owning type must have minimum_count() and maximum_count() methods
     * that return the same value, or a count() method(),  to use an alias.
     * - If the count is zero then it is flag-like so the aliased names are
     *   just inserted into @a tokens after the current token
     * - If the count is greater than zero then it is argument-like and the
     *   aliased names are inserted, each followed by @em count tokens
     *   (i.e. the value), after the current @em count tokens
     *
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list as received from the owning node
     * @param view The tokens to be used in the remaining parse phases, may be
     * empty
     * @param parents Parents instances pack
     * @exception parse_exception Thrown if @a view does not have enough tokens
     * in for the required value count
     */
    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        boost::ignore_unused(parents...);

        static_assert(sizeof...(Parents) >= 2,
                      "Alias requires at least 2 parents");

        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;
        using mode_type = boost::mp11::mp_second<std::tuple<Parents...>>;
        using aliased_indices = aliased_node_indices<mode_type>;

        // Check that the alias names do not appear in the owning node (i.e.
        // no self-references)
        {
            using owner_policies = typename node_type::policies_type;

            using policy_intersection =
                boost::mp11::mp_set_intersection<aliased_policies_type,
                                                 owner_policies>;
            static_assert(std::tuple_size_v<policy_intersection> == 0,
                          "Alias names cannot appear in owner");
        }

        static_assert(traits::has_minimum_count_method_v<node_type> &&
                          traits::has_maximum_count_method_v<node_type>,
                      "Node requires a count(), or minimum_count() and "
                      "maximum_count() methods to use an alias");
        static_assert(node_type::minimum_count() == node_type::maximum_count(),
                      "Node requires minimum_count() and maximum_count() "
                      "to return the same value to use an alias");

        auto new_tokens = parsing::token_list{};
        if constexpr (node_type::minimum_count() == 0) {
            new_tokens.reserve(std::tuple_size_v<aliased_indices>);

            utility::tuple_iterator(
                [&](auto /*i*/, auto index) {
                    using alias_node_type =
                        std::tuple_element_t<index,
                                             typename mode_type::children_type>;
                    new_tokens.push_back(
                        parsing::node_token_type<alias_node_type>());
                },
                aliased_indices{});
        } else {
            // The first token is the argument name, so skip that
            if (node_type::minimum_count() > view.size()) {
                throw parse_exception{
                    "Too few values for alias, needs " +
                        std::to_string(node_type::minimum_count()),
                    parsing::node_token_type<node_type>()};
            }

            new_tokens.reserve(std::tuple_size_v<aliased_indices> *
                               node_type::minimum_count());
            utility::tuple_iterator(
                [&](auto /*i*/, auto index) {
                    using alias_node_type =
                        std::tuple_element_t<index,
                                             typename mode_type::children_type>;
                    new_tokens.push_back(
                        parsing::node_token_type<alias_node_type>());
                    for (auto i = 0u; i < node_type::minimum_count(); ++i) {
                        new_tokens.push_back(view[i]);
                    }
                },
                aliased_indices{});
        }

        tokens.insert(tokens.begin() + node_type::minimum_count(),
                      new_tokens.begin(),
                      new_tokens.end());

        // Update the view in case the extra tokens have caused a reallocation
        view = utility::span<const parsing::token_type>{tokens.data(),  //
                                                        view.size()};
    }
};

/** Constructs a alias_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam AliasedPolicies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Alias instance
 */
template <typename... AliasedPolicies>
constexpr alias_t<AliasedPolicies...> alias(AliasedPolicies... policies)
{
    return alias_t{std::move(policies)...};
}

template <typename... AliasedPolicies>
struct is_policy<alias_t<AliasedPolicies...>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
