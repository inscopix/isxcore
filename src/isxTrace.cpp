#include "isxTrace.h"

namespace isx {

template <class T>
Trace<T>::Trace(isx::Time start, uint32_t numTimes, uint16_t step) {
    m_Domain = isx::TimeGrid(start, numTimes, step);
}

template <class T>
T
Trace<T>::getValue(uint32_t i) const {
    return m_Range[i];
}

template <class T>
void
Trace<T>::setValue(uint32_t i, T val)
{
    m_Range[i] = val;
}

} // namespace

