#include <catch2/catch.hpp>
#include <sde/module_input.hpp>

using namespace sde;
using set_t = typename ModuleInput::bounds_check_desc_t;

TEST_CASE("ModuleInput : default ctor") {
    ModuleInput i;

    REQUIRE_FALSE(i.has_type());
    REQUIRE_FALSE(i.has_value());
    REQUIRE_FALSE(i.has_description());
    REQUIRE_FALSE(i.is_optional());
    REQUIRE_FALSE(i.is_transparent());
    REQUIRE_FALSE(i.ready());
}

TEST_CASE("ModuleInput : has_type") {
    ModuleInput i;
    SECTION("No type") { REQUIRE_FALSE(i.has_type()); }
    SECTION("has type") {
        i.set_type<int>();
        REQUIRE(i.has_type());
    }
}

TEST_CASE("ModuleInput : has_value") {
    ModuleInput i;
    SECTION("No value") { REQUIRE_FALSE(i.has_value()); }
    SECTION("Has value") {
        i.set_type<int>();
        i.change(int{3});
        REQUIRE(i.has_value());
    }
}

TEST_CASE("ModuleInput : has_description") {
    ModuleInput i;
    SECTION("No description") { REQUIRE_FALSE(i.has_description()); }
    SECTION("Description") {
        i.set_description("Hello World");
        REQUIRE(i.has_description());
    }
}

TEST_CASE("ModuleInput : is_optional") {
    ModuleInput i;
    SECTION("Required") { REQUIRE_FALSE(i.is_optional()); }
    SECTION("Optional") {
        i.make_optional();
        REQUIRE(i.is_optional());
    }
}

TEST_CASE("ModuleInput : is_transparent") {
    ModuleInput i;
    SECTION("Opaque") { REQUIRE_FALSE(i.is_transparent()); }
    SECTION("Transparent") {
        i.make_transparent();
        REQUIRE(i.is_transparent());
    }
}

TEST_CASE("ModuleInput : ready") {
    ModuleInput i;
    SECTION("Not ready") { REQUIRE_FALSE(i.ready()); }
    SECTION("Optional is ready") {
        i.make_optional();
        REQUIRE(i.ready());
    }
    SECTION("Has value is read") {
        i.set_type<int>();
        i.change(int{3});
        REQUIRE(i.ready());
    }
}

TEST_CASE("ModuleInput : is_valid") {
    ModuleInput i;

    SECTION("Throws if the type has not been set") {
        REQUIRE_THROWS_AS(i.is_valid(int{3}), std::runtime_error);
    }
    SECTION("Valid only type") {
        i.set_type<int>();
        REQUIRE(i.is_valid(int{3}));
    }
    SECTION("Not valid only type") {
        i.set_type<int>();
        REQUIRE_FALSE(i.is_valid(double{3.14}));
    }
    SECTION("Valid type + other") {
        i.set_type<int>();
        i.add_check(bounds_checking::NotEqualTo<const int&>(4));
        REQUIRE(i.is_valid(int{3}));
    }
    SECTION("Not valid type + other") {
        i.set_type<int>();
        i.add_check(bounds_checking::NotEqualTo<const int&>(4));
        REQUIRE_FALSE(i.is_valid(int{4}));
    }
}

TEST_CASE("ModuleInput : hash") {
    ModuleInput i;
    i.set_type<int>();
    SECTION("Empty") {
        REQUIRE(sde::hash_objects(i) == "cbc357ccb763df2852fee8c4fc7d55f2");
    }
    SECTION("Has value and opaque") {
        i.change(int{3});
        REQUIRE(sde::hash_objects(i) == "9a4294b64e60cc012c5ed48db4cd9c48");
    }
    SECTION("Has value and transparent") {
        i.change(int{3});
        i.make_transparent();
        REQUIRE(sde::hash_objects(i) == "00000000000000000000000000000000");
    }
}

TEST_CASE("ModuleInput: set_type") {
    ModuleInput i;

    SECTION("Set to a value") {
        auto pi = &(i.set_type<int>());
        REQUIRE(pi == &i);
        REQUIRE(i.has_type());
        auto msg = std::string("Type == ") + typeid(int).name();
        REQUIRE(i.check_descriptions() == set_t{msg});
    }

    SECTION("Set to a const reference") {
        auto pi = &(i.set_type<const int&>());
        REQUIRE(pi == &i);
        REQUIRE(i.has_type());
        auto msg = std::string("Type == ") + typeid(const int&).name();
        REQUIRE(i.check_descriptions() == set_t{msg});
    }

    SECTION("Can't be read/write reference") {
        // i.set_type<int&>(); //should trip static assert if uncommented
    }

    SECTION("Can't be rvalue reference") {
        // i.set_type<int&&>(); //should trip static assert if uncommented
    }

    SECTION("Can't be pointer") {
        // i.set_type<int*>(); //should trip static assert if uncommented
    }

    SECTION("Can't be a const pointer") {
        // i.set_type<const int*>(); //should trip static assert if uncommented
    }
}

