/*
 * stm32_spi.h
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_STM32SPI_H_
#define SPI_ENGINE_STM32SPI_H_

#if !__has_include("spi_settings.h")

#include "spi_engine/SPIcontainer.h"

#ifdef HAL_SPI_MODULE_ENABLED

using SPIslave_t = SPIcontainer<SPIslave, 1>;
using SPImaster_t = SPIcontainer<SPImaster, 1>;

#endif /* HAL_SPI_MODULE_ENABLED */
#endif /* !__has_include("spi_settings.h") */
#endif /* SPI_ENGINE_STM32SPI_H_ */
