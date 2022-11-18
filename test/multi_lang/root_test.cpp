// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

namespace arg_router::multi_lang
{
template <>
class translation<AR_STRING("en_GB")>
{
public:
    using hello = AR_STRING("hello");
    using hello_description = AR_STRING("Hello description");
    using help = AR_STRING("help");
    using help_description = AR_STRING("Display help");
};

template <>
class translation<AR_STRING("fr")>
{
public:
    using hello = AR_STRING("bonjour");
    using hello_description = AR_STRING("Bonjour descriptif");
    using help = AR_STRING("aider");
    using help_description = AR_STRING("Afficher l'aide");
};

template <>
class translation<AR_STRING("es")>
{
public:
    using hello = AR_STRING("hola");
    using hello_description = AR_STRING("Hola descripción");
    using help = AR_STRING("ayuda");
    using help_description = AR_STRING("Mostrar ayuda");
};
}  // namespace arg_router::multi_lang

BOOST_AUTO_TEST_SUITE(multi_lang_suite)

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto f = [](auto lang, auto args, auto parse_result, std::string exception_message) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root<AR_STRING("en_GB"), AR_STRING("fr"), AR_STRING("es")>(
            lang,
            [&](auto tr_) {
                using tr = decltype(tr_);

                return root(mode(arg<int>(policy::long_name<typename tr::hello>,
                                          policy::required,
                                          policy::description<typename tr::hello_description>),
                                 policy::router{[&](auto value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                            policy::validation::default_validator);
            });

        try {
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(exception_message.empty());
            BOOST_REQUIRE(!!result);
            BOOST_CHECK_EQUAL(*result, parse_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), exception_message);
        }
    };

    test::data_set(
        f,
        {
            // English
            std::tuple{"en_GB", std::vector{"foo", "--hello", "42"}, 42, ""},
            std::tuple{"en_GB",
                       std::vector{"foo", "--bonjour", "42"},
                       42,
                       "Unknown argument: --bonjour"},

            // French
            std::tuple{"fr", std::vector{"foo", "--bonjour", "42"}, 42, ""},
            std::tuple{"fr", std::vector{"foo", "--hello", "42"}, 42, "Unknown argument: --hello"},

            // Spanish
            std::tuple{"es", std::vector{"foo", "--hola", "42"}, 42, ""},
            std::tuple{"es", std::vector{"foo", "--hello", "42"}, 42, "Unknown argument: --hello"},
        });
}

BOOST_AUTO_TEST_CASE(default_parse_test)
{
    for (auto input : {"da", "en-us", "POSIX", "*", "C", ""}) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root<AR_STRING("en_GB"), AR_STRING("fr"), AR_STRING("es")>(
            input,
            [&](auto tr_) {
                using tr = decltype(tr_);

                return root(mode(arg<int>(policy::long_name<typename tr::hello>,
                                          policy::required,
                                          policy::description<typename tr::hello_description>),
                                 policy::router{[&](auto value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                            policy::validation::default_validator);
            });

        auto args = std::vector{"foo", "--hello", "42"};
        r.parse(args.size(), const_cast<char**>(args.data()));
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, 42);
    }
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](auto input, auto expected_output) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root<AR_STRING("en_GB"), AR_STRING("fr"), AR_STRING("es")>(
            input,
            [&](auto tr_) {
                using tr = decltype(tr_);

                return root(help(policy::long_name<typename tr::help>,
                                 policy::short_name<'h'>,
                                 policy::description<typename tr::help_description>,
                                 policy::program_name<AR_STRING("foo")>,
                                 policy::program_version<AR_STRING("v3.14")>,
                                 policy::program_intro<AR_STRING("Fooooooo")>),
                            mode(arg<int>(policy::long_name<typename tr::hello>,
                                          policy::required,
                                          policy::description<typename tr::hello_description>),
                                 policy::router{[&](auto value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                            policy::validation::default_validator);
            });

        auto stream = std::ostringstream{};
        r.help(stream);

        BOOST_CHECK_EQUAL(stream.str(), expected_output);
    };

    test::data_set(f,
                   {
                       std::tuple{"en_GB",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --help,-h              Display help\n"
                                  "        --hello <Value>    Hello description\n"},
                       std::tuple{"fr",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --aider,-h               Afficher l'aide\n"
                                  "        --bonjour <Value>    Bonjour descriptif\n"},
                       std::tuple{"es",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --ayuda,-h            Mostrar ayuda\n"
                                  "        --hola <Value>    Hola descripción\n"},
                       std::tuple{"en-us",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --help,-h              Display help\n"
                                  "        --hello <Value>    Hello description\n"},
                   });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto r = multi_lang::root<AR_STRING("en_GB")>("en_GB", [&]([[maybe_unused]] auto tr_) {
        return root(
            mode(arg<int>(
                     policy::long_name<AR_STRING("hello")>,
                     policy::required,
                     policy::description<AR_STRING("Hello description")>),
                 policy::router{[&]([[maybe_unused]] auto value) {}}),
            policy::validation::default_validator);
    });
    return 0;
}
    )",
            "Must be more than one language supported",
            "must_be_more_than_one_language_provided_test"},
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

namespace arg_router::multi_lang
{
template <>
class translation<AR_STRING("en_GB")> {};
}

using namespace arg_router;

int main() {
    const auto r = multi_lang::root<AR_STRING("en_GB"), AR_STRING("en_GB")>(
        "en_GB", [&]([[maybe_unused]] auto tr_) {
        return root(
            mode(arg<int>(
                     policy::long_name<AR_STRING("hello")>,
                     policy::required,
                     policy::description<AR_STRING("Hello description")>),
                 policy::router{[&]([[maybe_unused]] auto value) {}}),
            policy::validation::default_validator);
    });
    return 0;
}
    )",
            "Supported languages must be unique",
            "unique_iso_codes1_test"},
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto r = multi_lang::root<AR_STRING("fr"), AR_STRING("en_GB"), AR_STRING("en_GB")>(
        "en_GB",
        [&]([[maybe_unused]] auto tr_) {
            return root(
                mode(arg<int>(
                         policy::long_name<AR_STRING("hello")>,
                         policy::required,
                         policy::description<AR_STRING("Hello description")>),
                     policy::router{[&]([[maybe_unused]] auto value) {}}),
                policy::validation::default_validator);
        });
    return 0;
}
    )",
            "Supported languages must be unique",
            "unique_iso_codes2_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
