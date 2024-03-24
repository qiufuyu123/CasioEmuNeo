#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class Miscellaneous : public Peripheral
	{
		MMURegion region_dsr, region_F048, region_F220;
		uint64_t data_F048;
		uint32_t data_F220;

		static constexpr uint16_t addr [] = {
			0xF00A, 0xF018, 0xF033, 0xF034, 0xF041, // both HW_ES_PLUS, HW_CLASSWIZ and HW_CLASSWIZ_II
			0xF035, 0xF036, 0xF039, 0xF012, 0xF03D, 0xF224, 0xF028, 0xF310, // HW_CLASSWIZ and HW_CLASSWIZ_II
			0xF037 //HW_CLASSWIZ_II only
		};
		static constexpr int N_BYTE = sizeof(addr) / sizeof(addr[0]);
		MMURegion region [N_BYTE];
		uint8_t data [N_BYTE];

	public:
		using Peripheral::Peripheral;

		void Initialise();
	};
}

