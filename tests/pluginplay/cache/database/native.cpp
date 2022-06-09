#include <catch2/catch.hpp>
#include <pluginplay/cache/database/native.hpp>

using namespace pluginplay::cache::database;

using id_pair    = std::pair<int, double>;
using si_pair    = std::pair<std::string, int>;
using test_types = std::tuple<id_pair, si_pair>;

TEMPLATE_LIST_TEST_CASE("Map", "", test_types) {
    using value_type  = TestType;
    using key_type    = std::tuple_element_t<0, value_type>;
    using mapped_type = std::tuple_element_t<1, value_type>;

    using map_type = Native<key_type, mapped_type>;

    key_type default_key{};
    mapped_type default_value{};

    typename map_type::map_type default_map, val{{default_key, default_value}};

    auto backup  = std::make_unique<map_type>();
    auto pbackup = backup.get();

    map_type defaulted;
    map_type has_val(val);
    map_type has_backup(val, std::move(backup));

    SECTION("Ctors") {
        REQUIRE(defaulted.map() == default_map);
        REQUIRE(has_val.map() == val);
        REQUIRE(has_backup.map() == val);
    }

    SECTION("Count") {
        REQUIRE_FALSE(defaulted.count(default_key));
        REQUIRE(has_val.count(default_key));
        REQUIRE(has_backup.count(default_key));
    }

    SECTION("insert") {
        defaulted.insert(default_key, default_value);
        REQUIRE(defaulted.map() == val);
    }

    SECTION("free") {
        has_val.free(default_key);
        REQUIRE_FALSE(has_val.count(default_key));

        has_backup.free(default_key);
        REQUIRE_FALSE(has_backup.count(default_key));
    }

    SECTION("at") {
        REQUIRE(has_val.at(default_key).get() == default_value);
        REQUIRE(has_backup.at(default_key).get() == default_value);
    }

    SECTION("backup") {
        has_backup.backup();
        REQUIRE(has_backup.count(default_key));
        REQUIRE(pbackup->count(default_key));
        REQUIRE(pbackup->at(default_key).get() == default_value);
    }

    SECTION("dump") {
        has_val.dump();
        REQUIRE_FALSE(has_val.count(default_key));

        has_backup.dump();
        REQUIRE_FALSE(has_backup.count(default_key));
        REQUIRE(pbackup->count(default_key));
        REQUIRE(pbackup->at(default_key).get() == default_value);
    }
}
