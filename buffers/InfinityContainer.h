/*
 * PrevIndexBuffer.h
 *
 *  Created on: Nov 16, 2024
 *      Author: admin
 */

#ifndef CHAOS_PP_BUFFERS_INFINITYCONTAINER_H_
#define CHAOS_PP_BUFFERS_INFINITYCONTAINER_H_

#include <basic_types.h>

class InfinityContainer
{
public:
	InfinityContainer() = default;
	~InfinityContainer();

	bool init(const u16 depth, const reg size);

	// write logic -----------------------------------------------------
	reg write(const void* data, const reg len);
	void* const getWriteBuffer();
	inline void writeAct() { ++m_wrIndex; }

	// read logic -----------------------------------------------------
	reg read(void* const data, const reg len);
	void* const getReadBuffer();
	inline void readAct() { m_rdIndex = m_wrIndex; }
	inline bool readIsExist() const { return m_rdIndex != m_wrIndex; }


	// info logic -----------------------------------------------------
	inline reg capacity() const { return m_capacity; }
	inline reg depth() const { return m_depth; }

private:
	void** m_pool 			= nullptr;  // Pointer array for buffers
	volatile reg m_wrIndex 	= 0;		// write position
	volatile reg m_rdIndex 	= 0;		// read position
	reg m_msk 				= 0;		// write mask (m_n_buffers - 1)

	// buffer info
	u16 m_depth 	= 0;      	// Number of buffers
    reg m_capacity 	= 0;      	// Maximum size of each buffer
};

#endif /* CHAOS_PP_BUFFERS_INFINITYCONTAINER_H_ */
