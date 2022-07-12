/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/utf8.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the program version string.
 *
 * Used by help nodes to produce their output, though in principle can be used by anything that
 * wants to.
 * @tparam S compile_time_string
 */
template <typename S>
class program_version_t
{
public:
    /** String type. */
    using string_type = S;

    /** Returns the program version.
     *
     * @return Program version
     */
    [[nodiscard]] constexpr static std::string_view program_version() noexcept { return S::get(); }

private:
    static_assert(utility::utf8::num_code_points(program_version()) > 1,
                  "Program version string must be longer than one character");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto program_version = program_version_t<S>{};

template <typename S>
struct is_policy<program_version_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
