#pragma once
#include "SDE/Module.hpp"
#include <Utilities/Containers/CaseInsensitiveMap.hpp>
#include <Utilities/SmartEnum.hpp>

namespace SDE {

/**
 * @brief Enumerations pertaining to the characteristics of the module.
 *
 * The moduletraits set of enumerations is envisioned as being one of the ways
 * the SDE, as well as other modules, can obtain information related to the
 * module, without knowing its contents.
 *
 * At the moment the recognized traits include:
 * - nondeterministic: Signals that runs of the same module with the same
 *   algorithmic parameters and the same input may yield different results
 *   (intentionally).  Consequentially memoization should not be performed.
 */
DECLARE_SmartEnum(ModuleTraits, nondeterministic);

/**
 *  @brief The class used by the ModuleManager to create a new instance of a
 *         module.  Also stores meta data that the module developer provided.
 *
 *  Once a module developer has written a module, the SDE needs to know how to
 *  create instances of that module.  The ModuleLoader ultimately fills that
 *  role.  In addition to literally being able to make that module, the module
 *  loader also tells us what the default parameters and submodules are for the
 *  module.
 *
 *  Usage of the ModuleLoader class is described in detail on the
 *  @ref using_module_loader "Using the ModuleLoader Class" page.
 */
struct ModuleLoaderBase {
    /// The type of the pointer returned by get_module
    using module_pointer = typename ModuleBase::module_pointer;

    /// The type of the pointer returned by clone
    using loader_pointer = std::unique_ptr<ModuleLoaderBase>;

    /// The type of the meta-data associated with the loader
    using meta_data_type = typename ModuleBase::meta_data_type;

    /// Ensures the class is polymorphic
    virtual ~ModuleLoaderBase() = default;

    /// To be implemented by the developer to return an instance of their module
    virtual module_pointer get_module() const = 0;

    /// Allows a polymorphic copy of the ModuleLoader class
    virtual loader_pointer clone() const = 0;

    /// The meta data the developer set
    meta_data_type meta_data;

    /// The traits the developer set
    std::set<ModuleTraits> traits;

    Utilities::CaseInsensitiveMap<std::string> submodules;

}; // class ModuleLoader

/**
 * @brief Class that implements ModuleLoaderBase for a particular module
 *
 * After deriving from this class the only thing a module developer should have
 * to do is fill in the meta-data contained in ModuleLoaderBase.
 *
 * @tparam ModuleType The class containing the implementation of your module.
 *         Generally speaking this is **NOT** the module type, but rather the
 *         class derived from it.
 */
template<typename ModuleType>
class ModuleLoader : public ModuleLoaderBase {
public:
    module_pointer get_module() const override {
        return std::make_unique<ModuleType>();
    }

    loader_pointer clone() const override {
        return std::make_unique<ModuleLoader<ModuleType>>(*this);
    }
};

} // namespace SDE