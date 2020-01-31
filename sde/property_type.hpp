#pragma once
#include "sde/field_tuple.hpp"
#include "sde/module_input.hpp"
#include "sde/module_result.hpp"

namespace sde {

/** @brief Base class for defining the API a module uses to compute a particular
 * property.
 *
 * The point of a property type is to standardize the API for obtaining a
 * property. Modules then interact with other modules through these standardized
 * APIs. This couples modules based on the properties they need and not how
 * those properties are obtained. The API is separated into two parts the
 * "input", *i.e.*, the arguments to the module's `run` member, and the
 * "results", *i.e.*, the values returned from the module's `run` member.
 *
 * The PropertyType class works by the curiously recursive template pattern, or
 * CRTP, as it's better known. CRTP enables compile-time polymorphism. What this
 * means to the user is that to implement a new property type you inherit from
 * `PropertyType<X>` where `X` is the type of the class you are implementing
 * (this is the recursive part of CRTP, your class appears as part of the base
 * class's type). Your derived class must then implement two functions:
 *
 * - `inputs_()` which calls the member function `declare_input()`, fills the
 *   object returned by `declare_input()` with the input part of the property
 *   type API, and then returns the object.
 * - `results_()` same as `inputs_()` except one calls `declare_result()` and
 *   fills the object with the details of the results that a property type
 *   computes.
 *
 * For what it's worth, the names of the functions are symbolic of the fact that
 * they should be treated like private/protected functions. The PropertyType
 * class will need access to them so the derived class must make them public or
 * declare `PropertyType<X>` as a friend. The latter is arguably idiomatically
 * more correct as it protects against users accidentally calling the
 * implementation directly.
 *
 * @tparam DerivedType Must be default constructable and define the functions
 *         listed in the above description.
 */
template<typename DerivedType>
class PropertyType {
private:
    /// Type of this base class
    using my_type = PropertyType<DerivedType>;

public:
    ///@{
    /** @name Public APIs for obtaining the fields comprising the API
     *
     *  The object that is returned from this function can be used like a map
     *  where the keys are the keywords used to access each field and the values
     *  are the actual fields. The object stores the fields in the order that
     *  the fields appear in the API. For example if the input API takes an
     *  `int`, a `double`, and a `std::string` the first field in the object
     *  will correspond to the `int`, the second to the `double`, and the third
     *  to the `std::string`. This works the same for the object that is
     *  returned containing the results.
     *
     * @return An object containing the fields in the input API of the property
     *         type or the return part of the API.
     * @throws ??? if the derived class's implementation throws. Same throw
     *         guarantee as the derived class.
     */
    static auto inputs();

    static auto results();
    ///@}

    ///@{
    /** @name Functions automating the wrapping/unwrapping of inputs/results
     *
     * There's a lot of boilerplate associated with taking the inputs/results
     * and placing them into ModuleInput/ModuleResult instances or taking
     * them out of such instances. The present class knows the keywords, types,
     * and position in the API of each input/result and thus is in a position to
     * automate this for you. That is what the functions in this section do.
     *
     * @tparam T The type of the map-like container we are wrapping into or
     *         unwrapping from.
     * @tparam Args The types of the arguments to wrap.
     * @param rv The map-like container instance to hold the wrapped values or
     *        take them from.
     * @param args The values to wrap.
     * @return For wrapping, the container with the wrapped values safely
     *         inside. For unwrapping, an std::tuple of the unwrapped values
     *         ready to be used with structured bindings.
     */
    template<typename T, typename... Args>
    static auto wrap_inputs(T&& rv, Args&&... args);

    template<typename T, typename... Args>
    static auto& wrap_results(T&& rv, Args&&... args);

    template<typename T>
    static auto unwrap_inputs(T&& rv);

    template<typename T>
    static auto unwrap_results(T&& rv);
    ///@}
private:
    ///@{
    /** @name Automatic wrapping/unwrapping implementations.
     *
     * The functions in this section are the implementations for the automatic
     * wrap/unwrap functions.
     *
     * Both the wrapping and unwrapping variants work recursively calling the
     * "guts" variant until all of the arguments are handled, keeping track of
     * the recursion depth via a template non-type parameter. The call at a
     * depth `i` then worries about wrapping/unwrapping the `i`-th element. We
     * get the required information, mainly type, by calling `inputs`/`results`
     * on-the-fly.
     */
    template<typename T, typename U, typename... Args>
    static auto& wrap_(T&& rv, U&& builder, Args&&... args);

    template<std::size_t ArgI, typename T, typename U, typename V,
             typename... Args>
    static void wrap_guts_(T&& rv, U&& builder, V&& value, Args&&... args);

    template<typename T, typename U>
    static auto unwrap_(T&& builder, U&& rv);

