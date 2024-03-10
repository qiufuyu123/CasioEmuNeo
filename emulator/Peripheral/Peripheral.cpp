#include "Peripheral.hpp"

namespace casioemu
{
	Peripheral::Peripheral(Emulator &_emulator) : emulator(_emulator), require_frame(false)
	{
	}

	Peripheral::~Peripheral()
	{
	}

	void Peripheral::Initialise()
	{
	}

	void Peripheral::Uninitialise()
	{
	}

	void Peripheral::Tick()
	{
	}

	void Peripheral::TickAfterInterrupts()
	{
	}

	void Peripheral::Frame()
	{
		require_frame = false;
	}

	void Peripheral::UIEvent(SDL_Event &)
	{
	}

	void Peripheral::Reset()
	{
	}

	bool Peripheral::GetRequireFrame()
	{
		return require_frame;
	}
}
