#include "isxTrace.h"

namespace isx
{

template <class T>
Trace<T>::Trace(isx::Time start, isx::Ratio step, isize_t numTimes)
{
    m_timingInfo = isx::TimingInfo(start, step, numTimes);
    m_data = new T[numTimes];
}

template <class T>
Trace<T>::~Trace()
{
    delete[] m_data;
}

template <class T>
T
Trace<T>::getValue(isize_t index) const {
    return m_data[index];
}

template <class T>
void
Trace<T>::setValue(isize_t index, T value) {
    m_data[index] = value;
}

// bogus instantiation to quiet ranlib message isxTrace.cpp.p has no symbols
Trace<int> intTrace(Time(), 0, 0);

} // namespace

