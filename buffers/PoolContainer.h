/*
 * PoolContainer.h
 *
 *  Created on: Jul 16, 2024
 *      Author: admin
 */

#ifndef CHAOSPP_POOLCONTAINER_H_
#define CHAOSPP_POOLCONTAINER_H_

#include "RingBase.h"

class PoolContainer : public RingBase
{
public:
    PoolContainer() = default;
    ~PoolContainer();

    bool init(const u16 depth, const reg max_size);

    // write logic -----------------------------------------------------
    reg write(const void* data, const reg len);
    void* const getWriteBuffer();
    inline void writeAct() { incrementHead(); }

    // read logic -----------------------------------------------------
    reg read(void* const data, const reg len);
    void* const getReadBuffer();
    inline void readAct() { incrementTail(); }

    // info logic -----------------------------------------------------
    inline reg capacity() const { return m_capacity; }
    inline reg depth() const { return RingBase::capacity(); }

private:
    void** m_pool 	= nullptr;  // Pointer array for buffers
    reg m_capacity 	= 0;       	// Maximum size of each buffer
};


#endif /* CHAOSPP_POOLCONTAINER_H_ */
