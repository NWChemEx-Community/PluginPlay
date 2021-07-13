#pragma once
#include "sde/hasher.hpp"
#include "sde/types.hpp"
#include "sde/utility.hpp"
#include <any>
#include <memory>
#include <utilities/printing/print_stl.hpp>

namespace sde::detail_ {
/// Forward declare class that will implement the API
template<typename T>
class SDEAnyWrapper;

/**
 * @brief Defines the API for interacting with the type-erased value.
 *
 * This class is meant for use with the SDEAny class and can be thought of
 * as a PIMPL for that class. This actual implementation works by holding
 * the value in an std::any instance (thereby differing casting and type
 * checks to it) and leveraging as much of that API as possible. Additional
 * member functions that are part of the SDEAny API, but not std::any's API,
 * are then implemented by a derived class, which knows the type to cast the
 * std::any to.
 */
class SDEAnyWrapperBase {
protected:
    /// My type
    using my_type = SDEAnyWrapperBase;

    /// type for determining if @p T inherits from us
    template<typename T>
    using is_wrapper = std::is_base_of<my_type, T>;

    /// does @p T does not inherit from us?
    template<typename T>
    static constexpr bool is_not_wrapper_v = std::negation_v<is_wrapper<T>>;

    /// type for enabling a function if @p T does not stem from us
    template<typename T>
    using enable_if_not_wrapper_t = std::enable_if_t<is_not_wrapper_v<T>, int>;

public:
    /// Type of a unique_ptr to the wrapper
    using wrapper_ptr = std::unique_ptr<SDEAnyWrapperBase>;

    /// Type of the RTTI of the wrapped instance
    using rtti_type = const std::type_info&;

    /** @brief Creates an SDEAnyWrapper by forwarding the provided value.
     *
     *  @note This ctor only participates in overload resolution if @p T is
     * not SDEAnyWrapperBase or any instanitation of SDEAnyWrapper.
     *
     *  @tparam T The type of the value we are wrapping.
     *
     *  @param[in] value The value we are wrapping.
     *
     *  @throw ??? throws if forwarding @p value to std::any's ctor throws.
     *             Same throw guarantee.
     */
    template<typename T, enable_if_not_wrapper_t<T> = 0>
    explicit SDEAnyWrapperBase(T&& value) : m_value_(std::forward<T>(value)) {}

    /** @brief Cleans up an SDEAnyBase_ instance.
     *
     *  Since SDEAnyBase_ instances have no state this is a null operation.
     *  However, it is important to mark this function as virtual since we
     * will literally always be passing the derived class around by its base
     * type.
     *
     *  @throws None. No throw guarantee.
     */
    virtual ~SDEAnyWrapperBase() = default;

    /**
     *  @brief Makes a polymorphic copy of the wrapper.
     *
     *  This member calls clone_() in order to get a polymorphic deep copy
     * of the wrapper.
     *
     *  @return A deep copy, on the heap, of the wrapper.
     *
     *  @throw ??? If the copy constructor of the wrapped instance throws.
     * Same guarantee as the wrapped instance's copy ctor.
     */
    wrapper_ptr clone() const { return clone_(); }

    /** @brief Provides the RTTI of the wrapped class.
     *
     * This member calls type_ to get the RTTI of the wrapped instance.
     *
     * @return The RTTI of the wrapped class.
     *
     * @throw None No throw guarantee.
     */
    rtti_type type() const noexcept { return type_(); };

    /** @brief Determines if the wrapped instance is convertible to the
     *         provided type.
     *
     * @tparam T The type to try casting to.
     * @return True if the wrapped instance is convertible to type @p T and
     *         false otherwise.
     *
     * @throw None no throw guarantee.
     */
    template<typename T>
    bool is_convertible() const noexcept;

    /**
     *  @brief Allows the SDEAnyBase_ instance to be hashed.
     *
     *  This function simply delegates to the protected hash_ member
     *  function (following the suggestion on BPHash's website).
     *
     *  @param[in, out] h A Hasher instance to use for the hashing.
     *
     *  @par Complexity:
     *  Same as the complexity of hashing the wrapped type.
     *
     *  @par Data Races:
     *  The state of the current instance will be accessed and data races
     *  may
     *  result if it is concurrently modified.
     *
     *  @throws ??? if the wrapped instance's hash function throws.  Strong
     *  throw guarantee.
     */
    void hash(Hasher& h) const { hash_(h); }

