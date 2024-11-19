/*
 * spi_pack.h
 *
 *  Created on: Nov 13, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPI_PACK_H_
#define SPI_ENGINE_SPI_PACK_H_

#include <basic_types.h>

typedef struct {
	u32 	val0;
	float 	val1;
} SPIMasterPack_t;

typedef struct {
	u8 		data0;
	u16 	data1;
	u32 	data2;
	float 	data3;
} SPISlavePack_t;

typedef struct {
	struct {
		SPIMasterPack_t master;
		SPISlavePack_t slave;
	}dout;

	SPIMasterPack_t din;
} SPIPack_t;


#endif /* SPI_ENGINE_SPI_PACK_H_ */
