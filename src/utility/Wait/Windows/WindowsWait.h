/*
 * Waiter.h
 *
 *  Created on: Dec 18, 2011
 *      Author: Evan
 */

#ifndef WINWAITER_H_
#define WINWAITER_H_

#include "../Wait.h"
#include <windows.h>
#include <vector>

class WindowsWaitable : public virtual Waitable {
public:
	virtual HANDLE beforeWaitStarted() = 0;
	virtual void afterWaitFinished(HANDLE handle) { }
	virtual int onFire(HANDLE handle) { return 0; }
};

class WindowsWaiter : public Waiter {
private:
	std::vector<WindowsWaitable *> waitables;

public:
	virtual void addWaitable(Waitable *waitable);
	virtual std::pair<Waitable *, int> waitUntil(double appTimeEnd = 0);
};


#endif /* WAITER_H_ */
