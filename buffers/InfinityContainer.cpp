/*
 * PrevIndexBuffer.cpp
 *
 *  Created on: Nov 16, 2024
 *      Author: admin
 */

#include "InfinityContainer.h"
#include <cstring>
#include <cstdlib> // For malloc and free

#include "RingBase.h"

InfinityContainer::~InfinityContainer()
{
	if (m_pool) {
		for (u16 i = 0; i < m_depth; ++i) {
			std::free(m_pool[i]);
		}

		std::free(m_pool);
		m_pool = nullptr;
	}
}

bool InfinityContainer::init(const u16 depth, const reg size)
{
	if (!RingBase::is_power_of_2(depth)) {
		return false;
	}

	m_pool = static_cast<void**>(std::calloc(depth, sizeof(void*)));
	if (!m_pool) {
		return false;
	}

	for (u16 i = 0; i < depth; ++i) {

		void* const ptr = std::calloc(1, size);

		if (!ptr) {
			for (u16 j = 0; j < i; ++j) {
				std::free(m_pool[j]);
			}

			std::free(m_pool);
			m_pool = nullptr;
			return false;
		}

		m_pool[i] = ptr;
	}

	m_depth 	= depth;
	m_capacity	= size;
	m_wrIndex 	= 0;
	m_msk 		= (depth - 1);
	return true;
}

reg InfinityContainer::write(const void *data, const reg len)
{
	if (!data || len > m_capacity || !m_pool) {
		return 0;
	}

	// get meta data from buffer
	reg write_reg 		= m_wrIndex;
	const reg wr_pos 	= write_reg & m_msk;

	void* const pool_ptr = m_pool[wr_pos];

	std::memcpy(pool_ptr, data, len);

	// save write position
	m_wrIndex = write_reg + 1;
	return len;
}

void* const InfinityContainer::getWriteBuffer()
{
    if (!m_pool) {
        return nullptr;
    }

	// get meta data from buffer
	const reg write_reg = m_wrIndex;
	const reg wr_pos 	= write_reg & m_msk;

	return m_pool[wr_pos];
}

reg InfinityContainer::read(void* const data, const reg len)
{
    if (!data || len > m_capacity || !m_pool) {
        return 0;
    }

    // get meta data from buffer
    const reg write_reg	= m_wrIndex;
    const reg read_reg 	= (write_reg - 1);
    const reg rd_pos 	= read_reg & m_msk;

	void* const pool_ptr = m_pool[rd_pos];

	std::memcpy(data, pool_ptr, len);

	m_rdIndex = write_reg;
	return len;
}

void* const InfinityContainer::getReadBuffer()
{
    if (!m_pool) {
        return nullptr;
    }

    // get meta data from buffer
    const reg read_reg 	= (m_wrIndex - 1);
    const reg rd_pos 	= read_reg & m_msk;

	return m_pool[rd_pos];
}
