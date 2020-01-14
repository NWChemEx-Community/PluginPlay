#include "sde/printing/detail_/print_submodules.hpp"
#include "tests/test_common.hpp"
#include <catch2/catch.hpp>
#include <tests/test_common.hpp>
#include <utilities/printing/word_wrap_stream.hpp>

using namespace sde::printing::detail_;

TEST_CASE("submod_table") {
    sde::type::submodule_map submods;

    SECTION("No submods") {
        auto corr = "+-----+---------------+-------------+\n"
                    "| Key | Property Type | Description |\n"
                    "+-----+---------------+-------------+";
        REQUIRE(submod_table(submods) == corr);
    }
    SECTION("Empty Submod") {
        submods["Submod1"];
        auto corr = "+---------+---------------+-------------+\n"
                    "| Key     | Property Type | Description |\n"
                    "+=========+===============+=============+\n"
                    "| Submod1 | N/A           | N/A         |\n"
                    "+---------+---------------+-------------+";
        REQUIRE(submod_table(submods) == corr);
    }
    SECTION("Has description") {
        submods["Submod1"].set_description("Hello World");
        auto corr = "+---------+---------------+-------------+\n"
                    "| Key     | Property Type | Description |\n"
                    "+=========+===============+=============+\n"
                    "| Submod1 | N/A           | Hello World |\n"
                    "+---------+---------------+-------------+";
        REQUIRE(submod_table(submods) == corr);
    }
    SECTION("Has property type") {
        submods["Submod1"].set_type<testing::NullPT>();
        auto corr = "+---------+-----------------+-------------+\n"
                    "| Key     | Property Type   | Description |\n"
                    "+=========+=================+=============+\n"
                    "| Submod1 | testing::NullPT | N/A         |\n"
                    "+---------+-----------------+-------------+";
        REQUIRE(submod_table(submods) == corr);
    }
    SECTION("The whole shebang") {
        submods["Submod1"].set_type<testing::NullPT>().set_description(
          "Hello World");
        auto corr = "+---------+-----------------+-------------+\n"
                    "| Key     | Property Type   | Description |\n"
                    "+=========+=================+=============+\n"
                    "| Submod1 | testing::NullPT | Hello World |\n"
                    "+---------+-----------------+-------------+";
        REQUIRE(submod_table(submods) == corr);
    }
}

TEST_CASE("print_submods") {
    sde::type::submodule_map submods;
    std::stringstream ss;
    utilities::printing::WordWrapStream w(&ss);
    reSTPrinter p(w);
    SECTION("No submods") {
        auto corr = "##########\n"
                    "Submodules\n"
                    "##########\n"
                    "\n"
                    "The module defines no submodules.\n\n";
        print_submods(p, submods);
        REQUIRE(ss.str() == corr);
    }
    SECTION("Submodules") {
        submods["Submod1"].set_type<testing::NullPT>().set_description(
          "Hello World");
        submods["Sumbod2"].set_type<testing::OneIn>().set_description(
          "Bye World");
        print_submods(p, submods);
        auto corr = "##########\n"
                    "Submodules\n"
                    "##########\n"
                    "\n"
                    "This section details the full list of submodules that the "
                    "module uses. For each\n"
                    "submodule we have listed:\n"
                    "\n"
                    "- Key : The key used to refer to this particular "
                    "callback.\n"
                    "- Property Type : The property type that the submodule "
                    "must satisfy.\n"
                    "- Description : How the module will use the submodule.\n"
                    "\n"
                    "+---------+-----------------+-------------+\n"
                    "| Key     | Property Type   | Description |\n"
                    "+=========+=================+=============+\n"
                    "| Submod1 | testing::NullPT | Hello World |\n"
                    "+---------+-----------------+-------------+\n"
                    "| Sumbod2 | testing::OneIn  | Bye World   |\n"
                    "+---------+-----------------+-------------+";
        REQUIRE(ss.str() == corr);
    }
}