    /** @brief Returns the string representation of the object stored in the
     *         any.
     *
     * @return A string representation of the
     */
    std::string str() const { return str_(); }

    /** @brief Used to determine if the type-erased value of this instance
     * is the same as that of another instance.
     *
     *  This function ultimately calls are_equal_ to determine if the two
     *  instances are equal. Equality of the wrapped values is determined by
     *  the result of calling operator== on the wrapped value. If the
     * wrapped instances are different types, as determined by their RTTI,
     * then they considered not equal.
     *
     *  @param[in] rhs the instance to compare to.
     *
     *  @return true if the wrapped instances compare equivalent and false
     *          otherwise.
     *
     *  @throw none No throw guarantee.
     */
    bool operator==(const SDEAnyWrapperBase& rhs) const noexcept;

    /** @brief Used to determine if the type-erased value of this instance
     * is different from that of another instance.
     *
     *  This function ultimately negates a call to are_equal_ in order to
     *  determine if the two instances are different. Equality of the
     * wrapped values is determined by the result of calling operator== on
     * the wrapped value. If the wrapped instances are different types, as
     * determined by their RTTI, then they are considered different.
     *
     *  @param[in] rhs the instance to compare to.
     *
     *  @return false if the wrapped instances compare equivalent and true
     *          otherwise.
     *
     *  @throw none No throw guarantee.
     */
    bool operator!=(const SDEAnyWrapperBase& rhs) const noexcept;

    /** @brief Casts the wrapped value back to a readonly type @p T
     *
     *  This function allows you to retrieve the wrapped value as long as
     * you request the value as a type it is implicitly convertible to. This
     *  particular overload additionally restricts you to deep copies of the
     *  wrapped value or read-only access.
     *
     *  @tparam T The type to retrieve the wrapped value as.
     * .
     *  @return The wrapped instance.
     *
     *  @throw std::bad_any_cast if the wrapped instance is not of type @p
     * T. Strong throw guarantee.
     */
    template<typename T>
    T cast() const {
        return std::any_cast<T>(m_value_);
    }

    /** @brief Casts the wrapped value back to a readonly type @p T
     *
     *  This function allows you to retrieve the wrapped value as long as
     * you request the value as a type it is implicitly convertible to. This
     *  particular overload allows you to get the wrapped value back in a
     * read write state.
     *
     *  @tparam T The type to retrieve the wrapped value as.
     * .
     *  @return The wrapped instance.
     *
     *  @throw std::bad_any_cast if the wrapped instance is not of type @p
     * T. Strong throw guarantee.
     */
    template<typename T>
    T cast() {
        return std::any_cast<T>(m_value_);
    }

protected:
    /** @brief Deep copies the type-erased value held in @p rhs.
     *
     *  This function is protected so it can be used by the derived class to
     *  implement clone_. Making it public opens us up to slicing, which we
     *  wish to avoid in the event that the derived class ever has state.
     *
     *  @param[in] rhs The instance to copy.
     *
     *  @throw ??? if the copying the value throws. Same guarantee.
     */
    SDEAnyWrapperBase(const SDEAnyWrapperBase&) = default;

    /** @brief Takes ownership of the type-erased value held in @p rhs.
     *
     *  This function is protected so it can be used by the derived class,
     * but is not public to avoid accidental slicing.
     *
     *  @param[in] rhs The instance to move from. After the move operation
     *                 @p rhs is in a valid, but otherwise undefined state.
     *
     *  @throw none No throw guarantee.
     */
    SDEAnyWrapperBase(SDEAnyWrapperBase&&) noexcept = default;

