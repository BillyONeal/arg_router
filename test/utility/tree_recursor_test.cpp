#include "arg_router/utility/tree_recursor.hpp"
#include "arg_router/node_category.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

namespace
{
struct test_Fn {
    template <typename Current, typename... Parents>
    constexpr static void fn()
    {
        using parents_type = std::tuple<Parents...>;

        if constexpr (std::is_same_v<
                          Current,
                          std::decay_t<decltype(policy::validation::
                                                    default_validator)>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<root_t<
                        std::decay_t<
                            decltype(policy::validation::default_validator)>,
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>>,
                        flag_t<policy::description_t<S_("test2")>,
                               policy::short_name_t<
                                   traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 policy::description_t<S_("test1")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>>,
                        root_t<std::decay_t<decltype(policy::validation::
                                                         default_validator)>,
                               flag_t<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>>,
                               flag_t<policy::description_t<S_("test2")>,
                                      policy::short_name_t<
                                          traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<Current,
                                            policy::long_name_t<S_("test")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>>,
                        root_t<std::decay_t<decltype(policy::validation::
                                                         default_validator)>,
                               flag_t<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>>,
                               flag_t<policy::description_t<S_("test2")>,
                                      policy::short_name_t<
                                          traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 policy::description_t<S_("test2")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag_t<policy::description_t<S_("test2")>,
                               policy::short_name_t<
                                   traits::integral_constant<'a'>>>,
                        root_t<std::decay_t<decltype(policy::validation::
                                                         default_validator)>,
                               flag_t<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>>,
                               flag_t<policy::description_t<S_("test2")>,
                                      policy::short_name_t<
                                          traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 policy::short_name_t<
                                     traits::integral_constant<'a'>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag_t<policy::description_t<S_("test2")>,
                               policy::short_name_t<
                                   traits::integral_constant<'a'>>>,
                        root_t<std::decay_t<decltype(policy::validation::
                                                         default_validator)>,
                               flag_t<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>>,
                               flag_t<policy::description_t<S_("test2")>,
                                      policy::short_name_t<
                                          traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 flag_t<policy::description_t<S_("test1")>,
                                        policy::long_name_t<S_("test")>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<root_t<
                        std::decay_t<
                            decltype(policy::validation::default_validator)>,
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>>,
                        flag_t<policy::description_t<S_("test2")>,
                               policy::short_name_t<
                                   traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 flag_t<policy::description_t<S_("test2")>,
                                        policy::short_name_t<
                                            traits::integral_constant<'a'>>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<root_t<
                        std::decay_t<
                            decltype(policy::validation::default_validator)>,
                        flag_t<policy::description_t<S_("test1")>,
                               policy::long_name_t<S_("test")>>,
                        flag_t<policy::description_t<S_("test2")>,
                               policy::short_name_t<
                                   traits::integral_constant<'a'>>>>>>);
        } else if constexpr (
            std::is_same_v<
                Current,
                root_t<std::decay_t<
                           decltype(policy::validation::default_validator)>,
                       flag_t<policy::description_t<S_("test1")>,
                              policy::long_name_t<S_("test")>>,
                       flag_t<policy::description_t<S_("test2")>,
                              policy::short_name_t<
                                  traits::integral_constant<'a'>>>>>) {
            static_assert(std::is_same_v<parents_type, std::tuple<>>);
        }
    }
};

struct skip_test_fn {
    template <typename Current, typename... Parents>
    constexpr static void fn()
    {
        static_assert(
            !std::is_same_v<Current, policy::description_t<S_("test2")>>,
            "Fail");
    }
};

struct skip_Fn {
    template <typename Current, typename...>
    constexpr static bool fn()
    {
        return node_category::is_generic_mode_like_v<Current>;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_CASE(tree_recursor_test)
{
    using Root =
        root_t<std::decay_t<decltype(policy::validation::default_validator)>,
               flag_t<policy::description_t<S_("test1")>,
                      policy::long_name_t<S_("test")>>,
               flag_t<policy::description_t<S_("test2")>,
                      policy::short_name_t<traits::integral_constant<'a'>>>>;

    utility::tree_recursor<test_Fn, Root>();
}

BOOST_AUTO_TEST_CASE(tree_recursor_skip_test)
{
    using Root = root_t<
        std::decay_t<decltype(policy::validation::default_validator)>,
        flag_t<policy::description_t<S_("test1")>,
               policy::long_name_t<S_("test")>>,
        arg_router::mode_t<  //
            flag_t<policy::description_t<S_("test2")>,
                   policy::short_name_t<traits::integral_constant<'a'>>>>>;

    utility::tree_recursor<skip_test_fn, skip_Fn, Root>();
}

BOOST_AUTO_TEST_SUITE_END()
