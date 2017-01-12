#include "isxVariant.h"
#include "isxJsonUtils.h"
#include "isxException.h"

namespace isx {

class Variant::Impl
{
public:
    
    Impl(){}

    Impl(float inValue)
    {
        m_metatype = MetaType::FLOAT;
        m_value = inValue;
    }

    Impl(const Time & inValue)
    {
        m_metatype = MetaType::ISXTIME;
        m_value = convertTimeToJson(inValue);
    }


    ~Impl(){}

    template <typename T>
    void 
    setValue(const T & inValue)
    {
        // Unspecialized template function
        ISX_THROW(ExceptionUserInput, "The type of the provided value is not supported. See isx::Variant::MetaType for supported types.");
    }    

    MetaType
    getType() const
    {
        return m_metatype;
    }

    template <typename T>
    T 
    value() const
    {
        ISX_THROW(ExceptionUserInput, "The type of the provided value is not supported. See isx::Variant::MetaType for supported types.");
        
        T value;
        return value;
    }

    std::string
    toString() const
    {
        return m_value.dump();
    }

    bool
    operator ==(const Impl & other) const
    {
        return (m_value == other.m_value);
    }

    bool
    operator !=(const Impl & other) const
    {
        return (m_value != other.m_value);
    }

    void 
    operator = (const Impl & right)
    {
        m_metatype = right.m_metatype;
        m_value = right.m_value;
    }    

private: 

    json        m_value;
    MetaType    m_metatype = MetaType::NONE;

};

/// Template function specialization for float
template<>
void 
Variant::Impl::setValue(const float & inValue)
{
    m_metatype = MetaType::FLOAT;
    m_value = inValue;
}

/// Template function specialization for Time
template<>
void 
Variant::Impl::setValue(const isx::Time & inValue)
{
    m_metatype = MetaType::ISXTIME;
    m_value = convertTimeToJson(inValue);
}

/// Template function specialization for float
template <>
float 
Variant::Impl::value() const
{
    if (m_metatype != MetaType::FLOAT)
    {
        ISX_THROW(ExceptionUserInput, "The type of the stored value cannot be converted to float.");
    }
    
    float value = m_value;

    return value;
}

/// Template function specialization for Time
template <>
Time 
Variant::Impl::value() const
{    
    if(m_metatype != MetaType::ISXTIME)
    {
        ISX_THROW(ExceptionUserInput, "The type of the stored value cannot be converted to Time.");
    }
    Time value = convertJsonToTime(m_value);  
    return value;
}

Variant::Variant()
{
    m_pImpl.reset(new Impl());
}

Variant::Variant(float inValue)
{
    m_pImpl.reset(new Impl(inValue));
}

Variant::Variant(const Time & inValue)
{
    m_pImpl.reset(new Impl(inValue));
}

Variant::Variant(const Variant & inOther) :
    Variant()
{
    MetaType type = inOther.getType();
    if (type == MetaType::FLOAT)
    {
        setValue(inOther.value<float>());
    }
    else if (type == MetaType::ISXTIME)
    {
        setValue(inOther.value<isx::Time>());
    }

}

Variant::~Variant()
{

}

template <typename T>
void 
Variant::setValue(const T & inValue)
{
    m_pImpl->setValue(inValue);
}

Variant::MetaType
Variant::getType() const
{
    return m_pImpl->getType();
}

template <typename T>
T 
Variant::value() const
{
    return m_pImpl->value<T>();
}

std::string
Variant::toString() const
{
    return m_pImpl->toString();
}

bool 
Variant::operator ==(const Variant & other) const
{
    return *(other.m_pImpl) == *m_pImpl;    
}

bool 
Variant::operator !=(const Variant & other) const
{
    return *(other.m_pImpl) != *m_pImpl;
}

void Variant::operator = (const Variant & right)
{
    *m_pImpl = *(right.m_pImpl);
}


} // namespace isx