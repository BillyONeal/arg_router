/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/token_type.hpp"

#include <boost/mp11/algorithm.hpp>

#include <type_traits>

namespace arg_router
{
/** Policy namespace.
 *
 * arg_router uses policies to compose behaviours on command line argument
 * types.
 */
namespace policy
{
/** Evaluates to true if @a T is a policy.
 *
 * Unfortunately due to the way that policies form an is-a relationship with
 * their owners, we can't tag a type as a policy via inheritance.  So all
 * policies must create a specialisation of this and manually mark themselves as
 * a policy.
 * @tparam T Type to test
 */
template <typename T, typename... Args>
struct is_policy : std::false_type {
};

/** Helper alias variable for is_policy.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr auto is_policy_v = is_policy<T>::value;

/** Evaluates to true if @a Tuple only contains policy types.
 *
 * @tparam Tuple Tuple of types to test
 */
template <typename Tuple>
struct is_all_policies : boost::mp11::mp_all_of<Tuple, is_policy> {
};

/** Helper alias variable for is_all_policies.
 *
 * @tparam Tuple Tuple of types to test
 */
template <typename Tuple>
constexpr auto is_all_policies_v = is_all_policies<Tuple>::value;

/** Determine if a policy has a <TT>pre_parse_phase</TT> method.
 *
 * @tparam T Policy type to query
 */
template <typename T>
struct has_pre_parse_phase_method : T {
    static_assert(policy::is_policy_v<T>, "T must be a policy");

    template <typename U>
    using type = decltype(  //
        std::declval<const has_pre_parse_phase_method&>()
            .template pre_parse_phase<>(std::declval<parsing::token_list&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_pre_parse_phase_method.
 *
 * @tparam T Policy type to query
 */
template <typename T>
constexpr static bool has_pre_parse_phase_method_v =
    has_pre_parse_phase_method<T>::value;

/** Determine if a policy has a <TT>parse_phase</TT> method.
 *
 * @tparam T Policy type to query
 * @tparam ValueType Parsed type
 */
template <typename T, typename ValueType>
struct has_parse_phase_method : T {
    static_assert(policy::is_policy_v<T>, "T must be a policy");

    template <typename U>
    using type = decltype(  //
        std::declval<const has_parse_phase_method&>()
            .template parse_phase<ValueType>(std::declval<std::string_view>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_parse_phase_method.
 *
 * @tparam T Policy type to query
 * @tparam ValueType Parsed type
 */
template <typename T, typename ValueType>
constexpr static bool has_parse_phase_method_v =
    has_parse_phase_method<T, ValueType>::value;

/** Determine if a policy has a <TT>validation_phase</TT> method.
 *
 * @tparam T Policy type to query
 * @tparam ValueType Parsed type
 */
template <typename T, typename ValueType>
struct has_validation_phase_method : T {
    static_assert(policy::is_policy_v<T>, "T must be a policy");

    template <typename U>
    using type = decltype(  //
        std::declval<const has_validation_phase_method&>()
            .template validation_phase<ValueType>(
                std::declval<const ValueType&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_validation_phase_method.
 *
 * @tparam T Policy type to query
 * @tparam ValueType Parsed type
 */
template <typename T, typename ValueType>
constexpr static bool has_validation_phase_method_v =
    has_validation_phase_method<T, ValueType>::value;

/** Determine if a policy has a <TT>routing_phase</TT> method.
 *
 * @tparam T Policy type to query
 */
template <typename T>
struct has_routing_phase_method : T {
    static_assert(policy::is_policy_v<T>, "T must be a policy");

    template <typename U>
    using type = decltype(  //
        std::declval<const has_routing_phase_method&>()
            .template routing_phase<>(
                std::declval<const parsing::token_list&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_routing_phase_method.
 *
 * @tparam T Policy type to query
 */
template <typename T>
constexpr static bool has_routing_phase_method_v =
    has_routing_phase_method<T>::value;

/** Determine if a policy has a <TT>missing_phase</TT> method.
 *
 * @tparam T Policy type to query
 * @tparam ValueType Parsed type
 */
template <typename T, typename ValueType>
struct has_missing_phase_method : T {
    static_assert(policy::is_policy_v<T>, "T must be a policy");

    template <typename U>
    using type = decltype(  //
        std::declval<const has_missing_phase_method&>()
            .template missing_phase<ValueType>());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_missing_phase_method.
 *
 * @tparam T Policy type to query
 * @tparam ValueType Parsed type
 */
template <typename T, typename ValueType>
constexpr static bool has_missing_phase_method_v =
    has_missing_phase_method<T, ValueType>::value;
}  // namespace policy
}  // namespace arg_router
