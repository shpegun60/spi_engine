/*
 * SPImaster.cpp
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */


#include "SPImaster.h"

#ifdef HAL_SPI_MODULE_ENABLED
#include "SPIreset.h"
#include <algorithm>

void SPImaster::init(const Init& settings, const u16 distance)
{
	if(settings.hspi == nullptr || settings.rx_ptr == nullptr || settings.tx_ptr == nullptr || distance == 0 ||
			settings.rx_size == 0 || settings.tx_size == 0) {
		return;
	}

	// SPI -----------------------
	m_hspi 		= settings.hspi;

	// total buffers size calculation
	const reg max_size = m_size = std::max(settings.rx_size, settings.tx_size) /* plus salt */ + sizeof(u32) /* plus hash */ + sizeof(u32);

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
#ifdef SPI_MODE_MASTER
		settings.hspi->Init.Mode 				= SPI_MODE_MASTER;
#endif /* SPI_MODE_MASTER */
#ifdef SPI_DIRECTION_2LINES
		settings.hspi->Init.Direction 			= SPI_DIRECTION_2LINES;
#endif /* SPI_DIRECTION_2LINES */
#ifdef SPI_DATASIZE_8BIT
		settings.hspi->Init.DataSize 			= SPI_DATASIZE_8BIT;
#endif /* SPI_DATASIZE_8BIT */
#ifdef SPI_POLARITY_LOW
		settings.hspi->Init.CLKPolarity 		= SPI_POLARITY_LOW;
#endif /* SPI_POLARITY_LOW */
#ifdef SPI_PHASE_2EDGE
		settings.hspi->Init.CLKPhase 			= SPI_PHASE_2EDGE;
#endif /* SPI_PHASE_2EDGE */
#ifdef SPI_NSS_HARD_OUTPUT
		settings.hspi->Init.NSS 				= SPI_NSS_HARD_OUTPUT;
#endif /* SPI_NSS_HARD_OUTPUT */
#ifdef SPI_FIRSTBIT_MSB
		settings.hspi->Init.FirstBit 			= SPI_FIRSTBIT_MSB;
#endif /* SPI_FIRSTBIT_MSB */
#ifdef SPI_TIMODE_DISABLE
		settings.hspi->Init.TIMode 				= SPI_TIMODE_DISABLE;
#endif /* SPI_TIMODE_DISABLE */
#ifdef SPI_NSS_PULSE_DISABLE
		settings.hspi->Init.NSSPMode 			= SPI_NSS_PULSE_DISABLE;
#endif /* SPI_NSS_PULSE_DISABLE */
#ifdef SPI_NSS_POLARITY_LOW
		settings.hspi->Init.NSSPolarity 		= SPI_NSS_POLARITY_LOW;
#endif /* SPI_NSS_POLARITY_LOW */
#ifdef SPI_MASTER_KEEP_IO_STATE_ENABLE
		settings.hspi->Init.MasterKeepIOState 	= SPI_MASTER_KEEP_IO_STATE_ENABLE;
#endif /* SPI_MASTER_KEEP_IO_STATE_ENABLE */


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

void SPImaster::loop(const u32 time)
{
	// rx loop
	if(m_rxBuffer.readIsExist()) {
		m_rxBuffer.read(m_rxPtr, m_rxSize);

		m_ping 			= time - m_lastTxTime;
		m_lastTxTime 	= time;
		m_isOnline 		= true;
	} else if((time - m_lastTxTime) > NoLinkTime) {
		m_ping 			= 0xFFFFFFFF;
		m_lastTxTime 	= time;
		m_isOnline 		= false;
	}

	// tx loop
	m_txBuffer.write(m_txPtr, m_txSize);

	// check tx status
	if(!isTxStarted) {
		updateBuffers();
	}
}

bool SPImaster::saltedLoop(const u32 time)
{
	bool isSalted = false;

#define STATE_TX 0
#define STATE_RX 1

	switch(state) {

	case STATE_TX: {
		// salt logic --------------------------------------------------------------
		++salt; 												// add salt
		u8* const wrPtr 	= static_cast<u8*>(m_txBuffer.getWriteBuffer());
		u32* const pSaltTx 	= reinterpret_cast<u32*>(wrPtr + m_txSize);
		*pSaltTx = salt;										// write salt to tx
		//--------------------------------------------------------------------------

		m_txBuffer.write(m_txPtr, m_txSize);
		m_lastTxTime 	= time;
		state 			= STATE_RX;
		break;}

	case STATE_RX: {
		// rx loop
		if(m_rxBuffer.readIsExist()) {

			// salt logic --------------------------------------------------------------
			const u8* const rdPtr 		= static_cast<u8*>(m_rxBuffer.getReadBuffer());
			const u32* const pSaltRx 	= reinterpret_cast<const u32*>(rdPtr + m_rxSize);
			isSalted = (*pSaltRx == salt);		// check total salt
			//--------------------------------------------------------------------------

			if(isSalted) {
				m_rxBuffer.read(m_rxPtr, m_rxSize);
				m_ping 		= time - m_lastTxTime;
				m_isOnline	= true;
				state 		= STATE_TX;
			}
		}

		if(!isSalted && (time - m_lastTxTime) > NoLinkTime) {
			++m_errors;
			m_ping 		= 0xFFFFFFFF;
			m_isOnline 	= false;
			state 		= STATE_TX;
		}
		break;}

	default: { state = STATE_TX; break; }

	}

#undef STATE_TX
#undef STATE_RX

	// check tx status
	if(!isTxStarted) {
		updateBuffers();
	}

	return isSalted;
}

void SPImaster::onError()
{
	++m_errors;
	{
		// reset DMA
		HAL_SPI_DMAStop(m_hspi);
		// Reset SPI for clear internal fifo
		SPI_ENGINE_RESET(m_hspi);
		// init after reset
		HAL_SPI_Init(m_hspi);
	}

	updateBuffers();
}

#endif /* HAL_SPI_MODULE_ENABLED */
