#include "ROMWindow.hpp"

#include "../Data/HardwareId.hpp"
#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

#include <string>

namespace casioemu
{
	static void SetupROMRegion(MMURegion &region, size_t region_base, size_t size, size_t rom_base, bool strict_memory, Emulator& emulator, std::string description = {})
	{
		if (rom_base + size > emulator.chipset.rom_data.size())
			PANIC("Invalid ROM region: base %zx, size %zx\n", rom_base, size);
		uint8_t *data = emulator.chipset.rom_data.data();
		auto offset = (ssize_t) rom_base - (ssize_t) region_base;
		if (description.empty())
			description = "ROM/Segment" + std::to_string(region_base >> 16);

		MMURegion::WriteFunction write_function = strict_memory ? [](MMURegion *region, size_t address, uint8_t data) {
			logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, address);
			region->emulator->HandleMemoryError();
		} : [](MMURegion *, size_t, uint8_t) {
		};

		if (offset >= 0)
			region.Setup(region_base, size, description, data + offset, [](MMURegion *region, size_t address) {
				return ((uint8_t *)(region->userdata))[address];
			}, write_function, emulator);
		else
			region.Setup(region_base, size, description, data + rom_base, [](MMURegion *region, size_t address) {
				return ((uint8_t *)(region->userdata))[address - region->base];
			}, write_function, emulator);
	}

	void ROMWindow::Initialise()
	{
		bool strict_memory = emulator.argv_map.find("strict_memory") != emulator.argv_map.end();

		switch (emulator.hardware_id)
		{ // Initializer list cannot be used with move-only type: https://stackoverflow.com/q/8468774
		case HW_ES_PLUS:
			regions.reset(new MMURegion[3]);
			SetupROMRegion(regions[0], 0x00000, 0x08000, 0x00000, strict_memory, emulator);
			SetupROMRegion(regions[1], 0x10000, 0x10000, 0x10000, strict_memory, emulator);
			SetupROMRegion(regions[2], 0x80000, 0x10000, 0x00000, strict_memory, emulator);
			break;

		case HW_CLASSWIZ:
			regions.reset(new MMURegion[5]);
			SetupROMRegion(regions[0], 0x00000, 0x0D000, 0x00000, strict_memory, emulator);
			SetupROMRegion(regions[1], 0x10000, 0x10000, 0x10000, strict_memory, emulator);
			SetupROMRegion(regions[2], 0x20000, 0x10000, 0x20000, strict_memory, emulator);
			SetupROMRegion(regions[3], 0x30000, 0x10000, 0x30000, strict_memory, emulator);
			SetupROMRegion(regions[4], 0x50000, 0x10000, 0x00000, strict_memory, emulator);
			break;
		case HW_CLASSWIZ_II:
			regions.reset(new MMURegion[8]);
			SetupROMRegion(regions[0], 0x00000, 0x09000, 0x00000, strict_memory, emulator);
			SetupROMRegion(regions[1], 0x10000, 0x10000, 0x10000, strict_memory, emulator);
			SetupROMRegion(regions[2], 0x20000, 0x10000, 0x20000, strict_memory, emulator);
			SetupROMRegion(regions[3], 0x30000, 0x10000, 0x30000, strict_memory, emulator);
			SetupROMRegion(regions[4], 0x40000, 0x10000, 0x40000, strict_memory, emulator);
			SetupROMRegion(regions[5], 0x50000, 0x10000, 0x50000, strict_memory, emulator);
			SetupROMRegion(regions[6], 0x70000, 0x10000, 0x70000, strict_memory, emulator);
			SetupROMRegion(regions[7], 0x80000, 0x08E00, 0x00000, strict_memory, emulator);
			break;
		}
	}

	void ROMWindow::Uninitialise()
	{
	}

	void ROMWindow::Tick()
	{
	}

	void ROMWindow::Frame()
	{
	}

	void ROMWindow::UIEvent(SDL_Event &)
	{
	}
}
