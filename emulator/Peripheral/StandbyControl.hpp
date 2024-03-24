#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class StandbyControl : public Peripheral
	{
		MMURegion region_stpacp, region_sbycon, region_F312;
		uint8_t stpacp_last, F312_last;
		bool stop_acceptor_enabled, shutdown_acceptor_enabled;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
	};
}
