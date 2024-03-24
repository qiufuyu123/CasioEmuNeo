#include "BCDCalc.hpp"

#include "../Chipset/MMURegion.hpp"
#include "../Data/HardwareId.hpp"
#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void BCDCalc::Initialise() {
		F400_write = false;
		F402_write = false;
		F404_write = false;
		F405_write = false;

		region_bcdcontrol.Setup(0xF400, 1, "BCDCalc/control", this, [](MMURegion* region, size_t offset) {
			BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
			return bcdcalc->data_F400;
		}, [](MMURegion* region, size_t, uint8_t data) {
			BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
			bcdcalc->data_F400 = data;
			bcdcalc->F400_write = true;
		}, emulator);

		region_param1.Setup(0xF480, 12, "BCDCalc/param1", data_param1, [](MMURegion* region, size_t offset) {
			return ((uint8_t*)region->userdata)[offset - region->base];
		}, [](MMURegion* region, size_t offset, uint8_t data) {
			((uint8_t*)region->userdata)[offset - region->base] = data;
		}, emulator);
		region_param2.Setup(0xF4A0, 12, "BCDCalc/param2", data_param2, [](MMURegion* region, size_t offset) {
			return ((uint8_t*)region->userdata)[offset - region->base];
		}, [](MMURegion* region, size_t offset, uint8_t data) {
			((uint8_t*)region->userdata)[offset - region->base] = data;
		}, emulator);
		region_temp1.Setup(0xF4C0, 12, "BCDCalc/temp1", data_temp1, [](MMURegion* region, size_t offset) {
			return ((uint8_t*)region->userdata)[offset - region->base];
		}, [](MMURegion* region, size_t offset, uint8_t data) {
			((uint8_t*)region->userdata)[offset - region->base] = data;
		}, emulator);
		region_temp2.Setup(0xF4E0, 12, "BCDCalc/temp2", data_temp2, [](MMURegion* region, size_t offset) {
			return ((uint8_t*)region->userdata)[offset - region->base];
		}, [](MMURegion* region, size_t offset, uint8_t data) {
			((uint8_t*)region->userdata)[offset - region->base] = data;
		}, emulator);

		region_F410.Setup(0xF410, 1, "BCDCalc/F410", &data_F410, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
		region_F414.Setup(0xF414, 1, "BCDCalc/F414", &data_F414, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
		region_F415.Setup(0xF415, 1, "BCDCalc/F415", &data_F415, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);

		region_F402.Setup(0xF402, 1, "BCDCalc/F402", this, [](MMURegion* region, size_t offset) {
			BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
			return bcdcalc->data_F402;
		}, [](MMURegion* region, size_t, uint8_t data) {
			BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
			bcdcalc->data_F402 = data;
			bcdcalc->F402_write = true;
		}, emulator);
		// region_F404.Setup(0xF404, 1, "BCDCalc/F404", this, [](MMURegion* region, size_t offset) {
		// 	BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
		// 	return bcdcalc->data_F404;
		// }, [](MMURegion* region, size_t, uint8_t data) {
		// 	BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
		// 	bcdcalc->data_F404 = data;
		// 	bcdcalc->F404_write = true;
		// }, emulator);
		// region_F405.Setup(0xF405, 1, "BCDCalc/F405", this, [](MMURegion* region, size_t offset) {
		// 	BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
		// 	return bcdcalc->data_F405;
		// }, [](MMURegion* region, size_t, uint8_t data) {
		// 	BCDCalc* bcdcalc = (BCDCalc*)region->userdata;
		// 	bcdcalc->data_F405 = data;
		// 	bcdcalc->F405_write = true;
		// }, emulator);
	}

	void BCDCalc::GenerateParams() {
		data_operator = (data_F400 >> 4) & 0x0F;
		data_type_2 = (data_F400 >> 2) & 0x03;
		data_type_1 = data_F400 & 0x03;
		if (data_operator == 0) {
			param1 = 0;
			param2 = 1;
		}
		else {
			param1 = 1;
		}
		return;
	}

	uint16_t CalcAddr(uint8_t base, uint8_t offset) {
		return (((uint16_t)base + 0x7A4) << 5) + (uint16_t)offset;
	}
	void BCDCalc::F405control() {
		if (data_mode == 0xFF && param1 == 0) {
			//
		}
		else {
			if (((data_a | data_b | data_c | data_d) != 0) && param1 == 0) {
				if (data_c != 0) {
					//
				}
				else if (data_b == 0 && data_a == 0) {
					//
				}
				else {
					//
				}
				if ((data_a | data_b | data_c) != 0) {
					param1 = 1;
					if ((data_operator | data_type_1 | data_type_2) == 0)
						param1 = 0;
				}
				param4 = 0;
			}
		}
		if ((data_a | data_b | data_c | data_d) != 0) {
			data_F405 = (data_F405 & 0x7F) | 0x80;
		}
		else {
			data_F405 = data_F405 & 0x7F;
		}
	}

	void BCDCalc::ShiftLeft(int param) {
		if (data_type_2 > 3)
			return;
		if (data_type_2 == 0) {
			for (uint8_t offset = 0; offset < 0x0B; offset++) {
				uint16_t addr = CalcAddr(data_type_1, 0x0B - offset);
				uint8_t val1 = emulator.chipset.mmu.ReadData((size_t)addr);
				uint8_t val2 = emulator.chipset.mmu.ReadData((size_t)(addr - 1));
				emulator.chipset.mmu.WriteData((size_t)addr, (val1 << 4) | (val2 >> 4));
			}
			uint16_t addr = CalcAddr((data_type_1 + 3) & 0x03, 0x0B);
			uint8_t val2 = emulator.chipset.mmu.ReadData((size_t)addr);
			addr = CalcAddr(data_type_1, 0);
			uint8_t val1 = emulator.chipset.mmu.ReadData((size_t)addr);
			if (param == 0) {
				val2 = 0;
			}
			emulator.chipset.mmu.WriteData((size_t)addr, (val1 << 4) | (val2 >> 4));
		}
		else if (data_type_2 == 1) {
			for (uint8_t offset = 0; offset < 0x0B; offset++) {
				uint16_t addr = CalcAddr(data_type_1, 0x0B - offset);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)(addr - 1));
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
			uint16_t addr = CalcAddr((data_type_1 + 3) & 0x03, 0x0B);
			uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
			if (param == 0) {
				val = 0;
			}
			addr = CalcAddr(data_type_1, 0);
			emulator.chipset.mmu.WriteData((size_t)addr, val);
		}
		else if (data_type_2 == 2) {
			for (uint8_t offset = 0; offset < 0x0A; offset++) {
				uint16_t addr = CalcAddr(data_type_1, 0x09 - offset);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
				emulator.chipset.mmu.WriteData((size_t)(addr+2), val);
			}
			for (int i = 0; i < 2; i++) {
				uint16_t addr = CalcAddr((data_type_1 + 3) & 0x03, i + 0x0A);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
				if (param == 0) {
					val = 0;
				}
				addr = CalcAddr(data_type_1, i);
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
		}
		else if (data_type_2 == 3) {
			for (uint8_t offset = 0; offset < 0x08; offset++) {
				uint16_t addr = CalcAddr(data_type_1, 0x07 - offset);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
				emulator.chipset.mmu.WriteData((size_t)(addr + 4), val);
			}
			for (int i = 0; i < 4; i++) {
				uint16_t addr = CalcAddr((data_type_1 + 3) & 0x03, i + 0x08);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
				if (param == 0) {
					val = 0;
				}
				addr = CalcAddr(data_type_1, i);
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
		}
	}

	void BCDCalc::ShiftRight(int param) {
		if (data_type_2 > 3)
			return;
		if (data_type_2 == 0) {
			for (uint8_t offset = 0; offset < 0x0B; offset++) {
				uint16_t addr = CalcAddr(data_type_1, offset);
				uint8_t val1 = emulator.chipset.mmu.ReadData((size_t)addr);
				uint8_t val2 = emulator.chipset.mmu.ReadData((size_t)(addr + 1));
				emulator.chipset.mmu.WriteData((size_t)addr, (val2 << 4) | (val1 >> 4));
			}
			uint16_t addr = CalcAddr(data_type_1, 0x0B);
			uint8_t val1 = emulator.chipset.mmu.ReadData((size_t)addr);
			addr = CalcAddr((data_type_1 + 1) & 0x03, 0);
			uint8_t val2 = emulator.chipset.mmu.ReadData((size_t)addr);
			if (param == 0) {
				val2 = 0;
			}
			addr = CalcAddr(data_type_1, 0x0B);
			emulator.chipset.mmu.WriteData((size_t)addr, (val2 << 4) | (val1 >> 4));
		}
		else if (data_type_2 == 1) {
			for (uint8_t offset = 0; offset < 0x0B; offset++) {
				uint16_t addr = CalcAddr(data_type_1, offset);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)(addr + 1));
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
			uint16_t addr = CalcAddr((data_type_1 + 1) & 0x03, 0);
			uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
			if (param == 0) {
				val = 0;
			}
			addr = CalcAddr(data_type_1, 0x0B);
			emulator.chipset.mmu.WriteData((size_t)addr, val);
		}
		else if (data_type_2 == 2) {
			for (uint8_t offset = 0; offset < 0x0A; offset++) {
				uint16_t addr = CalcAddr(data_type_1, offset);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)(addr + 2));
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
			for (int i = 0x0A; i < 0x0C; i++) {
				uint16_t addr = CalcAddr((data_type_1 + 1) & 0x03, i - 0x0A);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
				if (param == 0) {
					val = 0;
				}
				addr = CalcAddr(data_type_1, i);
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
		}
		else if (data_type_2 == 3) {
			for (uint8_t offset = 0; offset < 0x08; offset++) {
				uint16_t addr = CalcAddr(data_type_1, offset);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)(addr + 4));
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
			for (int i = 0x08; i < 0x0C; i++) {
				uint16_t addr = CalcAddr((data_type_1 + 1) & 0x03, i - 0x08);
				uint8_t val = emulator.chipset.mmu.ReadData((size_t)addr);
				if (param == 0) {
					val = 0;
				}
				addr = CalcAddr(data_type_1, i);
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
		}
	}

	uint32_t Calculate(uint32_t tmp, uint32_t val1, uint32_t val2, int flag) {
		if (flag == 1)
			tmp ^= 0x01;
		tmp &= 0x01;
		uint32_t val1_tmp = val1 & 0x0F;
		uint32_t val2_tmp = val2 & 0x0F;
		if (flag == 1) {
			val2_tmp = (0xFFFFFFF9 - val2_tmp) & 0x0F;
		}
		val2_tmp += val1_tmp;
		val2_tmp += tmp;
		int f = 0;
		if (val2_tmp >= 0x0A) {
			val2_tmp -= 0x0A;
			f = 1;
		}
		val2_tmp &= 0x0F;
		tmp = val2_tmp;
		val1_tmp = (val1 >> 4) & 0x0F;
		val2_tmp = (val2 >> 4) & 0x0F;
		if (flag == 1) {
			val2_tmp = (0xFFFFFFF9 - val2_tmp) & 0x0F;
		}
		val1_tmp += val2_tmp;
		val1_tmp += f;
		f = 0;
		if (val1_tmp >= 0x0A) {
			val1_tmp -= 0x0A;
			f = 1;
		}
		val1_tmp = val1_tmp << 4;
		tmp |= val1_tmp & 0xF0;
		val1_tmp = (val1 >> 8) & 0x0F;
		val2_tmp = (val2 >> 8) & 0x0F;
		if (flag == 1) {
			val2_tmp = (0xFFFFFFF9 - val2_tmp) & 0x0F;
		}
		val1_tmp += val2_tmp;
		val1_tmp += f;
		f = 0;
		if (val1_tmp >= 0x0A) {
			val1_tmp -= 0x0A;
			f = 1;
		}
		val1_tmp = (val1_tmp << 8) & 0xF00;
		val1 = (val1 >> 12) & 0x0F;
		val2 = (val2 >> 12) & 0x0F;
		tmp |= val1_tmp;
		if (flag == 1) {
			val2 = (0xFFFFFFF9 - val2) & 0x0F;
		}
		val1 += val2;
		val1 += f;
		f = 0;
		if (val1 >= 0x0A) {
			val1 -= 0x0A;
			f = 1;
		}
		val1 = (val1 << 12) & 0xF000;
		tmp |= val1;
		if (flag == 1)
			f ^= 0x01;
		f = f << 0x10;
		f += tmp;
		return f;
	}

	void BCDCalc::DataOperate() {
		if (param1 == 1 && param4 == 0 && data_F402_copy != 0) {
			bool storeresults = false;
			if (data_operator == 1 || data_operator == 2)
				storeresults = true;
			uint8_t offset = 0;
			uint16_t val1, val2;
			uint32_t tmp = 0;
			int flag;
			int data_F410_tmp = 1;
			for (int i = 0; i * 2 < data_F402_copy; i++) {
				offset = i * 4;
				uint16_t addr1 = CalcAddr(data_type_1, offset);
				val1 = (uint16_t)emulator.chipset.mmu.ReadData((size_t)(addr1 + 1)) * 0x100 + (uint16_t)emulator.chipset.mmu.ReadData((size_t)addr1);
				uint16_t addr2 = CalcAddr(data_type_2, offset);
				val2 = (uint16_t)emulator.chipset.mmu.ReadData((size_t)(addr2 + 1)) * 0x100 + (uint16_t)emulator.chipset.mmu.ReadData((size_t)addr2);
				if (data_operator == 2) {
					flag = 1;
				}
				else {
					flag = 0;
				}
				uint32_t res = Calculate(tmp,(uint32_t)val1, (uint32_t)val2, flag);
				tmp = (res >> 16) & 1;
				if ((res & 0xFFFF) == 0 && data_F410_tmp != 0) {
					data_F410_tmp = 1;
				}
				else {
					data_F410_tmp = 0;
				}
				if (storeresults) {
					uint16_t storeaddr = CalcAddr(data_type_1, offset);
					emulator.chipset.mmu.WriteData((size_t)storeaddr, (uint8_t)(res & 0xFF));
					emulator.chipset.mmu.WriteData((size_t)(storeaddr + 1), (uint8_t)((res >> 8) & 0xFF));
				}
				offset += 2;
				addr1 = CalcAddr(data_type_1, offset);
				addr2 = CalcAddr(data_type_2, offset);
				val1 = (uint16_t)emulator.chipset.mmu.ReadData((size_t)(addr1 + 1)) * 0x100 + (uint16_t)emulator.chipset.mmu.ReadData((size_t)addr1);
				val2 = (uint16_t)emulator.chipset.mmu.ReadData((size_t)(addr2 + 1)) * 0x100 + (uint16_t)emulator.chipset.mmu.ReadData((size_t)addr2);
				res = Calculate(tmp, (uint32_t)val1, (uint32_t)val2, flag);
				if (i * 2 + 1 != data_F402_copy) {
					tmp = (res >> 16) & 1;
					if ((res & 0xFFFF) == 0 && data_F410_tmp != 0) {
						data_F410_tmp = 1;
					}
					else {
						data_F410_tmp = 0;
					}
				}
				data_F410 = (uint8_t)(((tmp * 2) | data_F410_tmp) << 6);
				if (storeresults) {
					uint16_t storeaddr = CalcAddr(data_type_1, offset);
					if (i * 2 + 1 == data_F402_copy) {
						emulator.chipset.mmu.WriteData((size_t)storeaddr, 0);
						emulator.chipset.mmu.WriteData((size_t)(storeaddr + 1), 0);
					}
					else {
						emulator.chipset.mmu.WriteData((size_t)storeaddr, (uint8_t)(res & 0xFF));
						emulator.chipset.mmu.WriteData((size_t)(storeaddr + 1), (uint8_t)((res >> 8) & 0xFF));
					}
				}
			}
		}
		if (data_operator == 1 || data_operator == 2) {
			if (param2 == 1 || param3 == 1) {
				param4 += 2;
				if (param4 >= data_F402_copy)
					param1 = 0;
			}
		}
		uint8_t sign = data_operator;
		if (param1 == 0) {
			sign = 0;
		}
		else {
			sign &= 0x0F;
		}
		sign -= 8;
		switch (sign) {
		case 0:
			ShiftLeft(0);
			break;
		case 1:
			ShiftRight(0);
			break;
		case 2:
			uint16_t addr;
			for (uint8_t offset = 1; offset < 0x0C; offset++) {
				addr = CalcAddr(data_type_1, offset);
				emulator.chipset.mmu.WriteData(addr, 0);
			}
			addr = CalcAddr(data_type_1, 0);
			if (data_type_2 == 3) {
				emulator.chipset.mmu.WriteData((size_t)addr, 5);
			}
			else {
				emulator.chipset.mmu.WriteData((size_t)addr, data_type_2);
			}
			break;
		case 3:
			uint8_t val;
			for (uint8_t offset = 0; offset < 0x0C; offset++) {
				addr = CalcAddr(data_type_2, offset);
				val = emulator.chipset.mmu.ReadData((size_t)addr);
				addr = CalcAddr(data_type_1, offset);
				emulator.chipset.mmu.WriteData((size_t)addr, val);
			}
			break;
		case 4:
			ShiftLeft(1);
			break;
		case 5:
			ShiftRight(1);
			break;
		default:
			break;
		}
		uint8_t start = 0;
		uint8_t end = 0;
		if ((data_F400 & 0xF0) == 0 || (param3 == 1 && param2 != 1)) {
			uint8_t offset = 0x0B;
			bool flag = true;
			do {
				if (flag == false)
					break;
				uint16_t addr = CalcAddr(data_type_1, offset);
				uint8_t value = emulator.chipset.mmu.ReadData((size_t)addr);
				if (offset < data_F402_copy * 2) {
					if ((value & 0xF0) != 0) {
						flag = false;
					}
					else {
						end++;
						if ((value & 0x0F) != 0) {
							flag = false;
						}
						else {
							end++;
						}
					}
				}
				else {
					end += 2;
				}
				offset--;
			} while (offset != 0xFF);
			offset = 0;
			flag = true;
			do {
				if (flag == false)
					break;
				uint16_t addr = CalcAddr(data_type_1, offset);
				uint8_t value = emulator.chipset.mmu.ReadData((size_t)addr);
				if (offset < data_F402_copy * 2) {
					if ((value & 0x0F) != 0) {
						flag = false;
					}
					else {
						start++;
						if ((value & 0xF0) != 0) {
							flag = false;
						}
						else {
							start++;
						}
					}
				}
				else {
					end += 2;
				}
				offset++;
			} while (offset <= 0x0B);
			data_F414 = start;
			data_F415 = end;
		}
		if (data_F400 != 0 && (data_F400 & 0x08) == 0)
			return;
		param1 = 0;
		data_F402_copy = 6;
		return;
	}

	void BCDCalc::Tick() {
		if (F402_write) {
			if (data_F402 == 0)
				data_F402 = 1;
			if (data_F402 > 6)
				data_F402 = 6;
			F402_write = false;
			return;
		}
		if (F404_write) {
			data_F404 &= 0x1F;
			F404_write = false;
			return;
		}
		if (F400_write || F405_write) {
			data_mode = 0x3F;
			data_a = 0;
			data_b = 0;
			data_c = 0;
			data_d = 0;
			data_F404_copy = 0;
			data_operator = 0;
			data_type_1 = 0;
			data_type_2 = 0;
			param1 = 0;
			param2 = 0;
			param3 = 0;
			param4 = 0;
			if (data_F400 != 0xFF) {
				GenerateParams();
				data_F400 = 0xFF;
			}
			data_F402_copy = data_F402;
			if (data_F405 & 0x7F) {
				data_F404_copy = data_F404;
				data_F405 = 0;
				data_mode = 0xFF;
			}
			do {
				if (param1 == 0 && data_mode == 0x3F) {
					data_repeat_flag = 0;
				}
				else {
					data_repeat_flag = 1;
				}
				F405control();
				param3 = param2;
				param2 = param1;
				DataOperate();
			} while (data_repeat_flag == 1);
			F400_write = false;
			F405_write = false;
		}
	}
	void BCDCalc::Reset() {
		F400_write = false;
		F402_write = false;
		F404_write = false;
		F405_write = false;
		data_F400 = 0xFF;
		data_F402 = 0;
		data_F402_copy = 0;
		data_F404 = 0;
		data_F405 = 0;
	}
}