/*
 * SPImaster.h
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPIMASTER_H_
#define SPI_ENGINE_SPIMASTER_H_

#include "main.h"

#ifdef HAL_SPI_MODULE_ENABLED
#include <basic_types.h>
#include "buffers/InfinityContainer.h"


class SPImaster
{
public:
	SPImaster() = default;
	~SPImaster() = default;

	static constexpr bool CircularMode = false;
	static constexpr u32 NoLinkTime = 100;

	typedef struct {
		SPI_HandleTypeDef* 	hspi;
		// rx fields
		void* 			rx_ptr;
		reg 			rx_size;
		// tx fields
		const void* 	tx_ptr;
		reg 			tx_size;
	} Init;

	void init(const Init&, const u16 distance = 4);
	void loop(const u32 time = 0);
	bool saltedLoop(const u32 time = 0);
	inline bool isOnline() const { return m_isOnline; }

protected:
	inline void onTxRxComplete();
	void onError();
    inline bool matchesHandle(const SPI_HandleTypeDef* const hspi) const { return m_hspi && (hspi == m_hspi); }

private:
    inline void updateBuffers();

private:
	SPI_HandleTypeDef* m_hspi 	= nullptr;
	reg m_size 					= 0;

	volatile bool isTxStarted = false;

	// rx buffer -----------------------------
	InfinityContainer m_rxBuffer;
	void* m_rxPtr	 	= nullptr;
	reg m_rxSize 		= 0;

	// tx buffer -----------------------------
	InfinityContainer m_txBuffer;
	const void* m_txPtr 	= nullptr;
	reg m_txSize 			= 0;
	u32 m_lastTxTime 		= 0;

	// tx/rx salt
	u32 salt = 0;
	u8 state = 0;

	// ping
	u32 m_ping 		= 0xFFFFFFFF;
	bool m_isOnline = false;
	// errors counter
	reg m_errors 	= 0;

	// hash
	u32 hash = 0;
	reg hash_pos = 0;
};

inline void SPImaster::onTxRxComplete()
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

	if constexpr (CircularMode) {
		updateBuffers();
	} else {
		isTxStarted = false;
	}
}

inline void SPImaster::updateBuffers()
{
	const u8* pTxData = static_cast<u8*>(m_txBuffer.getReadBuffer());
	u8* const pRxData = static_cast<u8*>(m_rxBuffer.getWriteBuffer());

	const HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA(m_hspi, pTxData, pRxData, m_size);
	isTxStarted = (status == HAL_OK);
}

#endif /* HAL_SPI_MODULE_ENABLED */
#endif /* SPI_ENGINE_SPIMASTER_H_ */
