#include "InterruptSource.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"

namespace casioemu
{
	InterruptSource::InterruptSource()
	{
		setup_done = false;
	}

	void InterruptSource::Setup(size_t _interrupt_index, Emulator &_emulator)
	{
		if (setup_done)
			PANIC("Setup invoked twice\n");

		interrupt_index = _interrupt_index;
		emulator = &_emulator;

		setup_done = true;
	}

	bool InterruptSource::Enabled()
	{
		if (!setup_done)
			PANIC("Setup not invoked\n");

		return emulator->chipset.InterruptEnabledBySFR(interrupt_index);
	}

	bool InterruptSource::TryRaise()
	{
		if (!setup_done)
			PANIC("Setup not invoked\n");

		if (!emulator->chipset.InterruptEnabledBySFR(interrupt_index) || emulator->chipset.GetInterruptPendingSFR(interrupt_index))
		{
			raise_success = false;
		}
		else
		{
			emulator->chipset.RaiseMaskable(interrupt_index);
			raise_success = true;
		}

		return raise_success;
	}

	bool InterruptSource::Success()
	{
		if (!setup_done)
			PANIC("Setup not invoked\n");

		return raise_success && emulator->chipset.GetInterruptPendingSFR(interrupt_index);
	}
}

