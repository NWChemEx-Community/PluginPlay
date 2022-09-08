/*
 * Copyright 2022 NWChemEx-Project
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

#include "database/database_factory.hpp"
#include "module_cache_pimpl.hpp"
#include <filesystem>
#include <pluginplay/cache/module_cache.hpp>
#include <pluginplay/cache/module_manager_cache.hpp>
#include <pluginplay/cache/user_cache.hpp>

namespace pluginplay::cache {
namespace detail_ {

struct ModuleManagerCachePIMPL {
    using parent_class = ModuleManagerCache;

    using module_cache_key = typename parent_class::module_cache_key;

    using path_type = typename parent_class::path_type;

    using module_cache_pointer = typename parent_class::module_cache_pointer;

    using user_cache_pointer = typename parent_class::user_cache_pointer;

    using db_factory_type = database::DatabaseFactory;

    database::DatabaseFactory m_db_factory;

    std::map<module_cache_key, module_cache_pointer> m_module_caches;

    std::map<module_cache_key, user_cache_pointer> m_user_caches;
};

} // namespace detail_

ModuleManagerCache::ModuleManagerCache() noexcept = default;

ModuleManagerCache::ModuleManagerCache(path_type disk_location) {
    change_save_location(std::move(disk_location));
}

ModuleManagerCache::~ModuleManagerCache() noexcept = default;

void ModuleManagerCache::change_save_location(path_type disk_location) {
    std::filesystem::path root_dir(disk_location);
    if(!std::filesystem::exists(root_dir)) {
        std::filesystem::create_directories(root_dir);
    }

    std::filesystem::path cache_dir("cache");
    std::filesystem::path uuid_dir("uuid");

    auto p = root_dir / cache_dir;
    auto q = root_dir / uuid_dir;

    pimpl_().m_db_factory.set_serialized_pm_to_pm(p.string());
    m_pimpl_->m_db_factory.set_type_eraser_backend(q.string());
}

typename ModuleManagerCache::module_cache_pointer
ModuleManagerCache::get_or_make_module_cache(module_cache_key key) {
    if(!pimpl_().m_module_caches.count(key)) {
        auto p = std::make_shared<module_cache_type>(make_module_cache_(key));
        m_pimpl_->m_module_caches.emplace(key, p);
    }
    return m_pimpl_->m_module_caches.at(key);
}

typename ModuleManagerCache::user_cache_pointer
ModuleManagerCache::get_or_make_user_cache(module_cache_key key) {
    module_cache_key mangled_key = "__PP__ " + key + "-USER __PP__";
    if(!pimpl_().m_user_caches.count(mangled_key)) {
        auto mcache = make_module_cache_(mangled_key);
        auto p      = std::make_shared<user_cache_type>(std::move(mcache));
        m_pimpl_->m_user_caches.emplace(mangled_key, p);
    }
    return m_pimpl_->m_user_caches.at(mangled_key);
}

typename ModuleManagerCache::module_cache_type
ModuleManagerCache::make_module_cache_(module_cache_key key) {
    auto p  = std::make_unique<detail_::ModuleCachePIMPL>();
    p->m_db = pimpl_().m_db_factory.default_module_db(std::move(key));
    return module_cache_type(std::move(p));
}

void ModuleManagerCache::assert_pimpl_() const {
    if(m_pimpl_) return;
    throw std::runtime_error("This read-only ModuleManagerCache has no PIMPL. "
                             "Was it default initialized or moved from?");
}

typename ModuleManagerCache::pimpl_reference ModuleManagerCache::pimpl_() {
    if(!m_pimpl_) { m_pimpl_ = std::make_unique<pimpl_type>(); }
    return *m_pimpl_;
}

typename ModuleManagerCache::const_pimpl_reference ModuleManagerCache::pimpl_()
  const {
    assert_pimpl_();
    return *m_pimpl_;
}

} // namespace pluginplay::cache
