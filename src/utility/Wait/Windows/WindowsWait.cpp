/*
 * Waiter.cpp
 *
 *  Created on: Dec 18, 2011
 *      Author: Evan
 */

#include "WindowsWait.h"
#include "../../vassert.h"
#include "../../Threading/Thread.h"
#include "../../Clock.h"
#include "../../TrackedAlloc.h"

#include <TCHAR.h>

class WaitForMultipleObjectsWEndTimeInSInaccurateSleepingFunctor {
	Threading::Mutexed<float> mutexedAverageLatenessSeconds;

public:
	WaitForMultipleObjectsWEndTimeInSInaccurateSleepingFunctor() :
		mutexedAverageLatenessSeconds(0) { }

	float getAverageLatenessSeconds() { return mutexedAverageLatenessSeconds.lockGetAndUnlock(); }

	DWORD operator()(int numEvents, HANDLE events[], bool waitAll, double endTimeInS) {
		int desiredWaitInMS = (endTimeInS == 0 ? INFINITE : (endTimeInS - Clock().getElapsed()) * 1000);

		DWORD wfmoResult = WaitForMultipleObjects(numEvents, events, waitAll, desiredWaitInMS);
		double actualEndTimeInS = Clock().getElapsed();

		float lateByInS = actualEndTimeInS - endTimeInS;

		if (lateByInS > 0) {
			float &averageLatenessSeconds = mutexedAverageLatenessSeconds.lockAndGet();
			averageLatenessSeconds = (averageLatenessSeconds * 9 + lateByInS) / 10;
			mutexedAverageLatenessSeconds.unlock();
		}

		return wfmoResult;
	}
} static WaitForMultipleObjectsWEndTimeInSInaccurateSleeping;

DWORD WaitForMultipleObjectsWEndTimeInSAccurateAndExpensive(int numEvents, HANDLE events[], bool waitAll, double endTimeInS) {
	while (true) {
		if (Clock().getElapsed() >= endTimeInS) {
			return WAIT_TIMEOUT;
		}
		DWORD result = WaitForMultipleObjects(numEvents, events, waitAll, 0);
		if (result != WAIT_TIMEOUT)
			return result;
		Sleep(0);
	}
}

//DWORD WaitForMultipleObjectsAdjusted(int numEvents, HANDLE events[], bool waitAll, int desiredWaitInMS) {
////	if (desiredWaitInMS > waitForMultipleObjectsWithTimeout.getAverageLatenessSeconds() * 1000)
////		return waitForMultipleObjectsWithTimeout(numEvents, events, waitAll, desiredWaitInMS - waitForMultipleObjectsWithTimeout.getAverageLatenessSeconds() * 1000);
////	else
//		return WaitForMultipleObjectsAccurateAndExpensive(numEvents, events, waitAll, desiredWaitInMS);
//}

DWORD WaitForMultipleObjectsAdjustedWEndTimeInS(int numEvents, HANDLE events[], bool waitAll, double appEndTime) {
	if (appEndTime == 0)
		return WaitForMultipleObjectsWEndTimeInSInaccurateSleeping(numEvents, events, waitAll, appEndTime);

	if (appEndTime - Clock().getElapsed() > WaitForMultipleObjectsWEndTimeInSInaccurateSleeping.getAverageLatenessSeconds()) {
		DWORD result = WaitForMultipleObjectsWEndTimeInSInaccurateSleeping(numEvents, events, waitAll, appEndTime - WaitForMultipleObjectsWEndTimeInSInaccurateSleeping.getAverageLatenessSeconds());
		if (result != WAIT_TIMEOUT)
			return result;
	}

	return WaitForMultipleObjectsWEndTimeInSAccurateAndExpensive(numEvents, events, waitAll, appEndTime);
}




std::string GetLastErrorMessage(DWORD last_error, bool stripTrailingLineFeed) {
	TCHAR errmsg[512];

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, last_error, 0, errmsg, 511, NULL)) {
		/* if we fail, call ourself to find out why and return that error */
		return (GetLastErrorMessage(GetLastError(), stripTrailingLineFeed));
	}

	if (stripTrailingLineFeed) {
		const size_t length = _tcslen(errmsg);

		if (errmsg[length-1] == '\n') {
			errmsg[length-1] = 0;

			if (errmsg[length-2] == '\r')
				errmsg[length-2] = 0;
		}
	}

	return std::string(errmsg);
}

Waiter *Waiter::create() { return trackedNew(WindowsWaiter()); }

void WindowsWaiter::addWaitable(Waitable *waitable) {
	waitables.push_back(waitable->requiredAs<WindowsWaitable>());
}

std::pair<Waitable *, int> WindowsWaiter::waitUntil(double appTimeEnd) {
	HANDLE *events = new HANDLE[waitables.size()];
	for (unsigned int i = 0; i < waitables.size(); i++)
		events[i] = waitables[i]->beforeWaitStarted();

	DWORD wfmoResult = WaitForMultipleObjectsAdjustedWEndTimeInS(waitables.size(), events, false, appTimeEnd);

	WindowsWaitable *firedWaitable = NULL;
	int onFireResult = 0;

	if (wfmoResult >= WAIT_OBJECT_0 && wfmoResult < WAIT_OBJECT_0 + waitables.size()) {
		int waitableIndex = wfmoResult - WAIT_OBJECT_0;
		firedWaitable = waitables[waitableIndex];
		onFireResult = firedWaitable->onFire(events[waitableIndex]);
	}
	else if (wfmoResult == WAIT_TIMEOUT) {
	}
	else {
		std::cout << GetLastErrorMessage(GetLastError(), true) << std::endl;
		assert(false);
	}

	for (unsigned int i = 0; i < waitables.size(); i++) {
		waitables[i]->afterWaitFinished(events[i]);
	}

	delete[] events;

	return std::pair<Waitable *, int>(firedWaitable, onFireResult);
}
