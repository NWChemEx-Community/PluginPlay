#pragma once
#include "SDE/ModuleInput.hpp"
#include "SDE/ModuleOutput.hpp"

namespace SDE {
namespace detail_ {
class ModulePIMPL;
}

class Module {
public:
    using input_type  = ModuleInput;
    using input_map   = Utilities::CaseInsensitiveMap<input_type>;
    using output_type = ModuleOutput;
    using output_map  = Utilities::CaseInsensitiveMap<output_type>;
    using key_type    = std::string;

    Module();
    Module(const Module& rhs);
    Module& operator=(const Module& rhs);
    Module(Module&& rhs) noexcept;
    Module& operator=(Module&& rhs) noexcept;
    Module(std::unique_ptr<ModulePIMPL> pimpl) : pimpl_(std::move(pimpl)) {}
    ~Module() noexcept;

    template<typename property_type, typename... Args>
    auto run_as(Args&&... args) {
        auto temp = property_type::wrap_inputs(std::forward<Args>(args)...);
        input_map params = inputs();
        for(auto kv : temp) params.at(kv.first).change(std::move(kv.second));
        auto results = run(params);
        return property_type::unwrap_outputs(results);
    }

    const input_map& inputs() const;
    input_map& inputs() {
        const auto& input = const_cast<const Module&>(*this).inputs();
        return const_cast<input_map&>(input);
    }
    const output_map& outputs() const;
    output_map& outputs() {
        const auto& output = const_cast<const Module&>(*this).outputs();
        return const_cast<output_map&>(output);
    }
    const submod_map& submods() const;
    submod_map& submods() {
        const auto& submod = const_cast<const Module&>(*this).submods();
        return const_cast<submod_map&>(submod);
    }

    output_map run(input_map ps);

private:
    std::unique_ptr<ModulePIMPL> pimpl_;
};

} // namespace SDE
