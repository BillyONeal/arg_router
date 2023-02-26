// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

namespace ar = arg_router;
namespace arp = ar::policy;
using namespace ar::literals;

int main(int argc, char* argv[])
{
    ar::root(arp::validation::default_validator,
             ar::help(arp::long_name_t{"help"_S},
                      arp::short_name_t{"h"_S},
                      arp::program_name_t{"just-cats"_S},
                      arp::program_intro_t{"Prints cats!"_S},
                      arp::program_addendum_t{"An example program for arg_router."_S},
                      arp::description_t{"Display this help and exit"_S}),
             ar::flag(arp::long_name_t{"cat"_S},
                      arp::description_t{"English cat"_S},
                      arp::router{[](bool) { std::cout << "cat" << std::endl; }}),
             ar::flag(arp::short_name_t{"猫"_S},
                      arp::description_t{"日本語の猫"_S},
                      arp::router{[](bool) { std::cout << "猫" << std::endl; }}),
             ar::flag(arp::short_name_t{"🐱"_S},
                      arp::description_t{"Emoji cat"_S},
                      arp::router{[](bool) { std::cout << "🐱" << std::endl; }}),
             ar::flag(arp::long_name_t{"แมว"_S},
                      arp::description_t{"แมวไทย"_S},
                      arp::router{[](bool) { std::cout << "แมว" << std::endl; }}),
             ar::flag(arp::long_name_t{"кіт"_S},
                      arp::description_t{"український кіт"_S},
                      arp::router{[](bool) { std::cout << "кіт" << std::endl; }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
