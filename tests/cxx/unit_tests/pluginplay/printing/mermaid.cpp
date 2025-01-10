/*
 * Copyright 2024 NWChemEx-Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../catch.hpp"
#include "../test_common.hpp"
#include "module/macros.hpp"
#include "unit_testing_pts.hpp"
#include <pluginplay/module_manager/module_manager.hpp>
#include <pluginplay/printing/mermaid.hpp>

DECLARE_MODULE(SubModLvl10);
inline MODULE_CTOR(SubModLvl10) {
    satisfies_property_type<testing::NullPT>();
    add_submodule<testing::NullPT>("Submod 10");
}

DECLARE_MODULE(SubModLvl9);
inline MODULE_CTOR(SubModLvl9) {
    satisfies_property_type<testing::NullPT>();
    add_submodule<testing::NullPT>("Submod 9");
}

DECLARE_MODULE(TooManySubmods);
inline MODULE_CTOR(TooManySubmods) {
    satisfies_property_type<testing::NullPT>();
}

TEST_CASE("hello_world") {
    pluginplay::ModuleManager mm;

    SECTION("huh") {
        using mod_t = testing::NoPTModule;
        mm.add_module<mod_t>("A mod");
        using mod_s = testing::SubModModule;
        mm.add_module<mod_s>("SubModModule");

        auto hello = hello_world(mm);
        std::cout << "Bro the test ran but idk if the internals ran"
                  << std::endl;
        REQUIRE(hello == "Hello World!");
    }
}
