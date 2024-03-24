#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"
#include "../Chipset/InterruptSource.hpp"

namespace casioemu
{
	class Timer : public Peripheral
	{
		MMURegion region_counter, region_interval, region_F024, region_control;
		uint16_t data_counter, data_interval;
		uint8_t data_F024, data_control;

		InterruptSource interrupt_source;
		bool real_hardware;
		bool TimerSkipped;
		bool raise_required;
		uint64_t ext_to_int_counter, ext_to_int_next, ext_to_int_int_done;
		static const uint64_t ext_to_int_frequency = 10000;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();
		void TickAfterInterrupts();
		void DivideTicks();
	};
}

