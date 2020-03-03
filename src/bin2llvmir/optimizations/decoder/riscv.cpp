/**
* @file src/bin2llvmir/optimizations/decoder/riscv.cpp
* @brief Decoding methods specific to RISCV architecture.
*/

#include "retdec/bin2llvmir/optimizations/decoder/decoder.h"
#include "retdec/bin2llvmir/utils/capstone.h"
#include "retdec/utils/string.h"

using namespace llvm;
using namespace retdec::bin2llvmir::st_match;
using namespace retdec::capstone2llvmir;
using namespace retdec::common;

namespace retdec {
namespace bin2llvmir {


bool Decoder::disasm_riscv(
		csh ce,
		cs_mode m,
		ByteData& bytes,
		uint64_t& a,
		cs_insn* i)
{
	bool ret = cs_disasm_iter(ce, &bytes.first, &bytes.second, &a, i);

	if (ret == false && (m & CS_MODE_RISCV32))
	{
		_c2l->modifyBasicMode(CS_MODE_RISCV64);
		ret = cs_disasm_iter(ce, &bytes.first, &bytes.second, &a, i);
		_c2l->modifyBasicMode(CS_MODE_RISCV32);
	}

	return ret;
}

std::size_t Decoder::decodeJumpTargetDryRun_riscv(
		const JumpTarget& jt,
		ByteData bytes,
		bool strict)
{
	if (strict)
	{
		return true;
	}

	static csh ce = _c2l->getCapstoneEngine();

	uint64_t addr = jt.getAddress();
	std::size_t nops = 0;
	bool first = true;
	unsigned counter = 0;
	unsigned cfChangePos = 0;
	while (disasm_riscv(ce, _c2l->getBasicMode(), bytes, addr, _dryCsInsn))
	{
		++counter;

		if (jt.getType() == JumpTarget::eType::LEFTOVER
				&& (first || nops > 0)
				&& _abi->isNopInstruction(_dryCsInsn))
		{
			nops += _dryCsInsn->size;
		}
		else if (jt.getType() == JumpTarget::eType::LEFTOVER
				&& nops > 0)
		{
			return nops;
		}

		if (_c2l->isReturnInstruction(*_dryCsInsn))
		{
			return false;
		}
		if (_c2l->isBranchInstruction(*_dryCsInsn))
		{
			return false;
		}

		if (_c2l->isReturnInstruction(*_dryCsInsn)
				|| _c2l->isBranchInstruction(*_dryCsInsn)
				|| _c2l->isCondBranchInstruction(*_dryCsInsn)
				|| _c2l->isCallInstruction(*_dryCsInsn))
		{
			cfChangePos = counter;
		}

		first = false;
	}

	if (nops > 0)
	{
		return nops;
	}

	// There is a BB right after, that is not a function start.
	//
	if (getBasicBlockAtAddress(addr) && getFunctionAtAddress(addr) == nullptr)
	{
		return false;
	}

	return true;
}


} // namespace bin2llvmir
} // namespace retdec
