#include "SDE/Memoization.hpp"
#include "SDE/Module.hpp"
#include "SDE/PyBindings/PySmartEnums.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using module_pointer = typename SDE::ModuleBase::module_pointer;

namespace SDE {
namespace detail_ {

struct PyModuleBase {
    void set_submodule(module_pointer me, std::string key, module_pointer ptr) {
        me->submodules_[key] = ptr;
    }

    void set_metadata(module_pointer me, const MetaProperty& key,
                      std::string value) {
        me->metadata_[key] = value;
    }
};

} // namespace detail_

void pythonize_Module(pybind11::module& m) {
    DECLARE_PySmartEnum(Resource, time, memory, disk, processes, threads);
    DECLARE_PySmartEnum(MetaProperty, name, version, description, authors,
                        citations);
    DECLARE_PySmartEnum(ModuleTraits, nondeterministic);

    pybind11::class_<ModuleBase, module_pointer>(m, "ModuleBase")
      .def(pybind11::init<>())
      .def("submodules", &ModuleBase::submodules)
      .def("metadata", &ModuleBase::metadata)
      //.def("parameters", &ModuleBase::parameters)
      .def("change_submodule", &ModuleBase::change_submodule)
      //.def("change_parameter", &ModuleBase::change_parmaeter)
      .def("locked", &ModuleBase::locked)
      .def("lock", &ModuleBase::lock)
      .def("__eq__",
           [](module_pointer lhs, module_pointer rhs) { return lhs == rhs; })
      .def("_set_submodule",
           [](module_pointer me, std::string key, module_pointer ptr) {
               detail_::PyModuleBase().set_submodule(me, key, ptr);
           })
      .def("_set_submodule",
           [](module_pointer me, std::string key, pybind11::none a_none) {
               detail_::PyModuleBase().set_submodule(me, key, module_pointer{});
           })
      .def("_set_metadata",
           [](module_pointer me, const MetaProperty& key, std::string value) {
               detail_::PyModuleBase().set_metadata(me, key, value);
           });
}
} // namespace SDE
