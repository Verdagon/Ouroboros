/*
 * WindowsInterruptor.cpp
 *
 *  Created on: Dec 31, 2011
 *      Author: Evan
 */


#include "WindowsInterruptor.h"
#include "../../TrackedAlloc.h"

Threading::Interruptor *Threading::Interruptor::create() { return trackedNew(WindowsInterruptor()); }
