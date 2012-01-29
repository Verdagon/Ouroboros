/*
 * Thread.h
 *
 *  Created on: Oct 24, 2011
 *      Author: Evan
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include "../Utility.h"
#include "../Wait/Wait.h"
#include "../Threading/ThreadGlobals.h"
#include "../Threading/Mutex.h"

namespace Threading {

//	class IThread {
//	public:
//		virtual void *jumpIntoNewThreadAndStart() = 0;
//
//		virtual bool running() const = 0;
//		virtual std::string getThreadName() const = 0;
//
//		virtual bool join(double timeout = -1) = 0;
//		virtual bool shutdownAndJoin(double timeout = -1) = 0;
//		virtual void *getReturnValue() = 0;
//
//		virtual ~Thread() { }
//	};

	class Thread {// : public IThread {
	public:
		class Event : public Casting { };

	private:
		void *returnValue;

		bool threadRunning;

		Mutexed<std::list<Event *> > mutexedSharedEvents;

		static int getNextThreadID();

	protected:
		class ShutdownEvent : public Event { };

		bool executingInOwnThread() const;

		static void *pthreadMain(void *);

		Event *getWaitingEvent(bool expectEvent = true);

		virtual void onEventsAdded() { } // Usually for waking up a thread that needs to know about events

		virtual void *afterJumpIntoNewThread() { return NULL; }
		virtual void beforeEndingThread() { }
		virtual void *main() = 0;

		// returns whether or not the event has been handled.
		virtual bool handleEvent(Event *event) { return false; }

		pthread_t thread;
		pthread_mutex_t threadRunningMutex;

		void addEvent(Event *message);

		const int threadID;
		const std::string baseName;

	public:
		Thread(const std::string &baseName);

		// TODO: add a timeout to this method
		void *jumpIntoNewThreadAndStart();

		inline bool running() const { return threadRunning; }
		std::string getThreadName() const;

		bool join(double timeout = -1);
		bool shutdownAndJoin(double timeout = -1);
		void *getReturnValue();

		virtual ~Thread();
	};

	extern GlobalPerThread<std::string> threadsNames;

}

#endif /* THREAD_H_ */


//		template<typename EventType>
//		EventType *getWaitingEventOfType(bool expectEvent = true) {
//			assert(executingInOwnThread()); // Only the thread itself can get its waiting events
//
//			EventType *event = NULL;
//
//			std::list<Event *> &sharedEvents = mutexedSharedEvents.lockAndGet();
//
//				for (std::list<Event *>::iterator i = sharedEvents.begin(), iEnd = sharedEvents.end(); event == NULL && i != iEnd; i++)
//					if ((*i)->is<EventType>())
//						event = *i;
//
//				if (event)
//					sharedEvents.remove(event);
//
//			mutexedSharedEvents.unlock();
//
//			if (expectEvent)
//				assert(event != NULL);
//			return event;
//		}
//
//		template<typename EventType>
//		Event *getWaitingEventPrioritizingType(bool expectEvent = true) {
//			assert(executingInOwnThread()); // Only the thread itself can get its waiting events
//
//			Event *event = NULL;
//
//			std::list<Event *> &sharedEvents = mutexedSharedEvents.lockAndGet();
//
//				for (std::list<Event *>::iterator i = sharedEvents.begin(), iEnd = sharedEvents.end(); event == NULL && i != iEnd; i++)
//					if ((*i)->is<EventType>())
//						event = *i;
//
//				for (std::list<Event *>::iterator i = sharedEvents.begin(), iEnd = sharedEvents.end(); event == NULL && i != iEnd; i++)
//					event = *i;
//
//				if (event)
//					sharedEvents.remove(event);
//
//			mutexedSharedEvents.unlock();
//
//			if (expectEvent)
//				assert(event != NULL);
//			return event;
//		}
