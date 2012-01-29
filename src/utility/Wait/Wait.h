/*
 * Waiter.h
 *
 *  Created on: Dec 18, 2011
 *      Author: Evan
 */

#ifndef WAITER_H_
#define WAITER_H_

#include "../Utility.h"
#include <algorithm>

class Waitable : public Casting {
public:
	virtual ~Waitable() { }
};

class Waiter {
public:
	virtual void addWaitable(Waitable *waitable) = 0;
	virtual std::pair<Waitable *, int> waitUntil(double appTimeEnd = 0) = 0;

	static Waiter *create();
};

#endif /* WAITER_H_ */