TEST_CASE("ModuleInput : change") {
    ModuleInput i;

    SECTION("Throws if type is not set") {
        REQUIRE_THROWS_AS(i.change(int{3}), std::runtime_error);
    }
    SECTION("T == int") {
        i.set_type<int>();
        REQUIRE(i.has_type());

        SECTION("Good change : by value") {
            auto pi = &(i.change(int{3}));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            REQUIRE(i.value<int>() == 3);
        }
        SECTION("Good change : by reference") {
            int x   = 3;
            auto pi = &(i.change(x));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            REQUIRE(i.value<const int&>() == 3);
            SECTION("Made a copy") { REQUIRE(&(i.value<const int&>()) != &x); }
        }
        SECTION("Bad change: wrong type") {
            REQUIRE_THROWS_AS(i.change(double{3.14}), std::invalid_argument);
        }
    }
    SECTION("T == std::vector<int>") {
        using vector_t = std::vector<int>;
        i.set_type<vector_t>();
        vector_t value{1, 2, 3};
        auto pv = value.data();

        REQUIRE(i.has_type());

        SECTION("Good change : by value") {
            auto pi = &(i.change(vector_t{1, 2, 3}));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            REQUIRE(i.value<const vector_t&>() == value);
        }
        SECTION("Good change : by reference") {
            auto pi = &(i.change(value));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            REQUIRE(i.value<const vector_t&>() == value);
            SECTION("Made a copy") {
                REQUIRE(&(i.value<const vector_t&>()) != &value);
            }
        }
        SECTION("Good change : by move") {
            auto pi = &(i.change(std::move(value)));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            SECTION("Moved the value") {
                REQUIRE(i.value<const vector_t&>().data() == pv);
            }
        }
        SECTION("Bad change: wrong type") {
            REQUIRE_THROWS_AS(i.change(double{3.14}), std::invalid_argument);
        }
    }
    SECTION("T == const std::vector<int>&") {
        using vector_t = std::vector<int>;
        i.set_type<const vector_t&>();
        vector_t value{1, 2, 3};
        auto pv = value.data();

        REQUIRE(i.has_type());

        SECTION("Good change : by value") {
            auto pi = &(i.change(vector_t{1, 2, 3}));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            REQUIRE(i.value<const vector_t&>() == value);
        }
        SECTION("Good change : by reference") {
            auto pi = &(i.change(value));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            REQUIRE(i.value<const vector_t&>().data() == pv);
        }
        SECTION("Good change : by move") {
            vector_t value2(value);
            auto pi = &(i.change(std::move(value)));
            REQUIRE(pi == &i);
            REQUIRE(i.has_value());
            SECTION("Moved the value") {
                REQUIRE(i.value<const vector_t&>().data() == pv);
            }
        }
        SECTION("Bad change: wrong type") {
            REQUIRE_THROWS_AS(i.change(double{3.14}), std::invalid_argument);
        }
    }
}

TEST_CASE("ModuleInput : set_description") {
    ModuleInput i;

    SECTION("One call") {
        auto pi = &(i.set_description("Hello world"));
        REQUIRE(pi == &i);
        REQUIRE(i.has_description());
        REQUIRE(i.description() == "Hello world");
    }
    SECTION("Repeated calls overwrite") {
        i.set_description("Not going to be seen");
        i.set_description("Hello world");
        REQUIRE(i.description() == "Hello world");
    }
}

TEST_CASE("ModuleInput : add_check") {
    ModuleInput i;
    i.set_type<int>();
    set_t s{std::string("Type == ") + typeid(int).name()};
    SECTION("Pre-defined check") {
        auto pi = &(i.add_check(bounds_checking::NotEqualTo<int>(4)));
        REQUIRE(pi == &i);
        s.insert("!= 4");
        REQUIRE(i.check_descriptions() == s);
    }
    SECTION("User-defined check") {
        std::function<bool(const int&)> l{[](const int& x) { return x != 4; }};
        i.add_check(l, "My check");
        s.insert("My check");
        REQUIRE(i.check_descriptions() == s);
    }
    SECTION("Throws if already contains value and value fails check") {
        i.change(int{4});
        REQUIRE_THROWS_AS(i.add_check(bounds_checking::NotEqualTo<int>(4)),
                          std::invalid_argument);
    }
}

