/**
 * @file src/bin2llvmir/providers/abi/riscv.cpp
 * @brief ABI information for RISCV.
 */

#include "retdec/bin2llvmir/providers/abi/riscv.h"

using namespace llvm;

namespace retdec {
namespace bin2llvmir {

AbiRiscv::AbiRiscv(llvm::Module* m, Config* c) :
		Abi(m, c)
{
	_regs.reserve(RISCV_REG_ENDING);
	_id2regs.resize(RISCV_REG_ENDING, nullptr);
	_regStackPointerId = RISCV_REG_SP;
	_regZeroReg = RISCV_REG_X0;

	// system calls
	_regSyscallId = RISCV_REG_X7;
	_regSyscallReturn = RISCV_REG_A0;
	_syscallRegs = {
			RISCV_REG_A0,
			RISCV_REG_A1,
			RISCV_REG_A2,
			RISCV_REG_A3};

	_defcc = CallingConvention::ID::CC_RISCV;
}

bool AbiRiscv::isGeneralPurposeRegister(const llvm::Value* val) const
{
	uint32_t rid = getRegisterId(val);
	return RISCV_REG_X0 <= rid && rid <= RISCV_REG_X31;
}

bool AbiRiscv::isNopInstruction(cs_insn* insn)
{
	// True NOP variants.
	//
	if (insn->id == RISCV_INS_ADDI && insn->mnemonic == "nop")
	{
		return true;
	}

	return false;
}

} // namespace bin2llvmir
} // namespace retdec
