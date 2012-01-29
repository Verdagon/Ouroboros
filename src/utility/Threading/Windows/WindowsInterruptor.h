/*
 * WindowsInterruptor.h
 *
 *  Created on: Dec 31, 2011
 *      Author: Evan
 */

#ifndef WINDOWSINTERRUPTOR_H_
#define WINDOWSINTERRUPTOR_H_

#include "../../Threading/Interruptor.h"
#include "../../Wait/Windows/WindowsWait.h"

namespace Threading {

	class WindowsInterruptor : public Interruptor, public WindowsWaitable {
		HANDLE event;
	public:
		WindowsInterruptor() { event = CreateEvent(NULL, FALSE, FALSE, NULL); }
		~WindowsInterruptor() { CloseHandle(event); }
		virtual HANDLE beforeWaitStarted() { return event; }
		void interrupt() { SetEvent(event); }
	};

}

#endif /* WINDOWSINTERRUPTOR_H_ */
