#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

#include <memory>

namespace casioemu
{
	class ROMWindow : public Peripheral
	{
		std::unique_ptr<MMURegion[]> regions;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Tick();
		void Frame();
		void UIEvent(SDL_Event &event);
	};
}

