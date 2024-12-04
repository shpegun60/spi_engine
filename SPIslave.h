/*
 * SPIslave.h
 *
 *  Created on: Nov 15, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPISLAVE_H_
#define SPI_ENGINE_SPISLAVE_H_

#include "main.h"

#ifdef HAL_SPI_MODULE_ENABLED
#include <basic_types.h>
#include "buffers/InfinityContainer.h"
#include <cstring>

class SPIslave
{
public:
	SPIslave() = default;
	~SPIslave() = default;

	static constexpr bool CircularMode = false;
	static constexpr u32 NoLinkTime = 5000;

	typedef struct {
		SPI_HandleTypeDef* 	hspi;
		u16 				NSS_Pin; // must be external interrupt pin
		// rx fields
		void* 			rx_ptr;
		reg 			rx_size;
		// tx fields
		const void* 	tx_ptr;
		reg 			tx_size;
	} Init;

	void init(const Init&, const u16 distance = 4);
	bool loop(const u32 time = 0);
	inline bool isOnline() const { return m_isOnline; }

protected:
	inline void onTxRxComplete();
	void onNSSEvent();
	inline void onError() { ++m_errors; }

    inline bool matchesHandle(const SPI_HandleTypeDef* const hspi) const { return m_hspi && (hspi == m_hspi); }
    inline bool matchesNSS(const u16 NSS) const { return m_NSS_Pin == NSS; }
private:
	SPI_HandleTypeDef* m_hspi 	= nullptr;
	u16 m_NSS_Pin 				= 0;
	reg m_size 					= 0;

	// rx buffer -----------------------------
	InfinityContainer 	m_rxBuffer;
	void* m_rxPtr	 	= nullptr;
	reg m_rxSize 		= 0;

	// tx buffer -----------------------------
	InfinityContainer m_txBuffer;
	const void* m_txPtr = nullptr;
	reg m_txSize 			= 0;

	// errors counter
	reg m_errors = 0;

	// ping operation
	reg m_pack_cnt = 0;
	reg m_tx_cnt = 0;
	u32 lastMsgTime = 0;
	u32 ping = 0xFFFFFFFF;
	bool m_isOnline = false;

	// hash
	u32 hash = 0;
	reg hash_pos = 0;
};


inline void SPIslave::onTxRxComplete()
{
	// getting pointers to buffers ---------------------------------------------
	u8* const pRxData = static_cast<u8*>(m_rxBuffer.getWriteBuffer());
	const u32* const pHashRx = reinterpret_cast<const u32*>(pRxData + hash_pos);

	// check hash --------------------------------------------------------------
	if(*pHashRx == hash) {
		m_txBuffer.readAct();
		m_rxBuffer.writeAct();
	} else {
		++m_errors;
	}
}

#endif /* HAL_SPI_MODULE_ENABLED */
#endif /* SPI_ENGINE_SPISLAVE_H_ */