    /** @brief Sets the current state to a deep copy of @p rhs.
     *
     *  This function is protected so it can be used by the derived class,
     * but not public to avoid slicing.
     *
     *  @param[in] rhs The instance to copy.
     *
     *  @return the current instance containing a deep copy of @p rhs's
     * state.
     *
     *  @throw ??? if the copying the value throws. Same guarantee.
     */
    SDEAnyWrapperBase& operator=(const SDEAnyWrapperBase&) = default;

    /** @brief Replaces the current state with the type-erased value held
     *        in @p rhs.
     *
     *  This function is protected so it can be used by the derived class,
     * but is not public to avoid accidental slicing.
     *
     *  @param[in] rhs The instance to move from. After the move operation
     *                 @p rhs is in a valid, but otherwise undefined state.
     *
     *  @return The current instance set containing @p rhs's state.
     *
     *  @throw none No throw guarantee.
     */
    SDEAnyWrapperBase& operator=(SDEAnyWrapperBase&&) noexcept = default;

private:
    /// To be implemented by derived class in order for copying to work
    virtual wrapper_ptr clone_() const = 0;

    /// To be implemented by derived class so we can retrieve the RTTI
    virtual rtti_type type_() const noexcept = 0;

    /// To be implemented by derived class so we can hash
    virtual void hash_(Hasher& h) const = 0;

    /// To be implemented by derived class to make a string representation
    virtual std::string str_() const = 0;

    /// To be implemented by the derived class to define equality
    virtual bool are_equal_(const SDEAnyWrapperBase& rhs) const noexcept = 0;

    /// The type-erased value
    std::any m_value_;
}; // class SDEAnyWrapperBase

/** @brief The class responsible for holding the type-erased instance.
 *
 * The SDEAnyWrapper class holds the wrapped instance for the
 * SDEAnyWrapperBase class.  It also is responsible for implementing all of
 * the abstract methods
 *
 *
 * @tparam T The type of the instance to wrap. Must be copyable, hashable,
 * and have operator== defined.
 */
template<typename T>
class SDEAnyWrapper : public SDEAnyWrapperBase {
private:
    using base_type = SDEAnyWrapperBase;

    template<typename U>
    using enable_if_not_sde_any_t =
      typename base_type::template enable_if_not_wrapper_t<U>;

public:
    /**
     * @brief Creates a new SDEAnyWrapper with the provided value
     *
     * @param[in] value The value we are wrapping.
     *
     * @throw ??? If @p T's ctor throws when @p value is forwarded to it.
     * Strong throw guarantee.
     */
    template<typename U, enable_if_not_wrapper_t<U> = 0>
    explicit SDEAnyWrapper(U&& value) : base_type(std::forward<U>(value)) {}

protected:
    /** @brief Deep copies the type-erased value held in @p rhs.
     *
     *
     *  @param[in] rhs The instance to copy.
     *
     *  @throw ??? if the copying the value throws. Same guarantee.
     */
    SDEAnyWrapper(const SDEAnyWrapper<T>& rhs) = default;

    /** @brief Takes ownership of the type-erased value held in @p rhs.
     *
     *
     *  @param[in] rhs The instance to move from. After the move operation
     *                 @p rhs is in a valid, but otherwise undefined state.
     *
     *  @throw none No throw guarantee.
     */
    SDEAnyWrapper(SDEAnyWrapper<T>&&) noexcept = default;

    /** @brief Sets the current state to a deep copy of @p rhs.
     *
     *
     *  @param[in] rhs The instance to copy.
     *
     *  @return the current instance containing a deep copy of @p rhs's
     * state.
     *
     *  @throw ??? if the copying the value throws. Same guarantee.
     */
    SDEAnyWrapper<T>& operator=(const SDEAnyWrapper<T>&) = default;

    /** @brief Replaces the current state with the type-erased value held
     *        in @p rhs.
     *
     *  @param[in] rhs The instance to move from. After the move operation
     *                 @p rhs is in a valid, but otherwise undefined state.
     *
     *  @return The current instance set containing @p rhs's state.
     *
     *  @throw none No throw guarantee.
     */
    SDEAnyWrapper<T>& operator=(SDEAnyWrapper<T>&&) noexcept = default;

private:
    /// Type of a unique_ptr to the base
    using wrapper_ptr = typename base_type::wrapper_ptr;

