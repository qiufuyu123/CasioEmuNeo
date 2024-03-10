#include "MMURegion.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "MMU.hpp"

namespace casioemu
{
	MMURegion::MMURegion()
	{
		setup_done = false;
	}

	MMURegion::~MMURegion()
	{
		if (setup_done)
			Kill();
	}

	void MMURegion::Setup(size_t _base, size_t _size, std::string _description, void *_userdata, ReadFunction _read, WriteFunction _write, Emulator &_emulator)
	{
		if (setup_done)
			PANIC("Setup invoked twice\n");

		emulator = &_emulator;
		base = _base;
		size = _size;
		description = _description;
		userdata = _userdata;
		read = _read;
		write = _write;

		emulator->chipset.mmu.RegisterRegion(this);
		setup_done = true;
	}

	void MMURegion::Kill()
	{
		emulator->chipset.mmu.UnregisterRegion(this);
		setup_done = false;
	}
}

