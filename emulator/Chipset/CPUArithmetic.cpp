#include "CPU.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "MMU.hpp"

namespace casioemu
{
	// * Arithmetic Instructions
	void CPU::OP_ADD()
	{
		impl_flags_in &= ~PSW_C;
		impl_flags_in |= PSW_Z;
		OP_ADDC();
	}

	void CPU::OP_ADD16()
	{
		if (impl_hint & H_IE)
			impl_operands[1].value |= (impl_operands[1].value & 0x40) ? 0xFF80 : 0;

		impl_flags_in &= ~PSW_C;

		uint8_t op_high_0 = impl_operands[0].value >> 8;
		uint8_t op_high_1 = impl_operands[1].value >> 8;
		Add8();
		ZSCheck();

		impl_flags_in = (impl_flags_in & ~PSW_C) | (impl_flags_out & PSW_C);

		uint8_t op_low_0 = impl_operands[0].value;
		impl_operands[0].value = op_high_0;
		impl_operands[1].value = op_high_1;
		Add8();
		ZSCheck();

		impl_operands[0].value = (impl_operands[0].value << 8) | op_low_0;
	}

	void CPU::OP_ADDC()
	{
		Add8();
		if (!(impl_flags_in & PSW_Z))
			impl_flags_out &= ~PSW_Z;
		ZSCheck();
	}

	void CPU::OP_AND()
	{
		impl_operands[0].value &= impl_operands[1].value & 0xFF;
		ZSCheck();
	}

	void CPU::OP_MOV16()
	{
		if (impl_hint & H_IE)
			impl_operands[1].value |= (impl_operands[1].value & 0x40) ? 0xFF80 : 0;

		impl_operands[0].value = impl_operands[1].value & 0xFF;
		ZSCheck();

		uint8_t op_low_0 = impl_operands[0].value;
		impl_operands[0].value = (impl_operands[1].value >> 8) & 0xFF;
		ZSCheck();

		impl_operands[0].value = (impl_operands[0].value << 8) | op_low_0;
	}

	void CPU::OP_MOV()
	{
		impl_operands[0].value = impl_operands[1].value & 0xFF;
		ZSCheck();
	}

	void CPU::OP_OR()
	{
		impl_operands[0].value |= impl_operands[1].value & 0xFF;
		ZSCheck();
	}

	void CPU::OP_XOR()
	{
		impl_operands[0].value ^= impl_operands[1].value & 0xFF;
		ZSCheck();
	}

	void CPU::OP_CMP16()
	{
		impl_flags_in &= ~PSW_C;

		uint8_t op_high_0 = impl_operands[0].value >> 8;
		uint8_t op_high_1 = impl_operands[1].value >> 8;
		impl_operands[0].value ^= 0xFF;
		Add8();
		impl_operands[0].value ^= 0xFF;
		ZSCheck();

		impl_flags_in = (impl_flags_in & ~PSW_C) | (impl_flags_out & PSW_C);

		uint8_t op_low_0 = impl_operands[0].value;
		impl_operands[0].value = op_high_0;
		impl_operands[1].value = op_high_1;
		impl_operands[0].value ^= 0xFF;
		Add8();
		impl_operands[0].value ^= 0xFF;
		ZSCheck();

		impl_operands[0].value = (impl_operands[0].value << 8) | op_low_0;
	}

	void CPU::OP_SUB()
	{
		impl_flags_in &= ~PSW_C;
		impl_flags_in |= PSW_Z;
		OP_SUBC();
	}

	void CPU::OP_SUBC()
	{
		impl_operands[0].value ^= 0xFF;
		Add8();
		impl_operands[0].value ^= 0xFF;
		if (!(impl_flags_in & PSW_Z))
			impl_flags_out &= ~PSW_Z;
		ZSCheck();
	}

	// * Shift Instructions
	void CPU::OP_SLL()
	{
		impl_shift_buffer = 0;
		ShiftLeft8();
	}

	void CPU::OP_SLLC()
	{
		size_t external_reg_index = (impl_operands[0].register_index - 1) & 15;
		impl_shift_buffer = reg_r[external_reg_index];
		ShiftLeft8();
	}