    /// Type of the rtti
    using rtti_type = typename base_type::rtti_type;

    /** @brief Code factorization for casting to a read/write reference
     *
     *  @return The wrapped value in a read/write state
     *
     *  @throw none No throw guarantee (because we know the type).
     */
    T& value_() noexcept { return cast<T&>(); }

    /** @brief Code factorization for casting to a read-only reference
     *
     *  @return The wrapped value in a read-only state
     *
     *  @throw none No throw guarantee (because we know the type).
     */
    const T& value_() const noexcept { return cast<const T&>(); }

    /** @brief Deep copies the wrapped value into another
     *
     *  This function ultimately invokes the wrapped instances copy ctor.
     *
     *  @return A newly allocated SDEAnyBase_ instance.
     *
     *  @throw ??? If @p T's copy constructor throws.  Same guarantee as
     *             T's copy constructor.
     */
    wrapper_ptr clone_() const override;

    /** @brief Returns the RTTI of the wrapped instance.
     *
     *  This implementation simply calls `typeid` on @p T and returns the
     *  result.
     *
     *  @return The RTTI of the wrapped instance.
     *
     * @throws None. No throw guarantee.
     */
    rtti_type type_() const noexcept override { return typeid(T); }

    /** @brief Converts the contents to a string.
     *
     *  This function works by using utilties::type_traits::IsPrintable to
     *  determine if type T can be printed. If it can be printed we then
     * pass it to a stringstream and return the result. Otherwise, we print
     * the mangled name and address.
     *
     *  @return A string representation of the wrapped value.
     *
     *  @throw std::bad_alloc if there is insufficient memory to allocate
     * the string. Strong throw guarantee.
     */
    std::string str_() const override;

    /** @brief Implements the equality comparison for the SDEAny
     *
     *  Ultimately both operator== and operator!= will call this function
     * (the latter will negate the result).
     *
     *  @param[in] rhs The instance to compare against for equality.
     *
     *  @return true if the wrapped value compares equal to the value
     * wrapped in @p rhs and false otherwise.
     *
     *  @throw none No throw guarantee.
     */
    bool are_equal_(const SDEAnyWrapperBase& rhs) const noexcept override;

    /** @brief Implements hashing for the SDEAnyBase_ class.
     *
     *  @param[in,out] h The hasher instance to use for the hashing.
     *
     *  @throws ??? if the wrapped instance's hash function throws.  Strong
     *              throw guarantee.
     */
    void hash_(Hasher& h) const override { h(value_()); }
};

//-------------------------------Implementations--------------------------------

template<typename T>
bool SDEAnyWrapperBase::is_convertible() const noexcept {
    return std::any_cast<T>(&m_value_) != nullptr;
}

inline bool SDEAnyWrapperBase::operator==(
  const SDEAnyWrapperBase& rhs) const noexcept {
    return are_equal_(rhs);
}

inline bool SDEAnyWrapperBase::operator!=(
  const SDEAnyWrapperBase& rhs) const noexcept {
    return !((*this) == rhs);
}

template<typename U>
explicit SDEAnyWrapper(U&& value) -> SDEAnyWrapper<std::remove_reference_t<U>>;

template<typename T>
typename SDEAnyWrapper<T>::wrapper_ptr SDEAnyWrapper<T>::clone_() const {
    return std::make_unique<SDEAnyWrapper<T>>(value_());
}

template<typename T>
std::string SDEAnyWrapper<T>::str_() const {
    std::stringstream ss;
    using utilities::printing::operator<<;
    if constexpr(utilities::type_traits::is_printable_v<T>) {
        ss << value_();
    } else {
        ss << "<" << typeid(T).name() << " " << &value_() << ">";
    }
    return ss.str();
}

template<typename T>
bool SDEAnyWrapper<T>::are_equal_(const SDEAnyWrapperBase& rhs) const noexcept {
    try {
        return value_() == rhs.cast<const T&>();
    } catch(...) {
        // Means the cast failed, so @p rhs holds a different type
        return false;
    }
}

} // namespace sde::detail_
