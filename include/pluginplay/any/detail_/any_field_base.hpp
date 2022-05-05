#pragma once
#include "pluginplay/any/any_field.hpp"
#include <any>
#include <iostream>

namespace pluginplay::any::detail_ {

/** @brief
 */
class AnyFieldBase {
private:
    /// The base class of AnyInput/AnyResult, used to get types
    using any_field_type = AnyField;

public:
    /// How the PIMPL is stored
    using field_base_pointer = typename AnyField::pimpl_pointer;

    /// The type the Any class hierarchy uses for RTTI purposes
    using rtti_type = typename any_field_type::rtti_type;

    /** @brief Polymorphic copy
     *
     *  This method returns a pointer to a newly allocated AnyFieldBase instance
     *  which contains a deep copy of this instance's state. The copied state is
     *  not only the state in the AnyFieldBase class, but also the state in the
     *  derived class as well.
     *
     *  @return A pointer to a newly allocated, polymorphic deep copy of this
     *          instance.
     *
     *  @throw std::bad_alloc if there is a problem allocating the new state.
     *                        Strong throw guarantee.
     */
    field_base_pointer clone() const { return clone_(); }

    /// Standard polymorphic dtor
    virtual ~AnyFieldBase() noexcept = default;

    /** @brief Returns the RTTI for the wrapped object.
     *
     *  This function is actually implemented by calling the virtual type_
     *  function. The derived class (AnyFieldWrapper) is templated on the type
     *  of the wrapped object and implements type_ by simply returning the
     *  typeid of the template parameter.
     *
     *  Note the type that the derived class can wrap is fixed by the derived
     *  class's type, hence the type that this function can return can not
     *  change at runtime.
     *
     *  One should also note that typeid(T) == typeid(std::decay_t<T>) for all
     *  T. Meaning this function can not be used to determine how the value is
     *  stored in the derived class (storing_const_value and
     *  storing_const_reference can be though).
     *
     *  @return The RTTI for the wrapped object.
     *
     *  @throw None No throw guarantee.
     */
    rtti_type type() const noexcept { return type_(); }

    /** @brief Determines if the wrapped value can be retrieved as type @p T
     *
     *  @param T The fully qualified (cv-qualifications and/or reference) type
     *           you would like to get the wrapped value back as.
     *
     *  This overload of `is_convertible` is invoked when the AnyInputBase is
     *  mutable. Whether we can convert the wrapped object to an object of type
     *  @p T depends on which of the following scenarios is true:
     *
     *  1. We wrap the object by mutable value
     *  2. We hold the object in a read-only state (either by value or ref)
     *
     *  For scenerio 1 we can return the object by value, const value,
     *  reference, or const reference. Scenario 2 is essentially the same as
     *  when AnyInputBase is itself read-only and we defer to the const overload
     *  of is_convertible for scenario 2.
     *
     *  @return True if the wrapped instance can be returned as type @p T and
     *          false otherwise.
     *
     *  @throw None No throw guarantee.
     */
    template<typename T>
    bool is_convertible() noexcept;

    /** @brief Determines if the wrapped value can be retrieved as type @p T
     *
     *  @param T The fully qualified (cv-qualifications and/or reference) type
     *           you would like to get the wrapped value back as.
     *
     *  This overload of `is_convertible` is invoked when the AnyInputBase is
     *  read-only. Since the AnyInputBase is read-only it will only allow you
     *  to retrieve the wrapped value in a manner that does not allow you to
     *  modify it. Namely this function will return true if you request to get
     *  the value back by value, const value, or read-only reference (and if
     *  the wrapped value is actually of type `std::decay_t<T>`).
     *
     *  @return True if the wrapped instance can be returned as type @p T and
     *          false otherwise.
     *
     *  @throw None No throw guarantee.
     */
    template<typename T>
    bool is_convertible() const noexcept;

    bool storing_const_value() const noexcept { return storing_const_value_(); }

    bool storing_const_reference() const noexcept {
        return storing_const_ref_();
    }

    /** @brief Polymophically compares two objects derived from AnyFieldBase.
     *
     *  This function is actually implemented by symmetrically calling
     *  `are_equal_` (*i.e.* checking
     *  `this->are_equal_(rhs) && rhs.are_equal_(*this)`). `are_equal_` is a
     *  virtual function which the derived class implements to check if:
     *
     *  1. The input instance can be downcast to the derived class, and
     *  2. that the states in the derived classes are the same.
     *
     *  By calling it symmetrically we guarantee that both this instance and
     *  @p rhs have the same most derived class. Of note for our purposes this
     *  guarantees that both this instance and @p rhs are both wrapping the
     *  same value in the same way (by value, by ref, by const val, etc.)
     *
     *  @param[in] rhs The instance we are comparing to.
     *
     *  @return True if the most derived type of this instance compares equal to
     *               the most derived type of @p rhs, and if the state of each
     *               type in this instance's hierarchy compares equal to the
     *               state of teh corresponding type in @p rhs's hierarchy.
     *
     *  @throw None No throw guarantee.
     *
     */
    bool are_equal(const AnyFieldBase& rhs) const noexcept {
        return are_equal_(rhs) && rhs.are_equal_(*this);
    }