	void CPU::OP_SRA()
	{
		size_t shift_by = impl_operands[1].value & 7;
		uint64_t msb = impl_operands[0].value & 0x80;
		impl_shift_buffer = 0;
		ShiftRight8();
		if (msb)
			impl_operands[0].value |= (0xFF >> shift_by) ^ 0xFF;
	}

	void CPU::OP_SRL()
	{
		impl_shift_buffer = 0;
		ShiftRight8();
	}

	void CPU::OP_SRLC()
	{
		size_t external_reg_index = (impl_operands[0].register_index + 1) & 15;
		impl_shift_buffer = reg_r[external_reg_index];
		ShiftRight8();
	}

	// * ALU Instructions
	void CPU::OP_DAA()
	{
		impl_operands[1].value = 0;
		if ((impl_operands[0].value & 0x0F) > 0x09 || (impl_flags_in & PSW_HC)) impl_operands[1].value |= 0x06;
		if ((impl_operands[0].value & 0xF0) > 0x90 || (impl_flags_in &  PSW_C)) impl_operands[1].value |= 0x60;
		if ((impl_operands[0].value & 0xF0) == 0x90 && (impl_operands[0].value & 0x0F) > 0x09 && !(impl_flags_in & PSW_HC)) impl_operands[1].value |= 0x60;
		uint8_t flags_in_backup = impl_flags_in;
		OP_ADD();
		impl_flags_out |= flags_in_backup & PSW_C;
		impl_flags_changed &= ~PSW_OV;
	}

	void CPU::OP_DAS()
	{
		impl_operands[1].value = 0;
		if ((impl_operands[0].value & 0x0F) > 0x09 || (impl_flags_in & PSW_HC)) impl_operands[1].value |= 0x06;
		if ((impl_operands[0].value & 0xF0) > 0x90 || (impl_flags_in &  PSW_C)) impl_operands[1].value |= 0x60;
		uint8_t flags_in_backup = impl_flags_in;
		OP_SUB();
		impl_flags_out |= flags_in_backup & PSW_C;
		impl_flags_changed &= ~PSW_OV;
	}

	void CPU::OP_NEG()
	{
		impl_operands[1].value = impl_operands[0].value;
		impl_operands[0].value = 0;
		OP_SUB();
	}

	// * Bit Access Instructions
	void CPU::OP_BITMOD()
	{
		size_t src_index;
		uint64_t bit_in = 1 << impl_operands[1].value;
		if (impl_hint & H_TI)
		{
			src_index = impl_long_imm;
			impl_operands[0].value = emulator.chipset.mmu.ReadData((((size_t)reg_dsr) << 16) | src_index);
		}
		else
		{
			src_index = impl_operands[0].value;
			impl_operands[0].value = reg_r[src_index];
		}

		impl_flags_changed |= PSW_Z;
		impl_flags_out = (impl_operands[0].value & bit_in) ? 0 : PSW_Z;

		switch (impl_opcode & 0x000F)
		{
		case 0:
			impl_operands[0].value |= bit_in;
			break;
		case 2:
			impl_operands[0].value &= ~bit_in;
			break;
		}

		if ((impl_opcode & 0x000F) != 1)
		{
			if (impl_hint & H_TI)
				emulator.chipset.mmu.WriteData((((size_t)reg_dsr) << 16) | src_index, impl_operands[0].value);
			else
				reg_r[src_index] = impl_operands[0].value;
		}
	}

	// * Sign Extension Instruction
	void CPU::OP_EXTBW()
	{
		size_t index = (impl_opcode & 0x00E0) >> 4;
		impl_operands[0].value = (reg_r[index] & 0x80) ? 0xFF : 0x00;
		reg_r[index + 1] = impl_operands[0].value;
		ZSCheck();
	}

	// * Multiplication and Division Instructions
	void CPU::OP_MUL()
	{
		impl_operands[0].value &= 0xFF;
		impl_operands[0].value *= impl_operands[1].value;

		impl_flags_changed |= PSW_Z;
		impl_flags_out = impl_operands[0].value ? 0 : PSW_Z;
	}

