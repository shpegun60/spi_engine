/*
 * SPIslave.cpp
 *
 *  Created on: Nov 15, 2024
 *      Author: admin
 */

#include "SPIslave.h"

#ifdef HAL_SPI_MODULE_ENABLED
#include "SPIreset.h"
#include <algorithm>

void SPIslave::init(const Init& settings, const u16 distance)
{
	if(settings.hspi == nullptr || settings.rx_ptr == nullptr || settings.tx_ptr == nullptr || distance == 0 ||
			settings.rx_size == 0 || settings.tx_size == 0) {
		return;
	}

	// SPI -----------------------
	m_hspi 		= settings.hspi;
	m_NSS_Pin 	= settings.NSS_Pin;

	const reg max_size = m_size = std::max(settings.rx_size, settings.tx_size)  /* plus salt */ + sizeof(u32) /* plus hash */ + sizeof(u32);

	// hash
	{
		hash = max_size ^ (settings.tx_size * settings.rx_size);
		hash_pos = max_size - sizeof(u32);
	}

	// rx buffer -----------------------------
	{
		m_rxBuffer.init(distance, max_size);
		m_rxPtr 	= settings.rx_ptr;
		m_rxSize 	= settings.rx_size;
	}


	// tx buffer -----------------------------
	{
		if(m_txBuffer.init(distance, max_size)) {
			// write hash to table ------------------------------------------
			m_txBuffer.initializeBuff([this](void* buffer, reg size) {
				if(size < (0 /* plus salt */ + sizeof(u32) /* plus hash */ + sizeof(u32))) {
					return;
				}

				u8* const buffer_u8 = static_cast<u8*>(buffer);
				u32* const ptr 		= reinterpret_cast<u32*>(buffer_u8 + hash_pos);
				*ptr = hash;
			});
			// --------------------------------------------------------------
		}

		m_txPtr 	= settings.tx_ptr;
		m_txSize	= settings.tx_size;
	}

	// reinit spi with CRC polynomial
	{
		SPI_ENGINE_RESET(settings.hspi);
		// MAIN PARAMETERS -------------------------

#ifdef SPI_MODE_SLAVE
		settings.hspi->Init.Mode 			= SPI_MODE_SLAVE;
#endif /* SPI_MODE_SLAVE */
#ifdef SPI_DIRECTION_2LINES
		settings.hspi->Init.Direction 		= SPI_DIRECTION_2LINES;
#endif /* SPI_DIRECTION_2LINES */
#ifdef SPI_DATASIZE_8BIT
		settings.hspi->Init.DataSize 		= SPI_DATASIZE_8BIT;
#endif /* SPI_DATASIZE_8BIT */
#ifdef SPI_POLARITY_LOW
		settings.hspi->Init.CLKPolarity 	= SPI_POLARITY_LOW;
#endif /* SPI_POLARITY_LOW */
#ifdef SPI_PHASE_2EDGE
		settings.hspi->Init.CLKPhase 		= SPI_PHASE_2EDGE;
#endif /* SPI_PHASE_2EDGE */
#ifdef SPI_NSS_SOFT
		settings.hspi->Init.NSS 			= SPI_NSS_SOFT;
#endif /* SPI_NSS_SOFT */
#ifdef SPI_FIRSTBIT_MSB
		settings.hspi->Init.FirstBit 		= SPI_FIRSTBIT_MSB;
#endif /* SPI_FIRSTBIT_MSB */
#ifdef SPI_TIMODE_DISABLE
		settings.hspi->Init.TIMode 			= SPI_TIMODE_DISABLE;
#endif /* SPI_TIMODE_DISABLE */
#ifdef SPI_NSS_PULSE_DISABLE
		settings.hspi->Init.NSSPMode 		= SPI_NSS_PULSE_DISABLE;
#endif /* SPI_NSS_PULSE_DISABLE */
#ifdef SPI_NSS_POLARITY_LOW
		settings.hspi->Init.NSSPolarity 	= SPI_NSS_POLARITY_LOW;
#endif /* SPI_NSS_POLARITY_LOW */

		// CRC PARAMETERS --------------------------
#ifdef SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN
		settings.hspi->Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
		settings.hspi->Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
#endif /* SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN */

		settings.hspi->Init.CRCCalculation 	= SPI_CRCCALCULATION_ENABLE;
		settings.hspi->Init.CRCPolynomial 	= 0x8005; // CRC-16-ANSI polynomial
		settings.hspi->Init.CRCLength 		= SPI_CRC_LENGTH_16BIT;
		HAL_SPI_Init(settings.hspi);
	}
}


bool SPIslave::loop(const u32 time)
{
	const bool rx_not_empty = m_rxBuffer.readIsExist();

	// rx loop ------------------------------------
	if(rx_not_empty) {
		m_rxBuffer.read(m_rxPtr, m_rxSize);

		ping 		= time - lastMsgTime;
		lastMsgTime = time;
		m_isOnline 	= true;
	} else if((time - lastMsgTime) > NoLinkTime) {
		lastMsgTime = time;
		ping 		= 0xFFFFFFFF;
		m_isOnline 	= false;
	}

	// tx loop ------------------------------------
	if constexpr (CircularMode) {
		m_txBuffer.write(m_txPtr, m_txSize);
	} else {
		if(m_tx_cnt != m_pack_cnt) {
			m_txBuffer.write(m_txPtr, m_txSize);
			m_tx_cnt = m_pack_cnt;
		}
	}

	return rx_not_empty;
}

void SPIslave::onNSSEvent()
{
	// Synchronization if need
	if(__HAL_DMA_GET_COUNTER(m_hspi->hdmarx)) {
		// reset DMA
		HAL_SPI_DMAStop(m_hspi);
		// Reset SPI for clear internal fifo
		SPI_ENGINE_RESET(m_hspi);
		// init after reset
		HAL_SPI_Init(m_hspi);
		++m_errors;
	}

	// getting pointers to buffers ---------------------------------------------
	u8* const pRxData = static_cast<u8*>(m_rxBuffer.getWriteBuffer());
	u8* const pTxData = static_cast<u8*>(m_txBuffer.getReadBuffer());

	/*
	 * SALT operation -----------------------------------------------------------
	 */
	{
		// getting pointers to salts
		const u8* const pRxData_read 	= static_cast<u8*>(m_rxBuffer.getReadBuffer());
		const u32* const pSaltRx 		= reinterpret_cast<const u32*>(pRxData_read 	+ m_rxSize);
		u32* const pSaltTx 				= reinterpret_cast<u32*>(pTxData 				+ m_txSize);

		// write salt
		*pSaltTx 	= *pSaltRx;
	}
	//--------------------------------------------------------------------------

	/*
	 * PING operation -----------------------------------------------------------
	 */
	{
		const u32 tmp 	= m_pack_cnt + 1;
		m_pack_cnt 		= tmp;
	}

	HAL_SPI_TransmitReceive_DMA(m_hspi, pTxData, pRxData, m_size);
}

#endif /* HAL_SPI_MODULE_ENABLED */
