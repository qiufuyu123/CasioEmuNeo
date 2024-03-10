#include "CPU.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "MMU.hpp"
#include "../Logger.hpp"
#include "../Gui/Ui.hpp"

#include <sstream>
#include <iomanip>

namespace casioemu
{
	CPU::OpcodeSource CPU::opcode_sources[] = {
		//           function,                     hints, main mask, operand {size, mask, shift} x2
		// * Arithmetic Instructions
		{&CPU::OP_ADD        , H_WB                     , 0x8001, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_ADD        , H_WB                     , 0x1000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_ADD16      , H_WB                     , 0xF006, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_ADD16      , H_WB               | H_IE, 0xE080, {{2, 0x000E,  8}, {0, 0x007F,  0}}},
		{&CPU::OP_ADDC       , H_WB                     , 0x8006, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_ADDC       , H_WB                     , 0x6000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_AND        , H_WB                     , 0x8002, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_AND        , H_WB                     , 0x2000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_SUB        ,                         0, 0x8007, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SUB        ,                         0, 0x7000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_SUBC       ,                         0, 0x8005, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SUBC       ,                         0, 0x5000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_MOV16      , H_WB                     , 0xF005, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_MOV16      , H_WB               | H_IE, 0xE000, {{2, 0x000E,  8}, {0, 0x007F,  0}}},
		{&CPU::OP_MOV        , H_WB                     , 0x8000, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_MOV        , H_WB                     , 0x0000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_OR         , H_WB                     , 0x8003, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_OR         , H_WB                     , 0x3000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_XOR        , H_WB                     , 0x8004, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_XOR        , H_WB                     , 0x4000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_CMP16      ,                         0, 0xF007, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_SUB        , H_WB                     , 0x8008, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SUBC       , H_WB                     , 0x8009, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		// * Shift Instructions
		{&CPU::OP_SLL        , H_WB                     , 0x800A, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SLL        , H_WB                     , 0x900A, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SLLC       , H_WB                     , 0x800B, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SLLC       , H_WB                     , 0x900B, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SRA        , H_WB                     , 0x800E, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SRA        , H_WB                     , 0x900E, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SRL        , H_WB                     , 0x800C, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SRL        , H_WB                     , 0x900C, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SRLC       , H_WB                     , 0x800D, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SRLC       , H_WB                     , 0x900D, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		// * Load/Store Instructions
		{&CPU::OP_LS_EA      , 2 << 8                   , 0x9032, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 2 << 8 |      H_IA       , 0x9052, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       , 2 << 8                   , 0x9002, {{0, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     , 2 << 8 |      H_TI       , 0xA008, {{0, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_BP      , 2 << 8 |                0, 0xB000, {{0, 0x000E,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_FP      , 2 << 8 |                0, 0xB040, {{0, 0x000E,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_I       , 2 << 8 |      H_TI       , 0x9012, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 1 << 8                   , 0x9030, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 1 << 8 |      H_IA       , 0x9050, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       , 1 << 8                   , 0x9000, {{0, 0x000F,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     , 1 << 8 |      H_TI       , 0x9008, {{0, 0x000F,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_BP      , 1 << 8 |                0, 0xD000, {{0, 0x000F,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_FP      , 1 << 8 |                0, 0xD040, {{0, 0x000F,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_I       , 1 << 8 |      H_TI       , 0x9010, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 4 << 8                   , 0x9034, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 4 << 8 |      H_IA       , 0x9054, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 8 << 8                   , 0x9036, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 8 << 8 |      H_IA       , 0x9056, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 2 << 8 |             H_ST, 0x9033, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 2 << 8 |      H_IA | H_ST, 0x9053, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       , 2 << 8 |             H_ST, 0x9003, {{0, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     , 2 << 8 |      H_TI | H_ST, 0xA009, {{0, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_BP      , 2 << 8 |             H_ST, 0xB080, {{0, 0x000E,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_FP      , 2 << 8 |             H_ST, 0xB0C0, {{0, 0x000E,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_I       , 2 << 8 |      H_TI | H_ST, 0x9013, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 1 << 8 |             H_ST, 0x9031, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 1 << 8 |      H_IA | H_ST, 0x9051, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       , 1 << 8 |             H_ST, 0x9001, {{0, 0x000F,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     , 1 << 8 |      H_TI | H_ST, 0x9009, {{0, 0x000F,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_BP      , 1 << 8 |             H_ST, 0xD080, {{0, 0x000F,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_FP      , 1 << 8 |             H_ST, 0xD0C0, {{0, 0x000F,  8}, {0, 0x003F,  0}}},
		{&CPU::OP_LS_I       , 1 << 8 |      H_TI | H_ST, 0x9011, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 4 << 8 |             H_ST, 0x9035, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 4 << 8 |      H_IA | H_ST, 0x9055, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 8 << 8 |             H_ST, 0x9037, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      , 8 << 8 |      H_IA | H_ST, 0x9057, {{0, 0x0008,  8}, {0,      0,  0}}},
		// * Control Register Access Instructions
		{&CPU::OP_ADDSP      ,                         0, 0xE100, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                    1 << 8, 0xA00F, {{0,      0,  0}, {1, 0x000F,  4}}},
		{&CPU::OP_CTRL       ,                    2 << 8, 0xA00D, {{0,      0,  0}, {2, 0x000E,  8}}},
		{&CPU::OP_CTRL       ,                    3 << 8, 0xA00C, {{0,      0,  0}, {1, 0x000F,  4}}},
		{&CPU::OP_CTRL       , H_WB            |  4 << 8, 0xA005, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       , H_WB            |  5 << 8, 0xA01A, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                    6 << 8, 0xA00B, {{0,      0,  0}, {1, 0x000F,  4}}},
		{&CPU::OP_CTRL       ,                    7 << 8, 0xE900, {{0,      0,  0}, {0, 0x00FF,  0}}},
		{&CPU::OP_CTRL       , H_WB            |  8 << 8, 0xA007, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       , H_WB            |  9 << 8, 0xA004, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       , H_WB            | 10 << 8, 0xA003, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                   11 << 8, 0xA10A, {{0,      0,  0}, {2, 0x000E,  4}}},
		// * PUSH/POP Instructions
		{&CPU::OP_PUSH       ,                         0, 0xF05E, {{0,      0,  0}, {2, 0x000E,  8}}},
		{&CPU::OP_PUSH       ,                         0, 0xF07E, {{0,      0,  0}, {8, 0x0008,  8}}},
		{&CPU::OP_PUSH       ,                         0, 0xF04E, {{0,      0,  0}, {1, 0x000F,  8}}},
		{&CPU::OP_PUSH       ,                         0, 0xF06E, {{0,      0,  0}, {4, 0x000C,  8}}},
		{&CPU::OP_PUSHL      ,                         0, 0xF0CE, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_POP        , H_WB                     , 0xF01E, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_POP        , H_WB                     , 0xF03E, {{8, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_POP        , H_WB                     , 0xF00E, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_POP        , H_WB                     , 0xF02E, {{4, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_POPL       ,                         0, 0xF08E, {{0, 0x000F,  8}, {0,      0,  0}}},
		// * Coprocessor Data Transfer Instructions
		{&CPU::OP_CR_R       ,                         0, 0xA00E, {{0, 0x000F,  8}, {0, 0x000F,  4}}},
		{&CPU::OP_CR_EA      ,      2 << 8 |           0, 0xF02D, {{0,      0,  0}, {0, 0x000E,  8}}},
		{&CPU::OP_CR_EA      ,      2 << 8 | H_IA       , 0xF03D, {{0,      0,  0}, {0, 0x000E,  8}}},
		{&CPU::OP_CR_EA      ,      1 << 8 |           0, 0xF00D, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_CR_EA      ,      1 << 8 | H_IA       , 0xF01D, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_CR_EA      ,      4 << 8 |           0, 0xF04D, {{0,      0,  0}, {0, 0x000C,  8}}},
		{&CPU::OP_CR_EA      ,      4 << 8 | H_IA       , 0xF05D, {{0,      0,  0}, {0, 0x000C,  8}}},
		{&CPU::OP_CR_EA      ,      8 << 8 |           0, 0xF06D, {{0,      0,  0}, {0, 0x0008,  8}}},
		{&CPU::OP_CR_EA      ,      8 << 8 | H_IA       , 0xF07D, {{0,      0,  0}, {0, 0x0008,  8}}},
		{&CPU::OP_CR_R       ,                      H_ST, 0xA006, {{0, 0x000F,  8}, {0, 0x000F,  4}}},
		{&CPU::OP_CR_EA      ,      2 << 8 |        H_ST, 0xF0AD, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      2 << 8 | H_IA | H_ST, 0xF0BD, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      1 << 8 |        H_ST, 0xF08D, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      1 << 8 | H_IA | H_ST, 0xF09D, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      4 << 8 |        H_ST, 0xF0CD, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      4 << 8 | H_IA | H_ST, 0xF0DD, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      8 << 8 |        H_ST, 0xF0ED, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,      8 << 8 | H_IA | H_ST, 0xF0FD, {{0, 0x0008,  8}, {0,      0,  0}}},
		// * EA Register Data Transfer Instructions
		{&CPU::OP_LEA        ,                         0, 0xF00A, {{0,      0,  0}, {2, 0x000E,  4}}},
		{&CPU::OP_LEA        ,        H_TI              , 0xF00B, {{0,      0,  0}, {2, 0x000E,  4}}},
		{&CPU::OP_LEA        ,        H_TI              , 0xF00C, {{0,      0,  0}, {0,      0,  0}}},
		// * ALU Instructions
		{&CPU::OP_DAA        , H_WB                     , 0x801F, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_DAS        , H_WB                     , 0x803F, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_NEG        , H_WB                     , 0x805F, {{1, 0x000F,  8}, {0,      0,  0}}},
		// * Bit Access Instructions
		{&CPU::OP_BITMOD     ,                         0, 0xA000, {{0, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,        H_TI              , 0xA080, {{0,      0,  0}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                         0, 0xA002, {{0, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,        H_TI              , 0xA082, {{0,      0,  0}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                         0, 0xA001, {{0, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,        H_TI              , 0xA081, {{0,      0,  0}, {0, 0x0007,  4}}},
		// * PSW Access Instructions
		{&CPU::OP_PSW_OR     ,                         0, 0xED08, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_PSW_AND    ,                         0, 0xEBF7, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_PSW_OR     ,                         0, 0xED80, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_PSW_AND    ,                         0, 0xEB7F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_CPLC       ,                         0, 0xFECF, {{0,      0,  0}, {0,      0,  0}}},
		// * Conditional Relative Branch Instructions
		{&CPU::OP_BC         ,                         0, 0xC000, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC100, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC200, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC300, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC400, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC500, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC600, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC700, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC800, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xC900, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xCA00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xCB00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xCC00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xCD00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                         0, 0xCE00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		// * Sign Extension Instruction
		{&CPU::OP_EXTBW      ,                         0, 0x810F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x832F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x854F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x876F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x898F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x8BAF, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x8DCF, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                         0, 0x8FEF, {{0,      0,  0}, {0,      0,  0}}},
		// * Software Interrupt Instructions
		{&CPU::OP_SWI        ,                         0, 0xE500, {{0, 0x003F,  0}, {0,      0,  0}}},
		{&CPU::OP_BRK        ,                         0, 0xFFFF, {{0,      0,  0}, {0,      0,  0}}},
		// * Branch Instructions
		{&CPU::OP_B          ,        H_TI              , 0xF000, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_B          ,                         0, 0xF002, {{0,      0,  0}, {2, 0x000E,  4}}},
		{&CPU::OP_BL         ,        H_TI              , 0xF001, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_BL         ,                         0, 0xF003, {{0,      0,  0}, {2, 0x000E,  4}}},
		// * Multiplication and Division Instructions
		{&CPU::OP_MUL        , H_WB                     , 0xF004, {{2, 0x000E,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_DIV        , H_WB                     , 0xF009, {{2, 0x000E,  8}, {1, 0x000F,  4}}},
		// * Miscellaneous Instructions
		{&CPU::OP_INC_EA     ,                         0, 0xFE2F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_DEC_EA     ,                         0, 0xFE3F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_RT         ,                         0, 0xFE1F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_RTI        ,                         0, 0xFE0F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_NOP        ,                         0, 0xFE8F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_DSR        ,               H_DS       , 0xFE9F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_DSR        ,               H_DS | H_DW, 0xE300, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_DSR        ,               H_DS | H_DW, 0x900F, {{1, 0x000F,  4}, {0,      0,  0}}}
	};

	CPU::RegisterRecord CPU::register_record_sources[] = {
		{    "r", 16, 0, nullptr,    (RegisterStubArrayPointer)&CPU::reg_r},
		{   "cr", 16, 0, nullptr,   (RegisterStubArrayPointer)&CPU::reg_cr},
		{   "pc",  1, 0,        (RegisterStubPointer)&CPU::reg_pc, nullptr},
		{  "csr",  1, 0,       (RegisterStubPointer)&CPU::reg_csr, nullptr},
		{   "lr",  1, 0, nullptr,  (RegisterStubArrayPointer)&CPU::reg_elr},
		{ "elr1",  1, 1, nullptr,  (RegisterStubArrayPointer)&CPU::reg_elr},
		{ "elr2",  1, 2, nullptr,  (RegisterStubArrayPointer)&CPU::reg_elr},
		{ "elr3",  1, 3, nullptr,  (RegisterStubArrayPointer)&CPU::reg_elr},
		{ "lcsr",  1, 0, nullptr, (RegisterStubArrayPointer)&CPU::reg_ecsr},
		{"ecsr1",  1, 1, nullptr, (RegisterStubArrayPointer)&CPU::reg_ecsr},
		{"ecsr2",  1, 2, nullptr, (RegisterStubArrayPointer)&CPU::reg_ecsr},
		{"ecsr3",  1, 3, nullptr, (RegisterStubArrayPointer)&CPU::reg_ecsr},
		{  "psw",  1, 0, nullptr, (RegisterStubArrayPointer)&CPU::reg_epsw},
		{"epsw1",  1, 1, nullptr, (RegisterStubArrayPointer)&CPU::reg_epsw},
		{"epsw2",  1, 2, nullptr, (RegisterStubArrayPointer)&CPU::reg_epsw},
		{"epsw3",  1, 3, nullptr, (RegisterStubArrayPointer)&CPU::reg_epsw},
		{   "sp",  1, 0,        (RegisterStubPointer)&CPU::reg_sp, nullptr},
		{   "ea",  1, 0,        (RegisterStubPointer)&CPU::reg_ea, nullptr},
		{  "dsr",  1, 0,       (RegisterStubPointer)&CPU::reg_dsr, nullptr}
	};

	void CPU::OP_NOP()
	{
	}

	void CPU::OP_DSR()
	{
		if (impl_hint & H_DW)
			impl_last_dsr = impl_operands[0].value;

		reg_dsr = impl_last_dsr;
	}

	CPU::CPU(Emulator &_emulator) : emulator(_emulator), reg_lr(reg_elr[0]), reg_lcsr(reg_ecsr[0]), reg_psw(reg_epsw[0])
	{
		opcode_dispatch = new OpcodeSource *[0x10000];
		for (size_t ix = 0; ix != 0x10000; ++ix)
			opcode_dispatch[ix] = nullptr;
	}

	CPU::~CPU()
	{
		delete[] opcode_dispatch;
	}

	void CPU::SetupInternals()
	{
		SetupOpcodeDispatch();
		SetupRegisterProxies();

		impl_csr_mask = emulator.GetModelInfo("csr_mask");
	}

	void CPU::SetupOpcodeDispatch()
	{
		uint16_t *permutation_buffer = new uint16_t[0x10000];
		for (size_t ix = 0; ix != sizeof(opcode_sources) / sizeof(opcode_sources[0]); ++ix)
		{
			OpcodeSource &handler_stub = opcode_sources[ix];

			uint16_t varying_bits = 0;
			for (size_t ox = 0; ox != sizeof(impl_operands) / sizeof(impl_operands[0]); ++ox)
				varying_bits |= handler_stub.operands[ox].mask << handler_stub.operands[ox].shift;

			size_t permutation_count = 1;
			permutation_buffer[0] = handler_stub.opcode;
			for (uint16_t checkbit = 0x8000; checkbit; checkbit >>= 1)
			{
				if (varying_bits & checkbit)
				{
					for (size_t px = 0; px != permutation_count; ++px)
						permutation_buffer[px + permutation_count] = permutation_buffer[px] | checkbit;
					permutation_count <<= 1;
				}
			}

			for (size_t px = 0; px != permutation_count; ++px)
			{
				if (opcode_dispatch[permutation_buffer[px]])
					continue;
				opcode_dispatch[permutation_buffer[px]] = &handler_stub;
			}
		}
		delete[] permutation_buffer;
	}

	void CPU::SetupRegisterProxies()
	{
		for (size_t ix = 0; ix != sizeof(register_record_sources) / sizeof(register_record_sources[0]); ++ix)
		{
			RegisterRecord &record = register_record_sources[ix];

			if (record.stub)
			{
				RegisterStub *register_stub = &(this->*record.stub);
				register_stub->name = record.name;
				register_proxies[record.name] = register_stub;
			}

			if (record.stub_array)
			{
				if (record.array_size == 1)
				{
					RegisterStub *register_stub = &(this->*record.stub_array)[record.array_base];
					register_stub->name = record.name;
					register_proxies[record.name] = register_stub;
				}
				else
				{
					for (size_t rx = 0; rx != record.array_size; ++rx)
					{
						std::stringstream ss;
						ss << record.name << rx;
						RegisterStub *register_stub = &(this->*record.stub_array)[rx];
						register_stub->name = ss.str();
						register_proxies[ss.str()] = register_stub;
					}
				}
			}
		}

		*(CPU **)lua_newuserdata(emulator.lua_state, sizeof(CPU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			CPU *cpu = *(CPU **)lua_topointer(lua_state, 1);
			std::string index = lua_tostring(lua_state, 2);
			if (index == "bt")
			{
				lua_pushstring(lua_state, cpu->GetBacktrace().c_str());
				return 1;
			}
			auto it = cpu->register_proxies.find(index);
			if (it == cpu->register_proxies.end())
				return 0;
			RegisterStub *reg_stub = it->second;
			if (reg_stub->type_size == 1)
				lua_pushinteger(lua_state, (uint8_t)reg_stub->raw);
			else
				lua_pushinteger(lua_state, (uint16_t)reg_stub->raw);
			return 1;
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			CPU *cpu = *(CPU **)lua_topointer(lua_state, 1);
			auto it = cpu->register_proxies.find(lua_tostring(lua_state, 2));
			if (it == cpu->register_proxies.end())
				return 0;
			RegisterStub *reg_stub = it->second;
			if (reg_stub->type_size == 1)
				reg_stub->raw = (uint8_t)lua_tointeger(lua_state, 3);
			else
				reg_stub->raw = (uint16_t)lua_tointeger(lua_state, 3);
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "cpu");
	}

	uint16_t CPU::Fetch()
	{
		if (reg_csr.raw & ~impl_csr_mask)
			reg_csr.raw &= impl_csr_mask;
		if (reg_pc.raw & 1)
			reg_pc.raw &= ~1;
		uint16_t opcode = emulator.chipset.mmu.ReadCode((reg_csr.raw << 16) | reg_pc.raw);
		reg_pc.raw = (uint16_t)(reg_pc.raw + 2);
		return opcode;
	}

	void CPU::Next()
	{
		/**
		 * `reg_dsr` only affects the current instruction. The old DSR is stored in
		 * `impl_last_dsr` and is recalled every time a DSR instruction is encountered
		 * that activates DSR addressing without actually changing DSR.
		 */
		reg_dsr	= 0;

		while (1)
		{
			impl_opcode = Fetch();
			OpcodeSource *handler = opcode_dispatch[impl_opcode];

			if (!handler)
				continue;

			impl_long_imm = 0;
			if (handler->hint & H_TI)
				impl_long_imm = Fetch();

			for (size_t ix = 0; ix != sizeof(impl_operands) / sizeof(impl_operands[0]); ++ix)
			{
				impl_operands[ix].value = (impl_opcode >> handler->operands[ix].shift) & handler->operands[ix].mask;
				impl_operands[ix].register_index = impl_operands[ix].value;
				impl_operands[ix].register_size = handler->operands[ix].register_size;

				if (impl_operands[ix].register_size)
				{
					impl_operands[ix].value = 0;
					for (size_t bx = 0; bx != impl_operands[ix].register_size; ++bx)
						impl_operands[ix].value |= (uint64_t)(reg_r[impl_operands[ix].register_index + bx]) << (bx * 8);
				}
			}
			impl_hint = handler->hint;

			impl_flags_changed = 0;
			impl_flags_in = reg_psw;
			/**
			 * Yes, Z is always set to 1. While `impl_flags_changed` may not have
			 * PSW_Z set, `impl_flags_out` does as most of the time Z is calculated
			 * by one or more calls to `ZSCheck`. `ZSCheck` only changes Z if the
			 * value it checks is non-zero, otherwise it leaves it alone.
			 */
			impl_flags_out = PSW_Z;
			(this->*(handler->handler_function))();
			if(DebugUi::code_viewer){
				if((DebugUi::code_viewer->debug_flags & DEBUG_BREAKPOINT) && DebugUi::code_viewer->TryTrigBP(reg_csr, reg_pc)){
					emulator.SetPaused(true);
				}
				else if((DebugUi::code_viewer->debug_flags)&DEBUG_STEP && DebugUi::code_viewer->TryTrigBP(reg_csr, reg_pc,false)){
					emulator.SetPaused(true);
				}
			}
			reg_psw &= ~impl_flags_changed;
			reg_psw |= impl_flags_out & impl_flags_changed;

			if (handler->hint & H_WB && impl_operands[0].register_size)
				for (size_t bx = 0; bx != impl_operands[0].register_size; ++bx)
					reg_r[impl_operands[0].register_index + bx] = (uint8_t)(impl_operands[0].value >> (bx * 8));

			if (!(handler->hint & H_DS))
				break;
			
		}
	}

	void CPU::SetMemoryModel(MemoryModel _memory_model)
	{
		memory_model = _memory_model;
	}

	void CPU::Reset()
	{
		reg_sp = emulator.chipset.mmu.ReadCode(0);
		reg_dsr = 0;
		reg_psw = 0;
		stack.clear();
	}

	void CPU::Raise(size_t exception_level, size_t index)
	{
		if (exception_level == 1)
			reg_psw.raw &= ~PSW_MIE;
		reg_psw.raw = (reg_psw.raw & ~PSW_ELEVEL) | exception_level;

		reg_elr[exception_level].raw = reg_pc.raw;
		reg_ecsr[exception_level].raw = reg_csr.raw;

		reg_csr.raw = 0;
		reg_pc.raw = emulator.chipset.mmu.ReadCode(index * 2);
	}

	size_t CPU::GetExceptionLevel()
	{
		return reg_psw.raw & PSW_ELEVEL;
	}

	bool CPU::GetMasterInterruptEnable()
	{
		return reg_psw & PSW_MIE;
	}

	std::string CPU::GetBacktrace() const
	{
		std::stringstream output;
		output << std::hex << std::setfill('0') << std::uppercase;
		for (StackFrame frame : stack)
		{
			output << "  function "
				<< std::setw(6) << (((size_t)frame.new_csr) << 16 | frame.new_pc)
				<< " returns to " << std::setw(6);
			if (frame.lr_pushed)
			{
				uint16_t saved_lr, saved_lcsr = 0;
				MMU &mmu = emulator.chipset.mmu;
				saved_lr = ((uint16_t)mmu.ReadData(frame.lr_push_address + 1))
					<< 8 | mmu.ReadData(frame.lr_push_address);
				if (memory_model == MM_LARGE)
					saved_lcsr = mmu.ReadData(frame.lr_push_address + 2);
				output << (((size_t)saved_lcsr) << 16 | saved_lr);

				output << " - lr pushed at "
					<< std::setw(4) << frame.lr_push_address;
			}
			else
			{
				output << (((size_t)reg_lcsr) << 16 | reg_lr);
			}
			output << '\n';
		}
		return output.str();
	}
}

