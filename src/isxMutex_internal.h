#ifndef ISX_MUTEX_INTERNAL
#define ISX_MUTEX_INTERNAL

#include "isxMutex.h"
#include <QMutex>


namespace isx
{
	/// A wrapper around QMutex
	///
	class Mutex::Impl : public QMutex
	{
	public: 
		/// Constructor
		///
		Impl(){}

		/// Destructor
		///
		~Impl(){}

	};
}

#endif // ISX_MUTEX_INTERNAL
