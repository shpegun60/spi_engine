/*
 * RingBuf.cpp
 *
 *  Created on: Jul 16, 2024
 *      Author: admin
 */

#include "RingBuff.h"
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "crc/CrcCalculator.h"

RingBuff::~RingBuff()
{
	// TODO Auto-generated destructor stub
	if (dealoc) {
		std::free(buffer);
	}
}


bool RingBuff::init(void* const buffer, const reg size)
{
	if(!RingBase::init(size)) {
		return false;
	}

	if(buffer == nullptr) {
		void* const tmp = std::malloc(size);

		if(tmp == nullptr) {
			return false;
		}

		this->buffer 	= tmp;
		dealoc 			= true;
	} else {
		this->buffer 	= buffer;
		dealoc 			= false;
	}

	return true;
}

bool RingBuff::installBuffer(void* const buffer)
{
	if(buffer == nullptr) {
		return false;
	}

	if (dealoc) {
		std::free(buffer);
	}

	this->buffer 	= buffer;
	dealoc 			= false;

	clear();
	return true;
}



/*
 * *********************************************************
 * GET
 * *********************************************************
 */

reg RingBuff::get(void* const buffer, const reg size)
{
	// move to registers------------------------------------------
	const u8* const ring_ptr 			= static_cast<u8*>(this->buffer);
	if(ring_ptr == nullptr || isEmpty()) {
		return 0;
	}

	const reg tail_reg 			= getTail();
	const reg msk_reg			= getMask();

	const reg n_elem 			= length();								// get n elements
	const reg size_constr 		= (size < n_elem) ? size : n_elem;		// constrain input size
	const reg tail_pos 			= tail_reg & msk_reg;					// get tail position
	const reg remaining_to_end 	= capacity() - tail_pos;				// get remaining from tail to end

	// do logic --------------------------------------------------
	if(size_constr > remaining_to_end) {
		const reg remaining_n		= size_constr - remaining_to_end;
		/* first get the data from fifo->out until the end of the buffer */
		std::memcpy(buffer, ring_ptr + tail_pos, remaining_to_end);
		/* then get the rest (if any) from the beginning of the buffer */
		std::memcpy(static_cast<u8*>(buffer) + remaining_to_end, ring_ptr, remaining_n);
	} else {
		/* get all the data */
		std::memcpy(buffer, ring_ptr + tail_pos, size_constr);
	}

	// proceed signalls
	setTail(tail_reg + size_constr);
	return size_constr;
}

u8 RingBuff::getc()
{
	// move to registers------------------------------------------
	const u8* const ring_ptr 	= static_cast<u8*>(this->buffer);
	if(ring_ptr == nullptr || isEmpty()) {
		return 0;
	}

	const reg tail_reg 		= getTail();
	const reg msk_reg		= getMask();
	const reg tail_pos 		= tail_reg & msk_reg;					// get tail position

	// do logic --------------------------------------------------
	const u8 value = *(ring_ptr + tail_pos);

	// proceed signalls
	setTail(tail_reg + 1);
	return value;
}


/*
  * *********************************************************
  * PEEK
  * *********************************************************
  */

reg RingBuff::peek(void* const buffer, const reg size)
{
	// move to registers------------------------------------------
	const u8* const ring_ptr 			= static_cast<u8*>(this->buffer);
	if(ring_ptr == nullptr || isEmpty()) {
		return 0;
	}

	const reg tail_reg 			= getTail();
	const reg msk_reg			= getMask();

	const reg n_elem 			= length();								// get n elements
	const reg size_constr 		= (size < n_elem) ? size : n_elem;		// constrain input size
	const reg tail_pos 			= tail_reg & msk_reg;					// get tail position
	const reg remaining_to_end 	= capacity() - tail_pos;				// get remaining from tail to end

	// do logic --------------------------------------------------
	if(size_constr > remaining_to_end) {
		const reg remaining_n		= size_constr - remaining_to_end;
		/* first get the data from fifo->out until the end of the buffer */
		std::memcpy(buffer, ring_ptr + tail_pos, remaining_to_end);
		/* then get the rest (if any) from the beginning of the buffer */
		std::memcpy(static_cast<u8*>(buffer) + remaining_to_end, ring_ptr, remaining_n);
	} else {
		/* get all the data */
		std::memcpy(buffer, ring_ptr + tail_pos, size_constr);
	}
	return size_constr;
}


