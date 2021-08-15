#include "pluginplay/printing/detail_/rest_printer.hpp"
#include <catch2/catch.hpp>
#include <sstream>
using namespace pluginplay::printing::detail_;

TEST_CASE("reSTPrinter(ss)") {
    std::stringstream ss;
    std::stringstream corr;
    reSTPrinter p(ss, "*=", "* ");
    REQUIRE(ss.str() == corr.str());

    SECTION("Start a section") {
        REQUIRE_THROWS_AS(p.finish_section(), std::runtime_error);
        p.start_section("a section");
        corr << "*********" << std::endl
             << "a section" << std::endl
             << "*********" << std::endl;
        REQUIRE(ss.str() == corr.str());

        SECTION("Start subsection") {
            p.start_section("another section");
            REQUIRE_THROWS_AS(p.start_section(""), std::runtime_error);
            corr << "another section" << std::endl
                 << "===============" << std::endl;
            REQUIRE(ss.str() == corr.str());
            p.finish_section();

            p.start_section("same level section");
            corr << "same level section" << std::endl
                 << "==================" << std::endl;
            REQUIRE(ss.str() == corr.str());

            SECTION("Stop subsection and outer section") {
                p.finish_section();
                p.finish_section();
                p.start_section("a third section");
                corr << "***************" << std::endl
                     << "a third section" << std::endl
                     << "***************" << std::endl;
            }
        }
    }
}

TEST_CASE("reSTPrinter : operator<<") {
    std::stringstream ss;
    std::stringstream corr;
    reSTPrinter p(ss);
    const auto sen = "This is a short sentence under 80 characters long";
    p << sen;
    corr << sen;
    REQUIRE(ss.str() == corr.str());
}

TEST_CASE("reSTPrinter : print_verbatim") {
    std::stringstream ss;
    std::stringstream corr;
    reSTPrinter p(ss);
    const auto sen = "This is a sentence longer than 80 characters, "
                     "but should still be printed on one line.";

    p.print_verbatim(sen);
    corr << sen;

    REQUIRE(ss.str() == corr.str());
}
