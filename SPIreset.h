/*
 * spi_reset.h
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPIRESET_H_
#define SPI_ENGINE_SPIRESET_H_

#include "main.h"

#define ENGINE_SPI_NOP_DELAY()                            		\
		do{														\
			__NOP();                                        	\
			__NOP();                                          	\
		} while(0U)

#define ENGINE_SPI1_RESET(hspi)                       			\
		if ((hspi) == SPI1) {                             		\
			__HAL_RCC_SPI1_FORCE_RESET();                 		\
			ENGINE_SPI_NOP_DELAY();                           	\
			__HAL_RCC_SPI1_RELEASE_RESET();               		\
		}

#if defined(SPI2)
#define ENGINE_SPI2_RESET(hspi)                            		\
		else if ((hspi) == SPI2) {                             	\
			__HAL_RCC_SPI2_FORCE_RESET();                      	\
			ENGINE_SPI_NOP_DELAY();                             \
			__HAL_RCC_SPI2_RELEASE_RESET();                    	\
		}
#else
#define ENGINE_SPI2_RESET(hspi)
#endif

#if defined(SPI3)
#define ENGINE_SPI3_RESET(hspi)                            		\
		else if ((hspi) == SPI3) {                             	\
			__HAL_RCC_SPI3_FORCE_RESET();                      	\
			ENGINE_SPI_NOP_DELAY();                             \
			__HAL_RCC_SPI3_RELEASE_RESET();                    	\
		}
#else
#define ENGINE_SPI3_RESET(hspi)
#endif

#if defined(SPI4)
#define ENGINE_SPI4_RESET(hspi)                            		\
		else if ((hspi) == SPI4) {                             	\
			__HAL_RCC_SPI4_FORCE_RESET();                      	\
			ENGINE_SPI_NOP_DELAY();                             \
			__HAL_RCC_SPI4_RELEASE_RESET();                    	\
		}
#else
#define ENGINE_SPI4_RESET(hspi)
#endif

#if defined(SPI5)
#define ENGINE_SPI5_RESET(hspi)                            		\
		else if ((hspi) == SPI5) {                             	\
			__HAL_RCC_SPI5_FORCE_RESET();                      	\
			ENGINE_SPI_NOP_DELAY();                             \
			__HAL_RCC_SPI5_RELEASE_RESET();                    	\
		}
#else
#define ENGINE_SPI5_RESET(hspi)
#endif

#if defined(SPI6)
#define ENGINE_SPI6_RESET(hspi)                            		\
		else if ((hspi) == SPI6) {                             	\
			__HAL_RCC_SPI6_FORCE_RESET();                      	\
			ENGINE_SPI_NOP_DELAY();                             \
			__HAL_RCC_SPI6_RELEASE_RESET();                    	\
		}
#else
#define ENGINE_SPI6_RESET(hspi)
#endif


#define SPI_ENGINE_RESET(hspi)                         					\
		do {                                              	     		\
			const SPI_TypeDef* const Instance = (hspi)->Instance;		\
			ENGINE_SPI1_RESET(Instance)                              	\
			ENGINE_SPI2_RESET(Instance)                              	\
			ENGINE_SPI3_RESET(Instance)                              	\
			ENGINE_SPI4_RESET(Instance)                              	\
			ENGINE_SPI5_RESET(Instance)                              	\
			ENGINE_SPI6_RESET(Instance)                              	\
		} while(0U)

#endif /* SPI_ENGINE_SPIRESET_H_ */