	void CPU::OP_DIV()
	{
		impl_flags_changed |= PSW_Z | PSW_C;
		if (!impl_operands[1].value)
		{
			impl_flags_out |= PSW_C;
			return;
		}

		uint16_t quotient = impl_operands[0].value / impl_operands[1].value;
		uint16_t remainder = impl_operands[0].value % impl_operands[1].value;

		impl_operands[0].value = quotient;
		if (impl_operands[0].value)
			impl_flags_out &= ~PSW_Z;

		size_t remainder_reg_index = (impl_opcode >> 4) & 0x000F;
		reg_r[remainder_reg_index] = remainder;
	}

	// * Miscellaneous Instructions
	void CPU::OP_INC_EA()
	{
		impl_operands[0].value = emulator.chipset.mmu.ReadData((((size_t)reg_dsr) << 16) | reg_ea);
		impl_operands[1].value = 1;
		OP_ADD();
		impl_flags_changed &= ~PSW_C;
		emulator.chipset.mmu.WriteData((((size_t)reg_dsr) << 16) | reg_ea, impl_operands[0].value);
	}

	void CPU::OP_DEC_EA()
	{
		impl_operands[0].value = emulator.chipset.mmu.ReadData((((size_t)reg_dsr) << 16) | reg_ea);
		impl_operands[1].value = 1;
		OP_SUB();
		impl_flags_changed &= ~PSW_C;
		emulator.chipset.mmu.WriteData((((size_t)reg_dsr) << 16) | reg_ea, impl_operands[0].value);
	}

	void CPU::Add8()
	{
		uint8_t op8[2] = {(uint8_t)impl_operands[0].value, (uint8_t)impl_operands[1].value};
		uint16_t c_in = (impl_flags_in & PSW_C) ? 1 : 0;

		bool carry_8 = (((uint16_t)op8[0] & 0xFF) + (op8[1] & 0xFF) + c_in) >> 8;
		bool carry_7 = (((uint16_t)op8[0] & 0x7F) + (op8[1] & 0x7F) + c_in) >> 7;
		bool carry_4 = (((uint16_t)op8[0] & 0x0F) + (op8[1] & 0x0F) + c_in) >> 4;

		impl_flags_changed |= PSW_C | PSW_OV | PSW_HC;
		impl_flags_out = (impl_flags_out & ~PSW_C) | (carry_8 ? PSW_C : 0);
		impl_flags_out = (impl_flags_out & ~PSW_OV) | ((carry_8 ^ carry_7) ? PSW_OV : 0);
		impl_flags_out = (impl_flags_out & ~PSW_HC) | (carry_4 ? PSW_HC : 0);

		impl_operands[0].value = (uint8_t)(op8[0] + op8[1] + c_in);
	}

	void CPU::ZSCheck()
	{
		impl_flags_changed |= PSW_Z | PSW_S;
		if (impl_operands[0].value & 0xFF)
			impl_flags_out &= ~PSW_Z;
		impl_flags_out = (impl_flags_out & ~PSW_S) | ((impl_operands[0].value & 0x80) ? PSW_S : 0);
	}

	void CPU::ShiftLeft8()
	{
		impl_operands[0].value &= 0xFF;
		size_t shift_by = impl_operands[1].value & 7;
		uint16_t result = (uint16_t)impl_operands[0].value << shift_by;
		result |= impl_shift_buffer >> (8 - shift_by);
		impl_flags_changed |= PSW_C;
		if (result & 0x100)
			impl_flags_out |= PSW_C;
		impl_operands[0].value = (uint8_t)result;
	}

	void CPU::ShiftRight8()
	{
		impl_operands[0].value &= 0xFF;
		size_t shift_by = impl_operands[1].value & 7;
		uint16_t result = (uint16_t)impl_operands[0].value << (8 - shift_by);
		result |= (uint16_t)impl_shift_buffer << (16 - shift_by);
		impl_flags_changed |= PSW_C;
		if (result & 0x80)
			impl_flags_out |= PSW_C;
		impl_operands[0].value = (uint8_t)(result >> 8);
	}
}

