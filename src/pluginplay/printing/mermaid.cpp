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

#include <pluginplay/pluginplay.hpp>

std::string hello_world(const pluginplay::ModuleManager& mm) {
    auto n_modules = mm.size();
    for(decltype(n_modules) i = 0; i < n_modules; i++) {
        auto mod    = mm.keys()[i];
        auto module = mm.at(mod);
        std::cout << mod << std::endl;
        auto submods = module.submods();
        for(const auto& [key, value] : submods)
            std::cout << "This is a submod: " << key << std::endl;
    }
    return "Hello World!";
}
