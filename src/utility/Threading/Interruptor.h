/*
 * Interruptor.h
 *
 *  Created on: Dec 31, 2011
 *      Author: Evan
 */

#ifndef INTERRUPTOR_H_
#define INTERRUPTOR_H_

#include "../Wait/Wait.h"

namespace Threading {

	class Interruptor : public virtual Waitable {
	public:
		virtual void interrupt() = 0;

		static Interruptor *create();
	};

}

#endif /* INTERRUPTOR_H_ */