    template<std::size_t ArgI, typename T, typename U>
    static auto unwrap_guts_(T&& builder, U&& rv);
    ///@}

}; // End class property_type

///@{
/** @name API generators
 *
 * The class implementing the property type should use these functions to
 * generate the objects for declaring the input/result APIs.
 *
 * @return A factory object that will facilitate the declaration of the
 *         input/result APIs.
 *
 * @relates PropertyType
 */
inline auto declare_input() { return FieldTuple<ModuleInput>{}; }

inline auto declare_result() { return FieldTuple<ModuleResult>{}; }
///@}

// ------------------------- Helper Macros ------------------------------------

/** @brief Declares a new PropertyType
 *
 *  This macro will declare a derived class @p prop_type which inherits from
 *  PropertyType and forwarded declares all necessary hooks.
 *
 *  @param[in] prop_type The name to use for the resulting property type.
 *
 */
#define DECLARE_PROPERTY_TYPE(prop_type)              \
    struct prop_type : sde::PropertyType<prop_type> { \
        auto inputs_();                               \
        auto results_();                              \
    }

/** @brief Starts the definition of a property types' inputs hook.
 *
 *  The inputs hook of a class derived from PropertyTypes establishes the fields
 *  that the module recognizes as input. The hook is implemented as a member
 *  function of the @p prop_type class with the macro taking care of declaring
 *  the hook's signature so that all you need to do is declare the function's
 *  body.
 *
 *  @param[in] prop_type The name of the class defining the property type.
 */
#define PROPERTY_TYPE_INPUTS(prop_type) inline auto prop_type::inputs_()

/** @brief Starts the definition of a property types' results hook.
 *
 *  The results hook of a class derived from PropertyTypes establishes the
 *  fields that the module returns. The hook is implemented as a member
 *  function of the @p prop_type class with the macro taking care of declaring
 *  the hook's signature so that all you need to do is declare the function's
 *  body.
 *
 *  @param[in] prop_type The name of the class defining the property type.
 */
#define PROPERTY_TYPE_RESULTS(prop_type) inline auto prop_type::results_()

// ----------------------------- Implementations -------------------------------
#define PROP_TYPE PropertyType<DerivedType>

template<typename DerivedType>
auto PROP_TYPE::inputs() {
    DerivedType parent;
    return parent.inputs_();
}

template<typename DerivedType>
auto PROP_TYPE::results() {
    DerivedType parent;
    return parent.results_();
}

template<typename DerivedType>
template<typename T, typename... Args>
auto PROP_TYPE::wrap_inputs(T&& rv, Args&&... args) {
    return wrap_(std::forward<T>(rv), inputs(), std::forward<Args>(args)...);
}

template<typename DerivedType>
template<typename T, typename... Args>
auto& PROP_TYPE::wrap_results(T&& rv, Args&&... args) {
    return wrap_(std::forward<T>(rv), results(), std::forward<Args>(args)...);
}

template<typename DerivedType>
template<typename T>
auto PROP_TYPE::unwrap_inputs(T&& rv) {
    // If there are no inputs to unwrap we return an empty tuple
    using input_type                 = decltype(DerivedType::inputs());
    using traits_type                = typename input_type::traits_type;
    using input_tuple                = typename traits_type::tuple_of_fields;
    static constexpr bool has_inputs = traits_type::nfields > 0;

    if constexpr(has_inputs) {
        return unwrap_(inputs(), std::forward<T>(rv));
    } else {
        return input_tuple{};
    }
}

template<typename DerivedType>
template<typename T>
auto PROP_TYPE::unwrap_results(T&& rv) {
    return unwrap_(results(), std::forward<T>(rv));
}

template<typename DerivedType>
template<typename T, typename U, typename... Args>
auto& PROP_TYPE::wrap_(T&& rv, U&& builder, Args&&... args) {
    if constexpr(sizeof...(Args) > 0)
        wrap_guts_<0>(std::forward<T>(rv), std::forward<U>(builder),
                      std::forward<Args>(args)...);
    return rv;
}

template<typename DerivedType>
template<std::size_t ArgI, typename T, typename U, typename V, typename... Args>
void PROP_TYPE::wrap_guts_(T&& rv, U&& builder, V&& value, Args&&... args) {
    using traits_type     = typename std::decay_t<U>::traits_type;
    using tuple_of_fields = typename traits_type::tuple_of_fields;
    using type            = std::tuple_element_t<ArgI, tuple_of_fields>;
    static_assert(std::is_convertible_v<V, type>,
                  "Wrap argument is of incorrect type.");
    auto key = (builder.begin() + ArgI)->first;
    rv.at(key).change(std::forward<V>(value));
    if constexpr(sizeof...(Args) > 0)
        wrap_guts_<ArgI + 1>(std::forward<T>(rv), std::forward<U>(builder),
                             std::forward<Args>(args)...);
}

template<typename DerivedType>
template<typename T, typename U>
auto PROP_TYPE::unwrap_(T&& builder, U&& rv) {
    auto results =
      unwrap_guts_<0>(std::forward<T>(builder), std::forward<U>(rv));
    using tuple_type            = decltype(results);
    constexpr std::size_t nargs = std::tuple_size_v<tuple_type>;
    if constexpr(nargs == 0) return;
    //        else if constexpr(nargs == 1){
    //            using type = std::tuple_element_t <0, tuple_type>;
    //            return std::forward<type>(std::get<0>(results));
    //        }
    else
        return results;
}

template<typename DerivedType>
template<std::size_t ArgI, typename T, typename U>
auto PROP_TYPE::unwrap_guts_(T&& builder, U&& rv) {
    using tuple_of_fields = typename T::traits_type::tuple_of_fields;
    constexpr auto nargs  = std::tuple_size_v<tuple_of_fields>;
    if constexpr(ArgI == nargs)
        return std::make_tuple();
    else {
        using type = std::tuple_element_t<ArgI, tuple_of_fields>;
        auto key   = (builder.begin() + ArgI)->first;
        auto lhs   = std::tuple<type>(rv.at(key).template value<type>());
        auto rhs =
          unwrap_guts_<ArgI + 1>(std::forward<T>(builder), std::forward<U>(rv));
        return std::tuple_cat(std::move(lhs), std::move(rhs));
    }
}

#undef PROP_TYPE

} // namespace sde
