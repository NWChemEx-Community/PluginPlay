#include "pluginplay/detail_/any_result_wrapper.hpp"
#include <catch2/catch.hpp>
#include <sstream>

using namespace pluginplay::detail_;
using utilities::printing::operator<<;

template<typename corr_t, typename T>
inline static void compare_value(T&& w, corr_t corr) {
    SECTION("Type") { REQUIRE(w.type() == typeid(corr_t)); }

    SECTION("get by reference") {
        auto& value = w.template cast<corr_t&>();
        REQUIRE(value == corr);
    }

    SECTION("get by const reference") {
        auto& value = w.template cast<const corr_t&>();
        REQUIRE(value == corr);
    }

    SECTION("get by value") {
        auto value = w.template cast<corr_t>();
        REQUIRE(value == corr);
    }

    SECTION("Throws if wrong type") {
        REQUIRE_THROWS_AS(w.template cast<std::string>(), std::bad_any_cast);
    }

    SECTION("clone") {
        auto w2 = w.clone();
        REQUIRE(w == *w2);
    }

    SECTION("str") {
        std::stringstream ss;
        using utilities::printing::operator<<;
        ss << corr;
        REQUIRE(ss.str() == w.str());
    }

    SECTION("hashing") {
        REQUIRE(pluginplay::hash_objects(corr) == pluginplay::hash_objects(w));
    }
}

TEST_CASE("AnyResultWrapper<POD>(value)") {
    using three_type = decltype(3);
    AnyResultWrapper w(3);
    STATIC_REQUIRE(std::is_same_v<decltype(w), AnyResultWrapper<three_type>>);
    compare_value<three_type>(w, 3);

    SECTION("Serialize via base class") {
        std::stringstream ss;

        AnyResultWrapperBase* pw = &w;
        {
            pluginplay::BinaryOutputArchive ar(ss);
            Serializer s(ar);
            pw->serialize(s);
        }

        std::unique_ptr<AnyResultWrapperBase> pw2;
        {
            pluginplay::BinaryInputArchive ar(ss);
            Deserializer d(ar);
            auto temp = AnyResultWrapperBase::deserialize(d);
            pw2.swap(temp);
        }
        REQUIRE(*pw == *pw2);
    }
}

TEST_CASE("AnyResultWrapper<POD>(reference") {
    int x = 3;
    AnyResultWrapper w(x);
    STATIC_REQUIRE(std::is_same_v<decltype(w), AnyResultWrapper<int>>);
    compare_value<int>(w, 3);
}

TEST_CASE("AnyResultWrapper<POD>(const reference") {
    const int x = 3;
    AnyResultWrapper w(x);
    STATIC_REQUIRE(std::is_same_v<decltype(w), AnyResultWrapper<const int>>);
    compare_value<const int>(w, 3);
}

TEST_CASE("AnyResultWrapper<non-POD>(move)") {
    using vector_t = std::vector<double>;
    vector_t v{1.1, 2.2, 3.3};
    vector_t v2(v);
    const double* pv = v.data();

    AnyResultWrapper w(std::move(v));
    STATIC_REQUIRE(std::is_same_v<decltype(w), AnyResultWrapper<vector_t>>);
    compare_value<vector_t>(w, v2);

    REQUIRE(w.cast<vector_t&>().data() == pv);

    SECTION("Serialize via base class") {
        std::stringstream ss;

        AnyResultWrapperBase* pw = &w;
        {
            pluginplay::BinaryOutputArchive ar(ss);
            Serializer s(ar);
            pw->serialize(s);
        }

        std::unique_ptr<AnyResultWrapperBase> pw2;
        {
            pluginplay::BinaryInputArchive ar(ss);
            Deserializer d(ar);
            auto temp = AnyResultWrapperBase::deserialize(d);
            pw2.swap(temp);
        }
        REQUIRE(*pw == *pw2);
    }
}

TEST_CASE("AnyResultWrapper Comparisons") {
    AnyResultWrapper w(3);

    SECTION("Identical") {
        AnyResultWrapper w2(3);
        REQUIRE(w == w2);
        REQUIRE_FALSE(w != w2);
    }

    SECTION("Different value") {
        AnyResultWrapper w2(4);
        REQUIRE(w != w2);
        REQUIRE_FALSE(w == w2);
    }

    SECTION("Different type") {
        AnyResultWrapper w2(1.234);
        REQUIRE(w != w2);
        REQUIRE_FALSE(w == w2);
    }

    SECTION("Different const-ness") {
        const int x = 3;
        AnyResultWrapper w2(x);
        REQUIRE(w == w2);
        REQUIRE_FALSE(w != w2);
    }
}

struct NotPrintable {
    void hash(pluginplay::Hasher&) const noexcept {}
    template<typename Archive>
    void serialize(Archive& ar) {}
    bool operator==(const NotPrintable&) const noexcept { return true; }
};

TEST_CASE("AnyResultWrapper : str") {
    SECTION("printable type") {
        AnyResultWrapper w(int{3});
        REQUIRE(w.str() == "3");
    }
    SECTION("non-printable") {
        AnyResultWrapper w(NotPrintable{});
        auto* p = &w.cast<NotPrintable&>();
        std::stringstream ss;
        ss << "<" << typeid(NotPrintable).name() << " " << p << ">";
        REQUIRE(w.str() == ss.str());
    }
}

TEST_CASE("AnyResultWrapper : cast") {
    SECTION("non-const wrapper") {
        AnyResultWrapper w(int{3});
        REQUIRE(w.cast<int>() == 3);
        REQUIRE(w.cast<int&>() == 3);
        REQUIRE(w.cast<const int&>() == 3);
        REQUIRE_THROWS_AS(w.cast<double>(), std::bad_any_cast);
    }
    SECTION("const wrapper") {
        const AnyResultWrapper w(int{3});
        REQUIRE(w.cast<int>() == 3);
        // The following line should trip a static assert
        // w.cast<int&>()
        REQUIRE(w.cast<const int&>() == 3);
        REQUIRE_THROWS_AS(w.cast<double>(), std::bad_any_cast);
    }
}