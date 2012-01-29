/*
 * Globals.h
 *
 *  Created on: Apr 8, 2011
 *      Author: Evan
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <iostream>
#include <list>
#include <algorithm>
#include <pthread.h>
#include "../vassert.h"

namespace Threading {

	template<typename T>
	class GlobalPerThread {
		std::list<std::pair<pthread_t, const T> > globalsByThread;

	public:
		const T &mine() const {
			assert(mineIsSet());

			pthread_t self = pthread_self();
			for (typename std::list<std::pair<pthread_t, const T> >::const_iterator i = globalsByThread.begin(); i != globalsByThread.end(); i++)
				if (pthread_equal(i->first, self))
					return i->second;
			assert(false);
		}

		bool mineIsSet() const {
			pthread_t self = pthread_self();
			for (typename std::list<std::pair<pthread_t, const T> >::const_iterator i = globalsByThread.begin(); i != globalsByThread.end(); i++)
				if (pthread_equal(i->first, self))
					return true;
			return false;
		}

		void setMine(const T &newValue) {
			assert(!mineIsSet()); // If this fails, you probably called setGlobals twice in the same thread
			globalsByThread.push_back(std::pair<pthread_t, T>(pthread_self(), newValue));
		}
	};

}

#endif /* GLOBALS_H_ */
