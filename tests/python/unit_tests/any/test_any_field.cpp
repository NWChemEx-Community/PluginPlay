/*
 * Copyright 2023 NWChemEx-Project
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

#include "test_any.hpp"
#include <pluginplay/any/any.hpp>

namespace test_pluginplay {

void test_any_field(pybind11::module_& m) {
    auto m_test_any = m.def_submodule("test_any_field");
    m_test_any.def("get_vector", []() {
        std::vector<int> v{1, 2, 3};
        return pluginplay::any::make_any_field<std::vector<int>>(std::move(v));
    });
}

} // namespace test_pluginplay
