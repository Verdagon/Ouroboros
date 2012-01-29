/*
 * Thread.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: Evan
 */

#include "../TrackedAlloc.h"
#include "Thread.h"

using namespace Threading;

GlobalPerThread<std::string> Threading::threadsNames;

Thread::Thread(const std::string &baseName_) :
		returnValue(NULL),
		threadRunning(false),
		threadID(getNextThreadID()),
		baseName(baseName_) {
	pthread_mutex_init(&threadRunningMutex, NULL);
}

Thread::~Thread() {
	assert(!pthread_equal(pthread_self(), thread)); // Curious, can a thread destroy itself?

	assert(!threadRunning);

	// we'd call a shutdownAndWait here if we could, but since we're in the base class destructor,
	// the subclass destructors have already been called, and they wont be able to properly
	// react to a shutdown event.

	pthread_mutex_destroy(&threadRunningMutex);
}

int Thread::getNextThreadID() {
	static Mutexed<int> nextThreadID(1);
	int result = nextThreadID.lockAndGet()++;
	nextThreadID.unlock();
	return result;
}

struct PThreadMainArgs {
	Thread *thread;
	pthread_mutex_t threadStartedConditionMutex;
	pthread_cond_t threadStartedCondition;
	void *volatile afterJumpResult;

	PThreadMainArgs(Thread *thread_) :
			thread(thread_),
			afterJumpResult(NULL) {
		pthread_mutex_init(&threadStartedConditionMutex, NULL);
		pthread_cond_init(&threadStartedCondition, NULL);
	}

	~PThreadMainArgs() {
		pthread_mutex_destroy(&threadStartedConditionMutex);
		pthread_cond_destroy(&threadStartedCondition);
	}
};

void *Thread::pthreadMain(void *args_) {
	PThreadMainArgs *args = (PThreadMainArgs *)args_;
	Thread *thread = args->thread;

	threadsNames.setMine(thread->getThreadName());

	void *afterJumpResult = thread->afterJumpIntoNewThread();

	pthread_mutex_lock(&args->threadStartedConditionMutex);
	args->afterJumpResult = afterJumpResult;
	pthread_cond_signal(&args->threadStartedCondition);
	pthread_mutex_unlock(&args->threadStartedConditionMutex);
	// At this point, the calling thread may have deleted args.

	pthread_mutex_lock(&thread->threadRunningMutex);
	void *result = thread->main();
	thread->beforeEndingThread();
	pthread_mutex_unlock(&thread->threadRunningMutex);
	return result;
}

void *Thread::jumpIntoNewThreadAndStart() {
	assert(!threadRunning);
	threadRunning = true;

	// We have a condition to make sure that the thread has had a chance to start running.

	struct timespec timeout = { time(NULL) + 3, 0 };

	PThreadMainArgs *args = trackedNew(PThreadMainArgs(this));

	pthread_mutex_lock(&args->threadStartedConditionMutex);

	int error = pthread_create(&thread, NULL, pthreadMain, args);
	assert(error == 0);

	error = pthread_cond_timedwait(&args->threadStartedCondition, &args->threadStartedConditionMutex, &timeout);
	assert(error != ETIMEDOUT);
	assert(error == 0);
	void *afterJumpResult = args->afterJumpResult;
	pthread_mutex_unlock(&args->threadStartedConditionMutex);

	trackedDelete(args);

	return afterJumpResult;
}

bool Thread::join(double timeoutSeconds) {
	int error = 0;

	if (timeoutSeconds < 0) {
		error = pthread_mutex_lock(&threadRunningMutex);
	}
	else {
		struct timespec timeout = { time(NULL) + (int)timeoutSeconds, 0 };
		error = pthread_mutex_timedlock(&threadRunningMutex, &timeout);
		if (error == ETIMEDOUT) {
			return false;
		}
	}

	assert(error == 0);

	pthread_mutex_unlock(&threadRunningMutex);

	pthread_join(thread, &returnValue);

	threadRunning = false;

	return true;
}

bool Thread::shutdownAndJoin(double seconds) {
	assert(!executingInOwnThread()); // Only other threads can wake up the thread
	assert(threadRunning);
	addEvent(trackedNew(ShutdownEvent()));
	return join(seconds);
}

std::string Thread::getThreadName() const {
	return FString("%s#%d", baseName.c_str(), threadID);
}

void *Thread::getReturnValue() {
	assert(!threadRunning);
	return returnValue;
}

void Thread::addEvent(Event *event) {
	mutexedSharedEvents.lockAndGet().push_back(event);
	mutexedSharedEvents.unlock();

	onEventsAdded();
}

Thread::Event *Thread::getWaitingEvent(bool expectEvent) {
	assert(executingInOwnThread()); // Only the thread itself can get its waiting events

	Event *event = NULL;

	std::list<Event *> &sharedEvents = mutexedSharedEvents.lockAndGet();
	if (!sharedEvents.empty()) {
		event = sharedEvents.front();
		sharedEvents.pop_front();
	}
	mutexedSharedEvents.unlock();

	if (expectEvent)
		assert(event != NULL);

	return event;
}

bool Thread::executingInOwnThread() const {
	return pthread_equal(pthread_self(), thread);
}
