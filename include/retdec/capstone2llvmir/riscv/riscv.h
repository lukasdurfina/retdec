/**
 * @file include/retdec/capstone2llvmir/riscv/riscv.h
 * @brief RISCV specialization of translator's abstract public interface.
 */

#ifndef RETDEC_CAPSTONE2LLVMIR_RISCV_RISCV_H
#define RETDEC_CAPSTONE2LLVMIR_RISCV_RISCV_H

#include "retdec/capstone2llvmir/capstone2llvmir.h"
#include "retdec/capstone2llvmir/riscv/riscv_defs.h"

namespace retdec {
namespace capstone2llvmir {

/**
 * RISCV specialization of translator's abstract public interface.
 */
class Capstone2LlvmIrTranslatorRiscv : virtual public Capstone2LlvmIrTranslator
{

};

} // namespace capstone2llvmir
} // namespace retdec

#endif
