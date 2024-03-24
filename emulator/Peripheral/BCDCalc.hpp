#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class BCDCalc : public Peripheral
	{
		MMURegion region_bcdcontrol, region_F402, region_F404, region_F405, region_F410, region_F414, region_F415, region_param1, region_param2, region_temp1, region_temp2;

		uint8_t data_F400, data_F402, data_F404, data_F405, data_F410, data_F414, data_F415;

		uint8_t data_param1[12];
		uint8_t data_param2[12];
		uint8_t data_temp1[12];
		uint8_t data_temp2[12];

		bool F400_write;
		bool F402_write;
		bool F404_write;
		bool F405_write;

		uint8_t data_operator, data_type_1, data_type_2, param1, param2, param3, param4, data_F404_copy,
			data_mode, data_repeat_flag, data_a, data_b, data_c, data_d, data_F402_copy;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();

		void GenerateParams();
		void F405control();
		void ShiftLeft(int param);
		void ShiftRight(int param);
		void DataOperate();
	};
}