    /** @brief Determines if the value wrapped by two AnyField instances are
     *         equal.
     *
     *  The difference between this and `are_equal` is say we have an
     *  AnyFieldWrapper<T> and an AnyFieldWrapper<T&> (T being an unqualified
     *  type). `are_equal` will always return false because the derived classes
     *  of the hierarchy are different types. `value_equal`, however, will call
     *  `T::operator==` on the wrapped values to determine if they are equal.
     *
     *  Conceptually this method is sort of a non-polymorphic equality
     *  operator in the sense that it doesn't take into account the state of the
     *  derived class (in so far that the value is stored in this class and the
     *  type of the value is stored in the derived class). Rigorously there is
     *  no way for us to compare the values without going into the derived class
     *  so this comparison does have some polymorphic aspects to it.
     *
     */
    bool value_equal(const AnyFieldBase& rhs) const noexcept {
        return value_equal_(rhs);
    }

    bool value_less(const AnyFieldBase& rhs) const noexcept {
        return value_less_(rhs);
    }

    /** @brief Adds a text representation of the wrapped object to @p os
     *
     *  This function is actually implemented by calling the virtual function
     *  print_. The derived class then uses basic template meta-programming to
     *  determine if the wrapped type is capable of being printed to a
     *  std::ostream. If it is, the wrapped type is inserted into the stream. If
     *  it is not, the type of the object and its address are inserted into the
     *  stream. Either way the modified stream is returned to support chaining.
     *
     *  @param[in,out] os The stream we are adding a text representation to.
     *                    After this call @p os will contain a text
     *                    representation of the wrapped object.
     *
     *  @return @p os After adding the text representation of the current object
     *                to it.
     *
     *  @throw std::ios_base::failure if there is a problem writing to the
     *                                stream. Weak throw guarantee (the wrapped
     *                                object is still in the same state, but
     *                                @p os is a potentially different state).
     *  @throw ??? If calling `std::ostream::operator<<` on the wrapped type
     *             throws. Same throw guarantee.
     */
    std::ostream& print(std::ostream& os) const { return print_(os); }

    /** @brief Retrieves the value as an instance of type T.
     *
     *  @tparam T The exact type to retrieve the value as. @p T should include
     *            the desired cv-qualifiers and whether it is to be a reference
     *            or not.
     *
     *  Based on how the ctors for the AnyFieldBase class hierarchy work, each
     *  instance must always wrap a value (if an AnyField does not wrap a value
     *  it simply holds a nullptr). This function can be used to
     *  retrieve the wrapped value as an object of type @p T, assuming the
     *  wrapped object can be converted to an object of type @p T. See the
     *  is_convertible method for details on what conversions are allowed.
     *
     *  This overload is selected when the AnyFieldBase is mutable.
     *
     *  @return The wrapped instance as instance of type @p T.
     *
     *  @throw std::runtime_error if the wrapped instance can not be retrieved
     *                            as an instance of type @p T as determined by
     *                            the is_convertible method.
     *
     */
    template<typename T>
    T cast();

    /** @brief Retrieves the value as an instance of type T.
     *
     *  @tparam T The exact type to retrieve the value as. @p T should include
     *            the desired cv-qualifiers and whether it is to be a reference
     *            or not.
     *
     *  Based on how the ctors for the AnyFieldBase class hierarchy work, each
     *  instance must always wrap a value (if an AnyField does not
     *  wrap a value it simply holds a nullptr). This function can be used to
     *  retrieve the wrapped value as an object of type @p T, assuming the
     *  wrapped object can be converted to an object of type @p T. See the
     *  is_convertible method for details on what conversions are allowed.
     *
     *  This overload is selected when the AnyFieldBase is read-only
     *
     *  @return The wrapped instance as instance of type @p T.
     *
     *  @throw std::runtime_error if the wrapped instance can not be retrieved
     *                            as an instance of type @p T as determined by
     *                            the is_convertible method.
     *
     */
    template<typename T>
    T cast() const;

protected:
    /// AnyWrappers are always created via AnyResultWrapper/AnyInputWrapper
    ///@{
    AnyFieldBase(std::any da_any) : m_value_(std::move(da_any)) {}
    AnyFieldBase(const AnyFieldBase& other) = delete;
    ///@}

private:
    /// Deleted to avoid slicing
    ///@{
    AnyFieldBase(AnyFieldBase&& other) = delete;
    AnyFieldBase& operator=(const AnyFieldBase&) = delete;
    AnyFieldBase& operator=(AnyFieldBase&&) = delete;
    ///@}

    /// To be overriddent to implement clone
    virtual field_base_pointer clone_() const = 0;

    /// To be overridden by derived class to implement polymorphic equality
    virtual bool are_equal_(const AnyFieldBase& rhs) const noexcept = 0;

    /// To be overridden by derived class to implemet value_equal
    virtual bool value_equal_(const AnyFieldBase& rhs) const noexcept = 0;

    /// To be overridden by derived class to implement value_less
    virtual bool value_less_(const AnyFieldBase& rhs) const noexcept = 0;

    /// To be overridden by derived class to implement printing
    virtual std::ostream& print_(std::ostream& os) const = 0;

    /// To be overridden by derived class to implement type
    virtual rtti_type type_() const noexcept = 0;

    /// To be overridden by derived class to implement storing_const_ref
    virtual bool storing_const_ref_() const noexcept = 0;

    /// To be overridden by derived class to implement storing_const_value
    virtual bool storing_const_value_() const noexcept = 0;

    /// Code factorization for throwing when we can't convert to T
    template<typename T>
    void error_if_not_convertible_() const;

    /// The type-erased value
    std::any m_value_;
};

} // namespace pluginplay::any::detail_

#include "any_field_base.ipp"
