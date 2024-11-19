/*
 * SPIslave.h
 *
 *  Created on: Nov 15, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPISLAVE_H_
#define SPI_ENGINE_SPISLAVE_H_

#include "main.h"
#include <basic_types.h>
#include "buffers/InfinityContainer.h"

class SPIslave
{
public:
	SPIslave() = default;
	~SPIslave();

	typedef struct {
		SPI_HandleTypeDef* 	hspi;
		u16 				NSS_Pin;
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
	inline void onTxRxComplete() { m_rxBuffer.writeAct(); m_txBuffer.readAct(); }
	void onNSSEvent();
	inline void onError() { ++m_errors; }

    inline bool matchesHandle(const SPI_HandleTypeDef* const hspi) const { return m_hspi && (hspi == m_hspi); }
    inline bool matchesNSS(const u16 NSS) const { return m_NSS_Pin == NSS; }

private:
	SPI_HandleTypeDef* m_hspi 	= nullptr;
	u16 m_NSS_Pin 				= 0;
	reg m_size 					= 0;

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

#endif /* SPI_ENGINE_SPISLAVE_H_ */
