/*
 * RingBase.cpp
 *
 *  Created on: Jul 16, 2024
 *      Author: admin
 */

#include "RingBase.h"


RingBase::RingBase(const reg capacity)
{
	init(capacity);
}


bool RingBase::init(const reg capacity)
{
	if (!is_power_of_2(capacity)) {
		return false;
	}

	IRQ_LOCK();

	cap = capacity;
	msk = (capacity - 1);
	xor_msk = (cap | msk);
	tail = 0;
	head = 0;

	IRQ_UNLOCK();

	return true;
}
