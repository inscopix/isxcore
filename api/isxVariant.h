#ifndef ISX_VARIANT_H
#define ISX_VARIANT_H

#include "isxTime.h"
#include <memory>
#include <string>

namespace isx
{

/// Encapsulates a variant type.
/// 
class Variant
{
public:

    /// Supported metatypes codes
    /// 
    enum class MetaType 
    {
        FLOAT,
        ISXTIME,
        STRING,
        INT64,
        NONE
    };

    /// Constructor for an empty Variant
    ///
    Variant();

    /// Constructor for float value
    ///
    explicit Variant(float inValue);

    /// Constructor for Time value
    ///
    explicit Variant(const Time & inValue);

    /// Constructor for a string value
    ///
    explicit Variant(const std::string & inValue);
    
    /// Constructor for an int64_t value
    ///
    explicit Variant(int64_t inValue);
    
    /// Copy constructor
    /// \param inOther the other Variant to copy
    Variant(const Variant & inOther);

    /// Destructor
    ///
    ~Variant();

    /// Set the value stored in the Variant
    /// \param inValue
    /// \throw isx::ExceptionUserInput in case the type to store is unsupported 
    template <typename T>
    void 
    setValue(const T & inValue);

    /// \return the type of the data stored in this Variant
    ///
    MetaType
    getType() const;

    /// \return the value stored in this Variant
    /// \throw isx::ExceptionUserInput in case the requested type is unsupported
    /// or if the stored type cannot be converted to the requested type 
    template <typename T>
    T 
    value() const;

    /// Serialize to string 
    /// 
    std::string
    toString() const;
    

    /// \return     True if this is exactly equal to another variant, false otherwise.
    ///
    bool operator ==(const Variant & other) const;

    /// \return     True if this is not exactly equal to another variant, false otherwise.
    ///
    bool operator !=(const Variant & other) const;

    /// \ assignment operator
    ///
    void operator = (const Variant & right);    

private: 
    class Impl;
    /// Internal implementation of Variant class
    ///
    std::unique_ptr<Impl> m_pImpl;
}; // class Variant


} // namespace isx

#endif //ISX_VARIANT_H