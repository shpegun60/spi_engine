/*
 * stm32_spi.h
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_STM32SPI_H_
#define SPI_ENGINE_STM32SPI_H_

#include "SPI_Container.h"

using SPIslave_t = SPI_Container<SPIslave, 1>;
using SPImaster_t = SPI_Container<SPImaster, 1>;

#endif /* SPI_ENGINE_STM32SPI_H_ */
