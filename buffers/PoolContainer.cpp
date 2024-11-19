/*
 * PoolContainer.cpp
 *
 *  Created on: Jul 16, 2024
 *      Author: admin
 */

#include "PoolContainer.h"
#include <cstring>
#include <cstdlib> // For malloc and free


PoolContainer::~PoolContainer()
{
	if (m_pool) {
		const reg r_depth = depth();

		for (u16 i = 0; i < r_depth; ++i) {
			std::free(m_pool[i]);
		}

		std::free(m_pool);
		m_pool = nullptr;
	}
}


bool PoolContainer::init(const u16 depth, const reg size)
{
	if (!RingBase::init(depth) || size == 0) {
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

	m_capacity 		= size;
	return true;
}

reg PoolContainer::write(const void* data, const reg len)
{
	if (!data || len > m_capacity || !m_pool) {
		return 0;
	}

	reg head_reg 		= getHead();
	const reg msk_reg 	= getMask();
	const reg wr_pos 	= head_reg & msk_reg;

	void* const pool_ptr = m_pool[wr_pos];

	std::memcpy(pool_ptr, data, len);
	++head_reg;
	setHead(head_reg);
	return len;
}

reg PoolContainer::read(void* const data, const reg len)
{
    if (!data || len > m_capacity || !m_pool) {
        return 0;
    }

	reg tail_reg 		= getTail();
	const reg msk_reg 	= getMask();
	const reg rd_pos 	= tail_reg & msk_reg;

	void* const pool_ptr = m_pool[rd_pos];

	std::memcpy(data, pool_ptr, len);
	++tail_reg;
	setTail(tail_reg);
	return len;
}


void* const PoolContainer::getWriteBuffer()
{
    if (!m_pool) {
        return nullptr;
    }

	const reg head_reg 	= getHead();
	const reg msk_reg 	= getMask();
	const reg wr_pos 	= head_reg & msk_reg;

	return m_pool[wr_pos];
}

void* const PoolContainer::getReadBuffer()
{
    if (!m_pool) {
        return nullptr;
    }

	const reg tail_reg 	= getTail();
	const reg msk_reg 	= getMask();
	const reg rd_pos 	= tail_reg & msk_reg;

	return m_pool[rd_pos];
}



