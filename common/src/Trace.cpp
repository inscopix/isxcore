#include "Trace.h"

namespace mosaic {

template <class T>
Trace<T>::Trace(QDateTime start, qint32 nTimes, qint16 step) {
    this->domain = mosaic::TimeGrid(start, nTimes, step);
}

template <class T>
Trace<T>::~Trace() {

}

template <class T>
T Trace<T>::getValue(qint32 i) const {
    return range[i];
}

template <class T>
void Trace<T>::setValue(qint32 i, T val) {
    this->range[i] = val;
}

}

