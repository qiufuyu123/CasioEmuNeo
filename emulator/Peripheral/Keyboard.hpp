#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"
#include "../Chipset/InterruptSource.hpp"

#include <unordered_map>

namespace casioemu
{
	class Keyboard : public Peripheral
	{
		MMURegion region_ko_mask, region_ko, region_ki, region_input_filter;
		uint16_t keyboard_out, keyboard_out_mask;
		uint8_t keyboard_in, input_filter, keyboard_ghost[8];

		bool real_hardware;
		MMURegion region_ready_emu, region_ko_emu, region_ki_emu, region_pd_emu;
		uint8_t keyboard_ready_emu, keyboard_out_emu, keyboard_in_emu, keyboard_pd_emu;

		uint8_t has_input;
		InterruptSource interrupt_source;

	    SDL_Renderer *renderer;

		struct Button
		{
			enum ButtonType
			{
				BT_NONE,
				BT_BUTTON,
				BT_POWER
			} type;
			SDL_Rect rect;
			uint8_t ko_bit, ki_bit;
			bool pressed, stuck;
		} buttons[64];

		// Maps from keycode to an index to (buttons).
		std::unordered_map<SDL_Keycode, size_t> keyboard_map;

		bool p0, p1, p146;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();
		void Frame();
		void UIEvent(SDL_Event &event);
		void PressButton(Button& button, bool stick);
		void PressAt(int x, int y, bool stick);
		void ReleaseAll();
		void RecalculateKI();
		void RecalculateGhost();
	};
}

