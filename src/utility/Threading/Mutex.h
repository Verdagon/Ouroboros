/*
 * Mutex.h
 *
 *  Created on: Jan 27, 2012
 *      Author: Evan
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

namespace Threading {

	class Mutex {
		pthread_mutex_t mutex;
		bool locked;

		Mutex(const Mutex &that) { assert(false); } // implement

	public:
		Mutex() : locked(false) {
			pthread_mutex_init(&mutex, NULL);
		}

		~Mutex() {
			pthread_mutex_destroy(&mutex);
		}

		void lock() {
			pthread_mutex_lock(&mutex);
			locked = true;
		}

		void unlock() {
			assert(locked);
			locked = false;
			pthread_mutex_unlock(&mutex);
		}
	};

	template<typename T>
	class Mutexed {
		pthread_mutex_t mutex;
		bool locked;
		T value;

		Mutexed(const Mutexed &that) { assert(false); } // implement

	public:
		Mutexed(const T &value_ = T()) :
				locked(false),
				value(value_) {
			pthread_mutex_init(&mutex, NULL);
		}
		~Mutexed() {
			assert(!locked);
			pthread_mutex_destroy(&mutex);
		}

		T &lockAndGet() {
			pthread_mutex_lock(&mutex);
			assert(!locked);
			locked = true;
			return value;
		}

		void unlock() {
			assert(locked);
			locked = false;
			pthread_mutex_unlock(&mutex);
		}

		T &lockGetAndUnlock() {
			T &result = lockAndGet();
			unlock();
			return result;
		}

		void lockSetAndUnlock(const T &value_) {
			lockAndGet() = value_;
			unlock();
		}
	};

}


#endif /* MUTEX_H_ */