TEST_CASE("ModuleInput : make_optional") {
    ModuleInput i;
    SECTION("Not optional") {
        auto pi = &(i.make_optional());
        REQUIRE(pi == &i);
        REQUIRE(i.is_optional());
    }
    SECTION("Already optional") {
        i.make_optional();
        i.make_optional();
        REQUIRE(i.is_optional());
    }
}

TEST_CASE("ModuleInput : make_required") {
    ModuleInput i;
    SECTION("Already required") {
        auto pi = &(i.make_required());
        REQUIRE(pi == &i);
        REQUIRE_FALSE(i.is_optional());
    }
    SECTION("Optional") {
        i.make_optional();
        REQUIRE(i.is_optional());
        i.make_required();
        REQUIRE_FALSE(i.is_optional());
    }
}

TEST_CASE("ModuleInput : make_opaque") {
    ModuleInput i;
    SECTION("Already opaque") {
        auto pi = &(i.make_opaque());
        REQUIRE(pi == &i);
        REQUIRE_FALSE(i.is_transparent());
    }
    SECTION("Transparent") {
        i.make_transparent();
        REQUIRE(i.is_transparent());
        i.make_opaque();
        REQUIRE_FALSE(i.is_transparent());
    }
}

TEST_CASE("ModuleInput : make_transparent") {
    ModuleInput i;
    SECTION("Opaque") {
        auto pi = &(i.make_transparent());
        REQUIRE(pi == &i);
        REQUIRE(i.is_transparent());
    }
    SECTION("Already transparent") {
        i.make_transparent();
        i.make_transparent();
        REQUIRE(i.is_transparent());
    }
}

TEST_CASE("ModuleInput : value") {
    ModuleInput i;
    SECTION("Throws if no value") {
        REQUIRE_THROWS_AS(i.value<int>(), std::runtime_error);
    }
    SECTION("T == int") {
        i.set_type<int>();
        i.change(int{3});
        REQUIRE(i.value<int>() == 3);
        REQUIRE(i.value<int&>() == 3);
        REQUIRE(i.value<const int&>() == 3);
        REQUIRE_THROWS_AS(i.value<double>(), std::bad_any_cast);
    }
    SECTION("T == const std::vector<int>&") {
        using v_t = std::vector<int>;
        i.set_type<const v_t&>();
        v_t value{1, 2, 3};
        auto pv = value.data();
        i.change(value);
        REQUIRE(i.value<v_t>() == value);
        REQUIRE_FALSE(i.value<v_t>().data() == pv);
        REQUIRE(i.value<const v_t&>().data() == pv);
        REQUIRE_THROWS_AS(i.value<v_t&>(), std::bad_any_cast);
    }
}

TEST_CASE("ModuleInput : description") {
    ModuleInput i;

    SECTION("Throws if description is not set") {
        REQUIRE_THROWS_AS(i.description(), std::bad_optional_access);
    }

    SECTION("Gets description") {
        i.set_description("Hello world");
        REQUIRE(i.description() == "Hello world");
    }
}

TEST_CASE("Module Input : check_descriptions") {
    ModuleInput i;

    SECTION("No descriptions") { REQUIRE(i.check_descriptions().empty()); }

    SECTION("Only type description") {
        i.set_type<int>();
        auto msg = std::string("Type == ") + typeid(int).name();
        REQUIRE(i.check_descriptions() == set_t{msg});
    }
}

TEST_CASE("ModuleInput : Equality comparisons") {
    ModuleInput i, i2;

    SECTION("Defaults are equivalent") {
        REQUIRE(i == i2);
        REQUIRE_FALSE(i != i2);
    }
    SECTION("Different types") {
        i2.set_type<int>();
        REQUIRE_FALSE(i == i2);
        REQUIRE(i != i2);
    }
    SECTION("Different values") {
        i2.set_type<int>();
        i.set_type<int>();
        i2.change(int{4});
        i.change(int{3});
        REQUIRE_FALSE(i == i2);
        REQUIRE(i != i2);
    }
    SECTION("Different descriptions") {
        i2.set_description("hello world");
        REQUIRE_FALSE(i == i2);
        REQUIRE(i != i2);
    }
    SECTION("Different optional-ness") {
        i2.make_optional();
        REQUIRE_FALSE(i == i2);
        REQUIRE(i != i2);
    }
    SECTION("Different transparencies") {
        i2.make_transparent();
        REQUIRE_FALSE(i == i2);
        REQUIRE(i != i2);
    }
}
