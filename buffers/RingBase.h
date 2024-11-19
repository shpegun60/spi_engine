/*
 * RingBase.h
 *
 *  Created on: Jul 16, 2024
 *      Author: admin
 */

#ifndef CHAOSPP_RINGBASE_H_
#define CHAOSPP_RINGBASE_H_

#include <basic_types.h>
#include "irq/irq_block.h"

class RingBase
{
protected:
	RingBase() = default;
	explicit RingBase(const reg capacity);
	~RingBase() = default;

    bool init(const reg capacity);

public:
    inline void clear() {
    	IRQ_LOCK();
    	tail = 0;
    	head = 0;
    	IRQ_UNLOCK();
    }
    inline bool isEmpty() const { return tail == head; }
    inline bool isFull() const { return (((head ^ tail) & xor_msk) == cap); }
    inline bool islast() const { return length() == 1; }
    inline reg length() const { return head - tail; }

    inline reg capacity() const { return cap; }
    inline reg getMask() const { return msk; }
    inline reg getHead() const { return head; }
    inline reg getTail() const { return tail; }

protected:
    inline void setHead(const reg new_head) {head = new_head; }
    inline void incrementHead() {++head;}
    inline void setTail(const reg new_tail) {tail = new_tail; }
    inline void incrementTail() {++tail; }

public:
    static inline constexpr bool is_power_of_2(const reg x) {
        return (x != 0) && ((x & (x - 1)) == 0);
    }

private:
    reg cap = 0;       			// Buffer capacity power of 2
    reg msk = 0;       			// (cap - 1)
    reg xor_msk = 0;   			// xor mask  (cap + msk)
    volatile reg tail = 0;		// tail register
    volatile reg head = 0;		// head register
};



#endif /* CHAOSPP_RINGBASE_H_ */
