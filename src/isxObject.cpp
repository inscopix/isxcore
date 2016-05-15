#include <sstream>

#include "isxObject.h"

namespace isx {

std::string
Object::toString(uint8_t prec) const {
    std::stringstream strm;
    strm.precision(prec);
    strm << *this;
    return strm.str();
}

::std::ostream&
operator<<(::std::ostream& strm, const isx::Object& object) {
    object.serialize(strm);
    return strm;
}

} // namespace