u8 RingBuff::peekc()
{
	// move to registers------------------------------------------
	const u8* const ring_ptr 	= static_cast<u8*>(this->buffer);

	if(ring_ptr == nullptr || isEmpty()) {
		return 0;
	}

	const reg tail_reg 		= getTail();
	const reg msk_reg		= getMask();
	const reg tail_pos 		= tail_reg & msk_reg;					// get tail position

	// do logic --------------------------------------------------
	const u8 value = *(ring_ptr + tail_pos);

	return value;
}

/*
  * *********************************************************
  * PUT
  * *********************************************************
  */

reg RingBuff::put(const void* const buffer, const reg size)
{
	// move to registers------------------------------------------------------
	u8* const ring_ptr 			= static_cast<u8*>(this->buffer);
	if(ring_ptr == nullptr || isFull()) {
		return 0;
	}

	const reg tail_reg 			= getTail();
	const reg head_reg 			= getHead();
	const reg msk_reg			= getMask();
	const reg cap_reg			= capacity();

	const reg head_pos 			= head_reg & msk_reg;
	const reg remaining_to_end 	= cap_reg - head_pos;					// get remaining from head to end
	const reg n_empty 			= (cap_reg - head_reg) + tail_reg;		// black magic for get empty cells
	const reg size_constr 		= (size < n_empty) ? size : n_empty;	// constrain input size

	// do logic ------------------------------------------------------------
	if(size_constr > remaining_to_end) {
		const reg remaining_n		= size_constr - remaining_to_end;
		/* first put the data starting from fifo->in to buffer end */
		std::memcpy(ring_ptr + head_pos, buffer, remaining_to_end);
		/* then put the rest (if any) at the beginning of the buffer */
		std::memcpy(ring_ptr, (u8* const)(buffer) + remaining_to_end, remaining_n);
	} else {
		/* put all the data */
		std::memcpy(ring_ptr + head_pos, buffer, size_constr);
	}

	// proceed signalls
	setHead(head_reg + size_constr);
	return size_constr;
}

bool RingBuff::putc(const u8 c)
{
	// move to registers------------------------------------------------------
	u8* const ring_ptr 			= static_cast<u8*>(this->buffer);
	if(ring_ptr == nullptr || isFull()) {
		return false;
	}

	const reg head_reg 			= getHead();
	const reg msk_reg			= getMask();
	const reg head_pos 			= head_reg & msk_reg;

	*(ring_ptr + head_pos) = c;

	// proceed signalls
	setHead(head_reg + 1);
	return true;
}

/*
  * *********************************************************
  * CRC calculation
  * *********************************************************
  */
void RingBuff::crc_from(CrcCalculator &crc, const RingBuff &from)
{
	// move to registers------------------------------------------
	const u8* const ring_ptr 			= static_cast<u8*>(from.buffer);
	if(ring_ptr == nullptr || from.isEmpty()) {
		return;
	}

	const reg tail_reg 			= from.getTail();
	const reg msk_reg			= from.getMask();

	const reg n_elem 			= from.length();								// get n elements
	const reg tail_pos 			= tail_reg & msk_reg;					// get tail position
	const reg remaining_to_end 	= from.capacity() - tail_pos;				// get remaining from tail to end

	// do logic --------------------------------------------------
	if(n_elem > remaining_to_end) {
		const reg remaining_n		= n_elem - remaining_to_end;
		/* first get the data from fifo->out until the end of the buffer */
		crc.array(ring_ptr + tail_pos, remaining_to_end);
		/* then get the rest (if any) from the beginning of the buffer */
		crc.array(ring_ptr, remaining_n);
	} else {
		/* get all the data */
		crc.array(ring_ptr + tail_pos, n_elem);
	}
}
