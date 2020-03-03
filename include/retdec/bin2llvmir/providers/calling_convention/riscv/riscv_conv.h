/**
 * @file include/retdec/bin2llvmir/providers/calling_convention/riscv/riscv_conv.h
 * @brief Calling convention of Riscv architecture.
 */

#ifndef RETDEC_BIN2LLVMIR_PROVIDERS_CALL_CONV_RISCV_CONV_H
#define RETDEC_BIN2LLVMIR_PROVIDERS_CALL_CONV_RISCV_CONV_H

#include "retdec/bin2llvmir/providers/calling_convention/calling_convention.h"

namespace retdec {
namespace bin2llvmir {

class RiscvCallingConvention: public CallingConvention
{
	// Ctors, dtors.
	//
	public:
		RiscvCallingConvention(const Abi* a);

	// Construcor method.
	//
	public:
		static CallingConvention::Ptr create(const Abi* a);

};

}
}

#endif
