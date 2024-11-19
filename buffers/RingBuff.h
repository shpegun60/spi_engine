/*
 * RingBuf.h
 *
 *  Created on: Jul 16, 2024
 *      Author: admin
 */

#ifndef CHAOSPP_RINGBUFF_H_
#define CHAOSPP_RINGBUFF_H_

#include "RingBase.h"

class CrcCalculator;

class RingBuff : public RingBase
{
public:
	RingBuff() = default;
	~RingBuff();

    bool init(void* const buffer, const reg size);
    bool installBuffer(void* const buffer);
    inline bool hasBuffer() {return buffer != nullptr;}
    /*
     * *********************************************************
     * GET
     * *********************************************************
     */
    reg get(void* const buffer, const reg size);
    u8 getc();

    template <class T> inline T get();

    /*
      * *********************************************************
      * PEEK
      * *********************************************************
      */
    reg peek(void* const buffer, const reg size);
    u8 peekc();

    template <class T> inline T peek();

    /*
      * *********************************************************
      * PUT
      * *********************************************************
      */
    reg put(const void* const buffer, const reg size);
    bool putc(const u8 c);

    template <class T> inline bool put(const T value);

    /*
      * *********************************************************
      * STREAM
      * *********************************************************
      */

    template<class T>
    inline RingBuff& operator<<(const T value) { put<T>(value); return *this; }

    template<class T>
    inline RingBuff& operator>>(T& value) { value = get<T>(); return *this; }

    /*
      * *********************************************************
      * CRC calculation
      * *********************************************************
      */
    static void crc_from(CrcCalculator& crc, const RingBuff& from);

private:
	void* buffer = nullptr;
	bool dealoc = false;
};

/*
 * *********************************************************
 * GET
 * *********************************************************
 */

template <class T>
inline T RingBuff::get()
{
	T value;
	if (get(&value, sizeof(T)) != sizeof(T)) {
		return T();
	}
	return value;
}

template <> inline u8 RingBuff::get<u8>()
{
	return getc();
}

/*
  * *********************************************************
  * PEEK
  * *********************************************************
  */

template <class T>
inline T RingBuff::peek()
{
	T value;
	if (peek(&value, sizeof(T)) != sizeof(T)) {
		return T();
	}
	return value;
}

template <> inline u8 RingBuff::peek<u8>()
{
	return peekc();
}

/*
  * *********************************************************
  * PUT
  * *********************************************************
  */

template <class T>
inline bool RingBuff::put(const T value)
{
	return put(&value, sizeof(T)) == sizeof(T);
}

template <> inline bool RingBuff::put<u8>(const u8 value)
{
	 return putc(value);
}

#endif /* CHAOSPP_RINGBUFF_H_ */
