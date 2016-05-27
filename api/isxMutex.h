#ifndef ISX_MUTEX_H
#define ISX_MUTEX_H

#include <memory>
#include <mutex>

namespace isx
{
	///
	/// A class implementing a Mutex.
	///

	class Mutex
	{
	public:
		// Methods 

		/// Default constructor
		///
		Mutex();

		/// Default Destructor
		/// If locked when going out of scope, the mutex is unlocked 
		///
		~Mutex();

		/// lock mutex
		///
		void lock();

		/// unlock mutex
		///
		void unlock();

	private:

		// Attributes
		bool m_bIsLocked;

		class Impl;
		std::unique_ptr<Impl> m_internal;
	};
}

#endif // ISX_MUTEX_H
