/**
 * @file src/capstone2llvmir/riscv/riscv_init.cpp
 * @brief Initializations for RISCV implementation of @c Capstone2LlvmIrTranslator.
 */

#include "capstone2llvmir/riscv/riscv_impl.h"

namespace retdec {
namespace capstone2llvmir {

//
//==============================================================================
// Pure virtual methods from Capstone2LlvmIrTranslator_impl
//==============================================================================
//

void Capstone2LlvmIrTranslatorRiscv_impl::initializeArchSpecific()
{
	// Nothing.
}

void Capstone2LlvmIrTranslatorRiscv_impl::initializeRegNameMap()
{
	std::map<uint32_t, std::string> r2n =
	{
			// riscv_reg_fpu_double
			//
			/*{RISCV_REG_FD0, "fd0"},
			{RISCV_REG_FD2, "fd2"},
			{RISCV_REG_FD4, "fd4"},
			{RISCV_REG_FD6, "fd6"},
			{RISCV_REG_FD8, "fd8"},
			{RISCV_REG_FD10, "fd10"},
			{RISCV_REG_FD12, "fd12"},
			{RISCV_REG_FD14, "fd14"},
			{RISCV_REG_FD16, "fd16"},
			{RISCV_REG_FD18, "fd18"},
			{RISCV_REG_FD20, "fd20"},
			{RISCV_REG_FD22, "fd22"},
			{RISCV_REG_FD24, "fd24"},
			{RISCV_REG_FD26, "fd26"},
			{RISCV_REG_FD28, "fd28"},
			{RISCV_REG_FD30, "fd30"},*/
	};

	_reg2name = std::move(r2n);
}

void Capstone2LlvmIrTranslatorRiscv_impl::initializeRegTypeMap()
{
	auto* i1 = llvm::IntegerType::getInt32Ty(_module->getContext());
	auto* i32 = llvm::IntegerType::getInt32Ty(_module->getContext());
	auto* i64 = llvm::IntegerType::getInt64Ty(_module->getContext());
	auto* i128 = llvm::IntegerType::getInt64Ty(_module->getContext());
	auto* f32 = llvm::Type::getFloatTy(_module->getContext());
	auto* f64 = llvm::Type::getDoubleTy(_module->getContext());
	auto* f128 = llvm::Type::getFP128Ty(_module->getContext());

	auto* defTy = _basicMode == CS_MODE_RISCV64 ? i64 : i32;
	auto* defFty = _basicMode == CS_MODE_RISCV64 ? f64 : f32;

	std::map<uint32_t, llvm::Type*> r2t =
	{
			// Program counter.
			//
			//{RISCV_REG_PC, defTy},

			// General purpose registers.
			//
			{RISCV_REG_X0, defTy},
			{RISCV_REG_X1, defTy},
			{RISCV_REG_X2, defTy},
			{RISCV_REG_X3, defTy},
			{RISCV_REG_X4, defTy},
			{RISCV_REG_X5, defTy},
			{RISCV_REG_X6, defTy},
			{RISCV_REG_X7, defTy},
			{RISCV_REG_X8, defTy},
			{RISCV_REG_X9, defTy},
			{RISCV_REG_X10, defTy},
			{RISCV_REG_X11, defTy},
			{RISCV_REG_X12, defTy},
			{RISCV_REG_X13, defTy},
			{RISCV_REG_X14, defTy},
			{RISCV_REG_X15, defTy},
			{RISCV_REG_X16, defTy},
			{RISCV_REG_X17, defTy},
			{RISCV_REG_X18, defTy},
			{RISCV_REG_X19, defTy},
			{RISCV_REG_X20, defTy},
			{RISCV_REG_X21, defTy},
			{RISCV_REG_X22, defTy},
			{RISCV_REG_X23, defTy},
			{RISCV_REG_X24, defTy},
			{RISCV_REG_X25, defTy},
			{RISCV_REG_X26, defTy},
			{RISCV_REG_X27, defTy},
			{RISCV_REG_X28, defTy},
			{RISCV_REG_X29, defTy},
			{RISCV_REG_X30, defTy},
			{RISCV_REG_X31, defTy},


			// FPU registers.
			//
			{RISCV_REG_F0_32, defFty},
			{RISCV_REG_F1_32, defFty},
			{RISCV_REG_F2_32, defFty},
			{RISCV_REG_F3_32, defFty},
			{RISCV_REG_F4_32, defFty},
			{RISCV_REG_F5_32, defFty},
			{RISCV_REG_F6_32, defFty},
			{RISCV_REG_F7_32, defFty},
			{RISCV_REG_F8_32, defFty},
			{RISCV_REG_F9_32, defFty},
			{RISCV_REG_F10_32, defFty},
			{RISCV_REG_F11_32, defFty},
			{RISCV_REG_F12_32, defFty},
			{RISCV_REG_F13_32, defFty},
			{RISCV_REG_F14_32, defFty},
			{RISCV_REG_F15_32, defFty},
			{RISCV_REG_F16_32, defFty},
			{RISCV_REG_F17_32, defFty},
			{RISCV_REG_F18_32, defFty},
			{RISCV_REG_F19_32, defFty},
			{RISCV_REG_F20_32, defFty},
			{RISCV_REG_F21_32, defFty},
			{RISCV_REG_F22_32, defFty},
			{RISCV_REG_F23_32, defFty},
			{RISCV_REG_F24_32, defFty},
			{RISCV_REG_F25_32, defFty},
			{RISCV_REG_F26_32, defFty},
			{RISCV_REG_F27_32, defFty},
			{RISCV_REG_F28_32, defFty},
			{RISCV_REG_F29_32, defFty},
			{RISCV_REG_F30_32, defFty},
			{RISCV_REG_F31_32, defFty},
	};

	// riscv_reg_fpu_double
	//
	/*if (defFty->isFloatTy())
	{
		r2t.emplace(RISCV_REG_FD0, f64);
		r2t.emplace(RISCV_REG_FD2, f64);
		r2t.emplace(RISCV_REG_FD4, f64);
		r2t.emplace(RISCV_REG_FD6, f64);
		r2t.emplace(RISCV_REG_FD8, f64);
		r2t.emplace(RISCV_REG_FD10, f64);
		r2t.emplace(RISCV_REG_FD12, f64);
		r2t.emplace(RISCV_REG_FD14, f64);
		r2t.emplace(RISCV_REG_FD16, f64);
		r2t.emplace(RISCV_REG_FD18, f64);
		r2t.emplace(RISCV_REG_FD20, f64);
		r2t.emplace(RISCV_REG_FD22, f64);
		r2t.emplace(RISCV_REG_FD24, f64);
		r2t.emplace(RISCV_REG_FD26, f64);
		r2t.emplace(RISCV_REG_FD28, f64);
		r2t.emplace(RISCV_REG_FD30, f64);
	}*/

	_reg2type = std::move(r2t);
}

void Capstone2LlvmIrTranslatorRiscv_impl::initializePseudoCallInstructionIDs()
{
	_callInsnIds =
	{
			RISCV_INS_JAL,
			RISCV_INS_JALR,
			//
	};

	_returnInsnIds =
	{
			// Nothing - RISCV returns via jump to return address register.
	};

	_branchInsnIds =
	{

	};

	_condBranchInsnIds =
	{
		    RISCV_INS_BEQ,
			RISCV_INS_BGE,
			RISCV_INS_BGEU,
			RISCV_INS_BLT,
			RISCV_INS_BLTU,
			RISCV_INS_BNE
	};

	_controlFlowInsnIds =
	{
			// Currently, all instructions can be categorized based on their
			// IDs alone.
	};
}

//
//==============================================================================
// Instruction translation map initialization.
//==============================================================================
//

std::map<
	std::size_t,
	void (Capstone2LlvmIrTranslatorRiscv_impl::*)(
			cs_insn* i,
			cs_riscv*,
			llvm::IRBuilder<>&)>
Capstone2LlvmIrTranslatorRiscv_impl::_i2fm =
{
		{MIPS_INS_INVALID, nullptr},

		{RISCV_INS_ADD, &Capstone2LlvmIrTranslatorRiscv_impl::translateAdd},
		{RISCV_INS_ADDI, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  //  TODO
		{RISCV_INS_ADDIW, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop}, // TODO
		{RISCV_INS_ADDW, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

		{RISCV_INS_AND, &Capstone2LlvmIrTranslatorRiscv_impl::translateAnd},
		{RISCV_INS_ANDI, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

		{RISCV_INS_AUIPC, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

		{RISCV_INS_BEQ, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO
		{RISCV_INS_C_BEQZ, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO
		{RISCV_INS_BLT, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO
		{RISCV_INS_BLTU, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

		{RISCV_INS_EBREAK, &Capstone2LlvmIrTranslatorRiscv_impl::translateBreak},

		{RISCV_INS_JALR, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

        {RISCV_INS_LBU, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO
		{RISCV_INS_LD, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

		{RISCV_INS_SRL, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO
		{RISCV_INS_SRLI, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},  // TODO

		// some alias instructions
		{RISCV_INS_C_NOP, &Capstone2LlvmIrTranslatorRiscv_impl::translateNop},

		// special instructions

		{RISCV_INS_ENDING, nullptr},
};

} // namespace capstone2llvmir
} // namespace retdec
