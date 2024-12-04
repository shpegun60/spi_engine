/*
 * spi_pack.h
 *
 *  Created on: Nov 13, 2024
 *      Author: admin
 *
 *      SPI system test structures (only for tests)
 */

#ifndef SPI_SYNC_SPI_TESTPACK_H_
#define SPI_SYNC_SPI_TESTPACK_H_

#include <basic_types.h>

typedef struct {
	u32 	val0;
	float 	val1;
} SPIMasterPack_test;

typedef struct {
	u8 		data0;
	u16 	data1;
	u32 	data2;
	float 	data3;
} SPISlavePack_test;


/*
 * HOW TO USE:
 *
 * 	// init slave spi ------------------------------------------
 * 	1) in stm32Spi.h set slaves count
 * 	using SPIslave_t = SPIcontainer<SPIslave, 1>; <<-- 1..n
 *
 * 	2) install this code:
 *
	{
		extern SPI_HandleTypeDef hspi1;

		SPIslave_t m_spi;
		SPIMasterPack_test m_masterPack;
		SPISlavePack_test m_slavePack;

		SPIslave_t::Init init = {
				.hspi = &hspi1,
				.NSS_Pin = NSSSSSSSSSSS_Pin,

				.rx_ptr 	= &m_masterPack,
				.rx_size 	= sizeof(m_masterPack),

				.tx_ptr 	= &m_slavePack,
				.tx_size 	= sizeof(m_slavePack)
		};

		m_spi.init(init);
	}

	// in BL
	{
		++m_slavePack.data0;
		m_slavePack.data3 = 42.50;
	}

* 	// init master spi ------------------------------------------
* 	1) in stm32Spi.h set masters count
 * 	using SPImaster_t = SPIcontainer<SPImaster, 1>; <<-- 1..n
 *
 * 	2) install this code:
 *
	{
		extern SPI_HandleTypeDef hspi1;

		SPImaster_t m_spi;
		SPIMasterPack_test m_masterPack;
		SPISlavePack_test m_slavePack;

		SPImaster_t::Init init = {
				.hspi = &hspi1,

				.rx_ptr 	= &m_slavePack,
				.rx_size 	= sizeof(m_slavePack),

				.tx_ptr 	= &m_masterPack,
				.tx_size 	= sizeof(m_masterPack)
		};

		m_spi.init(init);
	}


 *
 *
 */


#endif /* SPI_SYNC_SPI_TESTPACK_H_ */
