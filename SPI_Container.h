/*
 * SPI_Container.h
 *
 *  Created on: Nov 17, 2024
 *      Author: admin
 */

#ifndef SPI_ENGINE_SPI_CONTAINER_H_
#define SPI_ENGINE_SPI_CONTAINER_H_

#include <array>
#include "SPImaster.h"
#include "SPIslave.h"


template <class SPIType, u8 N = 0>
class SPI_Container : public SPIType
{
public:
	using type = SPI_Container<SPIType, N>;
	static constexpr u8 elements = N;

	SPI_Container();
	~SPI_Container();

private:
	static inline bool handleTxRxComplete(SPI_HandleTypeDef* const hspi);
	static inline bool handleError(SPI_HandleTypeDef* const hspi);

	// SFINAE method (only for SPIslave)
    template <class T = SPIType>
    static inline std::enable_if_t<std::is_same_v<T, SPIslave>, bool>
    handleNSS(const uint16_t GPIO_Pin); // only slave function

	// Export from HAL callbacks
	friend void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi);
	friend void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi);
	friend void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

private:
	static inline std::array<type*, N> m_instances{};
};



// ----------------------------------------------------------------------------
template <class SPIType, u8 N>
SPI_Container<SPIType, N>::SPI_Container()
{
	if constexpr (N == 0) {
		// No instances to handle
		return;
	} else if constexpr (N == 1) {
		// Optimization for one instance
		m_instances[0] = this;
	} else {
		// Standard logic for multiple instances
		for (u8 i = 0; i < N; ++i) {
			if (!m_instances[i]) {
				m_instances[i] = this;
				return;
			}
		}
	}
}

template <class SPIType, u8 N>
SPI_Container<SPIType, N>::~SPI_Container()
{
	if constexpr (N == 0) {
		// No instances to handle
		return;
	} else if constexpr (N == 1) {
		// Optimization for one instance
		m_instances[0] = nullptr;
	} else {
		// Standard logic for multiple instances
		for (u8 i = 0; i < N; ++i) {
			if (m_instances[i] == this) {
				m_instances[i] = nullptr;
				return;
			}
		}
	}
}

template <class SPIType, u8 N>
inline bool SPI_Container<SPIType, N>::handleTxRxComplete(SPI_HandleTypeDef* const hspi)
{
	if constexpr (N == 0) {
		// No instances to handle
		return false;
	} else if constexpr (N == 1) {
		// Optimization for one instance
		type* const instance = m_instances[0];
		if (instance && instance->matchesHandle(hspi)) {
			instance->onTxRxComplete();
			return true;
		}
	} else {
		// Standard logic for multiple instances
		for (u8 i = 0; i < N; ++i) {
			type* const instance = m_instances[i];
			if (instance && instance->matchesHandle(hspi)) {
				instance->onTxRxComplete();
				return true;
			}
		}
	}
	// Default case if no matches found
	return false;
}


template <class SPIType, u8 N>
inline bool SPI_Container<SPIType, N>::handleError(SPI_HandleTypeDef* const hspi)
{
	if constexpr (N == 0) {
		// No instances to handle
		return false;
	} else if constexpr (N == 1) {
		// Optimization for one instance
		type* const instance = m_instances[0];
		if (instance && instance->matchesHandle(hspi)) {
			instance->onError();
			return true;
		}
	} else {
		// Standard logic for multiple instances
		for (u8 i = 0; i < N; ++i) {
			type* const instance = m_instances[i];
			if (instance && instance->matchesHandle(hspi)) {
				instance->onError();
				return true;
			}
		}
	}
	// Default case if no matches found
	return false;
}


// SFINAE method (only for SPIslave)
template <class SPIType, u8 N>
template <class T>
inline std::enable_if_t<std::is_same_v<T, SPIslave>, bool>
SPI_Container<SPIType, N>::handleNSS(const uint16_t GPIO_Pin)
{
	if constexpr (std::is_same_v<SPIType, SPIslave>) {
		if constexpr (N == 0) {
			// No instances to handle
			return false;
		} else if constexpr (N == 1) {
			// Optimization for one instance
			type* const instance = m_instances[0];
			if (instance && instance->matchesNSS(GPIO_Pin)) {
				instance->onNSSEvent();
				return true;
			}
		} else {
			// Standard logic for multiple instances
			for (u8 i = 0; i < N; ++i) {
				type* const instance = m_instances[i];
				if (instance && instance->matchesNSS(GPIO_Pin)) {
					instance->onNSSEvent();
					return true;
				}
			}
		}
	}
	// Default case if no matches found
	return false;
}

#endif /* SPI_ENGINE_SPI_CONTAINER_H_ */
