/**
 * @file src/bin2llvmir/providers/calling_convention/riscv/riscv_conv.cpp
 * @brief Calling conventions of RISCV architecture.
 */

#include "retdec/bin2llvmir/providers/abi/abi.h"
#include "retdec/bin2llvmir/providers/calling_convention/riscv/riscv_conv.h"
#include "retdec/capstone2llvmir/riscv/riscv.h"

namespace retdec {
namespace bin2llvmir {

RiscvCallingConvention::RiscvCallingConvention(const Abi* a) :
	CallingConvention(a)
{
	_paramRegs = {
		RISCV_REG_A0,
		RISCV_REG_A1,
		RISCV_REG_A2,
		RISCV_REG_A3,
		RISCV_REG_A4,
		RISCV_REG_A5,
		RISCV_REG_A6,
		RISCV_REG_A7,
	};
	_paramFPRegs = {
		RISCV_REG_F10_32,
		RISCV_REG_F11_32,
		RISCV_REG_F12_32,
		RISCV_REG_F13_32
	};
	_paramDoubleRegs = {
		RISCV_REG_F10_64,
		RISCV_REG_F11_64,
		RISCV_REG_F12_64,
		RISCV_REG_F13_64,
	};

	_returnRegs = {
		RISCV_REG_A0
	};

	_numOfRegsPerParam = 2;
	_largeObjectsPassedByReference = true;
	_respectsRegCouples = true;
}

CallingConvention::Ptr RiscvCallingConvention::create(const Abi* a)
{
	if (!a->isRiscv())
	{
		return nullptr;
	}

	return std::make_unique<RiscvCallingConvention>(a);
}

}
}
