#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class StandbyControl : public Peripheral
	{
		MMURegion region_stpacp, region_sbycon;
		uint8_t stpacp_last;
		bool stop_acceptor_enabled;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
	};
}

