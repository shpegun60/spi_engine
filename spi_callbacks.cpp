/*
 * spi_callbacks.cpp
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#include "main.h"
#include "stm32Spi.h"

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if constexpr (SPIslave_t::elements > 0) {
		SPIslave_t::handleTxRxComplete(hspi);
	}

	if constexpr (SPImaster_t::elements > 0) {
		SPImaster_t::handleTxRxComplete(hspi);
	}
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if constexpr (SPIslave_t::elements > 0) {
		SPIslave_t::handleError(hspi);
	}

	if constexpr (SPImaster_t::elements > 0) {
		SPImaster_t::handleError(hspi);
	}
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if constexpr (SPIslave_t::elements > 0) {
		SPIslave_t::handleNSS(GPIO_Pin);
	}

//	// no need for master ------------------------
//	if constexpr (SPImaster_t::elements > 0) {
//		SPImaster_t::handleNSS(GPIO_Pin);
//	}
}

