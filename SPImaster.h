/*
 * SPImaster.h
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPIMASTER_H_
#define SPI_ENGINE_SPIMASTER_H_

#include "main.h"
#include <basic_types.h>
#include "buffers/InfinityContainer.h"


class SPImaster
{
public:
	SPImaster() = default;
	~SPImaster();

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

protected:
	inline void onTxRxComplete() {	m_rxBuffer.writeAct(); m_txBuffer.readAct(); updateBuffers(); }
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

	// errors cnt
	reg m_errors = 0;
};


inline void SPImaster::updateBuffers()
{
	const u8* pTxData = reinterpret_cast<u8*>(m_txBuffer.getReadBuffer());
	u8* const pRxData = reinterpret_cast<u8*>(m_rxBuffer.getWriteBuffer());

	const HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA(m_hspi, pTxData, pRxData, m_size);
	isTxStarted = (status == HAL_OK);
}

#endif /* SPI_ENGINE_SPIMASTER_H_ */
