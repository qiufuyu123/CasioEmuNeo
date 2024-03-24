#include "BatteryBackedRAM.hpp"

#include "../Data/HardwareId.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Logger.hpp"
#include "../Gui/ui.hpp"
#include <fstream>
#include <cstring>

namespace casioemu
{
	void BatteryBackedRAM::Initialise()
	{
		bool real_hardware = emulator.GetModelInfo("real_hardware");
		switch (emulator.hardware_id)
		{
		case HW_ES_PLUS:
			ram_size = 0xE00;
			break;
		case HW_CLASSWIZ:
			ram_size = 0x2000;
			break;
		case HW_CLASSWIZ_II:
			ram_size = 0x6000;
			break;
		}
		if (!real_hardware)
			ram_size += 0x100;

		ram_buffer = new uint8_t[ram_size];
		for (size_t ix = 0; ix != ram_size; ++ix)
			ram_buffer[ix] = 0;

		ram_file_requested = false;
		if (emulator.argv_map.find("ram") != emulator.argv_map.end())
		{
			ram_file_requested = true;

			if (emulator.argv_map.find("clean_ram") == emulator.argv_map.end())
				LoadRAMImage();
		}

		region.Setup(emulator.hardware_id == HW_ES_PLUS ? 0x8000 : emulator.hardware_id == HW_CLASSWIZ ? 0xD000 : 0x9000,
			emulator.hardware_id == HW_ES_PLUS ? 0x0E00 : emulator.hardware_id == HW_CLASSWIZ ? 0x2000 : 0x6000 ,
			"BatteryBackedRAM", ram_buffer, [](MMURegion *region, size_t offset) {
			return ((uint8_t *)region->userdata)[offset - region->base];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			((uint8_t *)region->userdata)[offset - region->base] = data;
		}, emulator);
		if (!real_hardware)
			region_2.Setup(emulator.hardware_id == HW_ES_PLUS ? 0x9800 : emulator.hardware_id == HW_CLASSWIZ ? 0x49800 : 0x89800, 0x0100,
				"BatteryBackedRAM/2", ram_buffer + ram_size - 0x100, [](MMURegion* region, size_t offset) {
					return ((uint8_t*)region->userdata)[offset - region->base];
				}, [](MMURegion* region, size_t offset, uint8_t data) {
					((uint8_t*)region->userdata)[offset - region->base] = data;
				}, emulator);
		BatteryBackedRAM::rom_addr = (char*)ram_buffer;
		logger::Info("inited hex editor!\n");
	}

	void BatteryBackedRAM::Uninitialise()
	{
		if (ram_file_requested && emulator.argv_map.find("preserve_ram") == emulator.argv_map.end())
			SaveRAMImage();

		delete[] ram_buffer;
	}

	void BatteryBackedRAM::SaveRAMImage()
	{
		std::ofstream ram_handle(emulator.argv_map["ram"], std::ofstream::binary);
		if (ram_handle.fail())
		{
			logger::Info("[BatteryBackedRAM] std::ofstream failed: %s\n", std::strerror(errno));
			return;
		}
		ram_handle.write((char *)ram_buffer, ram_size);
		if (ram_handle.fail())
		{
			logger::Info("[BatteryBackedRAM] std::ofstream failed: %s\n", std::strerror(errno));
			return;
		}
	}

	void BatteryBackedRAM::LoadRAMImage()
	{
		std::ifstream ram_handle(emulator.argv_map["ram"], std::ifstream::binary);
		if (ram_handle.fail())
		{
			logger::Info("[BatteryBackedRAM] std::ifstream failed: %s\n", std::strerror(errno));
			return;
		}
		ram_handle.read((char *)ram_buffer, ram_size);
		if (ram_handle.fail())
		{
			logger::Info("[BatteryBackedRAM] std::ifstream failed: %s\n", std::strerror(errno));
			return;
		}
	}

	char* BatteryBackedRAM::rom_addr = 0;
}
