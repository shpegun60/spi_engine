/*
 * SPImaster.cpp
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#include "SPImaster.h"
#include "spi_reset.h"
#include <algorithm>

SPImaster::~SPImaster() {
	// TODO Auto-generated destructor stub
}

void SPImaster::init(const Init& settings, const u16 distance)
{
	if(settings.hspi == nullptr || settings.rx_ptr == nullptr || settings.tx_ptr == nullptr || distance == 0 ||
			settings.rx_size == 0 || settings.tx_size == 0) {
		return;
	}

	// SPI -----------------------
	m_hspi 		= settings.hspi;

	// rx buffer -----------------------------
	const reg max_size = m_size = std::max(settings.rx_size, settings.tx_size);
	m_rxBuffer.init(distance, max_size);
	m_rxPtr 	= settings.rx_ptr;
	m_rxSize 	= settings.rx_size;

	// tx buffer -----------------------------
	m_txBuffer.init(distance, max_size);
	m_txPtr 	= settings.tx_ptr;
	m_txSize	= settings.tx_size;

	// reinit spi with CRC polynomial
	SPI_ENGINE_RESET(settings.hspi);
	// MAIN PARAMETERS -------------------------
	settings.hspi->Init.Mode 				= SPI_MODE_MASTER;
	settings.hspi->Init.Direction 			= SPI_DIRECTION_2LINES;
	settings.hspi->Init.DataSize 			= SPI_DATASIZE_8BIT;
	settings.hspi->Init.CLKPolarity 		= SPI_POLARITY_LOW;
	settings.hspi->Init.CLKPhase 			= SPI_PHASE_2EDGE;
	settings.hspi->Init.NSS 				= SPI_NSS_HARD_OUTPUT;
	settings.hspi->Init.FirstBit 			= SPI_FIRSTBIT_MSB;
	settings.hspi->Init.TIMode 				= SPI_TIMODE_DISABLE;
	settings.hspi->Init.NSSPMode 			= SPI_NSS_PULSE_DISABLE;
	settings.hspi->Init.NSSPolarity 		= SPI_NSS_POLARITY_LOW;

	// CRC PARAMETERS --------------------------
	settings.hspi->Init.CRCCalculation 	= SPI_CRCCALCULATION_ENABLE;
	settings.hspi->Init.CRCPolynomial 	= 0x8005; // CRC-16-ANSI polynomial
	settings.hspi->Init.CRCLength 		= SPI_CRC_LENGTH_16BIT;
	HAL_SPI_Init(settings.hspi);
}

void SPImaster::loop(const u32 time)
{
	// rx loop
	if(m_rxBuffer.readIsExist()) {
		m_rxBuffer.read(m_rxPtr, m_rxSize);
	}

	// tx loop
	m_txBuffer.write(m_txPtr, m_txSize);

	// check tx status
	if(!isTxStarted) {
		updateBuffers();
	}
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


