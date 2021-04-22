#include <catch2/catch.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <map>
#include <sde/detail_/sde_any.hpp>
#include <string>
#include <typeindex>
#include <utility> //std::pair
#include <vector>

using namespace sde::detail_;

TEST_CASE("SDEAny : Default ctor") {
    SDEAny a;
    REQUIRE_FALSE(a.has_value());
    REQUIRE(a.type() == typeid(void));
    REQUIRE(sde::hash_objects(a) == "cbc357ccb763df2852fee8c4fc7d55f2");
    REQUIRE_THROWS_AS(a.cast<double>(), std::bad_any_cast);
    REQUIRE(a.str() == "<empty SDEAny>");
}

TEST_CASE("SDEAny : Value ctor") {
    SECTION("POD by value") {
        SDEAny a(int{3});
        REQUIRE(a.has_value());
        REQUIRE(a.type() == typeid(int));
        REQUIRE(sde::hash_objects(a) == "9a4294b64e60cc012c5ed48db4cd9c48");
        REQUIRE(a.cast<int>() == 3);
        REQUIRE(a.str() == "3");
    }
    SECTION("POD by reference") {
        int x{3};
        SDEAny a(x);
        REQUIRE(a.has_value());
        REQUIRE(a.type() == typeid(int));
        REQUIRE(sde::hash_objects(a) == "9a4294b64e60cc012c5ed48db4cd9c48");
        REQUIRE(a.cast<int>() == 3);
        REQUIRE(a.str() == "3");
    }
    SECTION("POD by const reference") {
        const int x{3};
        SDEAny a(x);
        REQUIRE(a.has_value());
        REQUIRE(a.type() == typeid(const int));
        REQUIRE(sde::hash_objects(a) == "9a4294b64e60cc012c5ed48db4cd9c48");
        REQUIRE(a.cast<int>() == 3);
        REQUIRE(a.str() == "3");
    }
    SECTION("Non-POD by move") {
        std::vector<int> x{1, 2, 3, 4};
        auto px = x.data();
        SDEAny a(std::move(x));
        REQUIRE(a.has_value());
        REQUIRE(a.type() == typeid(std::vector<int>));
        REQUIRE(sde::hash_objects(a) == "ad06a09d17cceb43c8d7f0283f889ef6");
        REQUIRE(a.cast<std::vector<int>&>().data() == px);
        REQUIRE(a.str() == "[1, 2, 3, 4]");
    }
}

TEST_CASE("SDEAny : Comparisons") {
    SDEAny a(int{3});
    SECTION("Two empty anys") {
        SDEAny a2, a3;
        REQUIRE(a2 == a3);
        REQUIRE_FALSE(a2 != a3);
    }
    SECTION("Same any") {
        SDEAny a2(int{3});
        REQUIRE(a == a2);
        REQUIRE_FALSE(a != a2);
    }
    SECTION("Different full-ness") {
        SDEAny a2;
        REQUIRE_FALSE(a == a2);
        REQUIRE(a != a2);
    }
    SECTION("Different values") {
        SDEAny a2(int{4});
        REQUIRE_FALSE(a == a2);
        REQUIRE(a != a2);
    }
    SECTION("Different types") {
        SDEAny a2(std::vector<int>{3, 4, 5});
        REQUIRE_FALSE(a == a2);
        REQUIRE(a != a2);
    }
}

TEST_CASE("SDEAny : emplace") {
    std::vector<int> x{1, 2, 3, 4};
    SDEAny a(x), a2;
    a2.emplace<std::vector<int>>(x);
    REQUIRE(a == a2);
}

TEST_CASE("SDEAny : reset") {
    SDEAny a(int{3});
    a.reset();
    REQUIRE(a == SDEAny{});
}

TEST_CASE("SDEAny : swap") {
    SDEAny a(int{3}), a2;
    a.swap(a2);
    REQUIRE(a == SDEAny{});
    REQUIRE(a2 == SDEAny{int{3}});
}

TEST_CASE("SDEAny : cast") {
    SECTION("non-const SDEAny") {
        SDEAny a(int{3});
        REQUIRE(a.cast<int>() == 3);
        REQUIRE(a.cast<int&>() == 3);
        REQUIRE(a.cast<const int&>() == 3);
        REQUIRE_THROWS_AS(a.cast<double>(), std::bad_any_cast);
    }
    SECTION("const SDEAny") {
        const SDEAny a(int{3});
        REQUIRE(a.cast<int>() == 3);
        // a.cast<int&>() //Should trip a static assert if uncommented
        REQUIRE(a.cast<const int&>() == 3);
        REQUIRE_THROWS_AS(a.cast<double>(), std::bad_any_cast);
    }
}

TEST_CASE("SDEAny : Copy ctor") {
    SDEAny a(int{3});
    SDEAny a2(a);
    REQUIRE(a == a2);
}

TEST_CASE("SDEAny : Copy assignment") {
    SDEAny a(int{3});
    SDEAny a2;
    auto pa2 = &(a2 = a);
    REQUIRE(a == a2);
    REQUIRE(pa2 == &a2);
}

TEST_CASE("SDEAny : Move ctor") {
    SDEAny a(int{3});
    SDEAny a2(a);
    SDEAny a3(std::move(a));
    REQUIRE(a2 == a3);
}

TEST_CASE("SDEAny : Move assignment") {
    SDEAny a(int{3});
    SDEAny a2(a);
    SDEAny a3;
    auto pa3 = &(a3 = std::move(a));
    REQUIRE(a2 == a3);
    REQUIRE(pa3 == &a3);
}

TEST_CASE("SDEAny : is_convertible") {
    SDEAny a{int{3}};
    REQUIRE(a.is_convertible<int>());
    REQUIRE_FALSE(a.is_convertible<double>());
}

TEST_CASE("SDEAnyCast") {
    SDEAny a{int{3}};
    REQUIRE(&a.cast<int&>() == &SDEAnyCast<int&>(a));
}

TEST_CASE("make_SDEAny") {
    SDEAny a{int{3}};
    REQUIRE(a == make_SDEAny<int>(3));
}

TEMPLATE_TEST_CASE(
  "SDEAny serialization", "[SDEAny][serialization]",
  (std::pair<cereal::JSONOutputArchive, cereal::JSONInputArchive>),
  (std::pair<cereal::BinaryOutputArchive, cereal::BinaryInputArchive>)) {
    std::vector<int> v{3, 1, 4}, v2{};
    SDEAny sai{int{33}};
    SDEAny sad{double{33.}};
    SDEAny sas{std::string{"thirtythree"}};
    SDEAny sav{v};
    // Type and size of the object needs to be known to avoid seg fault during
    // deserialization.
    SDEAny sai2{int{}}, sad2{double{}}, sas2{std::string{}}, sav2{v2};
    std::stringstream ss;
    {
        typename TestType::first_type oarchive(ss);
        oarchive(sai, sad, sas, sav);
    }
    {
        typename TestType::second_type iarchive(ss);
        iarchive(sai2, sad2, sas2, sav2);
    }
    REQUIRE(sai == sai2);
    REQUIRE(sad == sad2);
    REQUIRE(sas == sas2);
    REQUIRE(sav == sav2);
}