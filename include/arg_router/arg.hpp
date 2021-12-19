#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Represents an argument on the command line that has a value that needs
 * parsing.
 *
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class arg_t :
    public tree_node<policy::count_t<std::integral_constant<std::size_t, 1>>,
                     Policies...>
{
    static_assert(policy::is_all_policies_v<std::tuple<Policies...>>,
                  "Args must only contain policies (not other nodes)");

    using parent_type =
        tree_node<policy::count_t<std::integral_constant<std::size_t, 1>>,
                  Policies...>;

    static_assert(traits::has_long_name_method_v<arg_t> ||
                      traits::has_short_name_method_v<arg_t>,
                  "Arg must have a long and/or short name policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = T;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit arg_t(Policies... policies) :
        parent_type{policy::count<1>, std::move(policies)...}
    {
    }

    /** Returns true and calls @a visitor if @a token matches the name of this
     * node.
     * 
     * @a visitor needs to be equivalent to:
     * @code
     * [](const auto& node) { ... }
     * @endcode
     * <TT>node</TT> will be a reference to this node.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @return Match result
     */
    template <typename Fn>
    bool match(const parsing::token_type& token, const Fn& visitor) const
    {
        if (parsing::default_match<arg_t>(token)) {
            visitor(*this);
            return true;
        }

        return false;
    }

    /** Parse function.
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    value_type parse(parsing::token_list& tokens,
                     const Parents&... parents) const
    {
        // Check we have enough tokens to even do a parse
        if (tokens.size() <= parent_type::count()) {
            throw parse_exception{"Missing argument", tokens.front()};
        }

        // Remove this node's name
        tokens.erase(tokens.begin());

        auto view = utility::span<parsing::token_type>{tokens};

        // Pre-parse
        utility::tuple_type_iterator<policies_type>([&](auto /*i*/, auto ptr) {
            using policy_type = std::remove_pointer_t<decltype(ptr)>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type,
                                                               arg_t,
                                                               Parents...>) {
                this->policy_type::pre_parse_phase(tokens,
                                                   view,
                                                   *this,
                                                   parents...);
            }
        });

        // Parse the value token, we don't need to loop on view here as we know
        // there is only one token
        auto result = parent_type::template parse<value_type>(view.front().name,
                                                              *this,
                                                              parents...);

        // Pop the token, we don't need it anymore
        tokens.erase(tokens.begin());

        // Validation
        utility::tuple_type_iterator<policies_type>([&](auto /*i*/, auto ptr) {
            using policy_type = std::remove_pointer_t<decltype(ptr)>;
            if constexpr (policy::has_validation_phase_method_v<policy_type,
                                                                value_type,
                                                                arg_t,
                                                                Parents...>) {
                this->policy_type::validation_phase(result, *this, parents...);
            }
        });

        // Routing
        using routing_policy = typename parent_type::template phase_finder<
            policy::has_routing_phase_method,
            value_type>::type;
        if constexpr (!std::is_void_v<routing_policy>) {
            this->routing_policy::routing_phase(tokens, std::move(result));
        }

        return result;
    }
};

/** Constructs an arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or
 * none, and unfortunately in our case we need @a T to be explicitly set by the
 * user whilst @a Policies need to be deduced.
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
constexpr arg_t<T, Policies...> arg(Policies... policies)
{
    return arg_t<T, std::decay_t<Policies>...>{std::move(policies)...};
}
}  // namespace arg_router
