/**
 * @file src/capstone2llvmir/riscv/riscv_impl.h
 * @brief RISCV implementation of @c Capstone2LlvmIrTranslator.
 */

#ifndef CAPSTONE2LLVMIR_RISCV_RISCV_IMPL_H
#define CAPSTONE2LLVMIR_RISCV_RISCV_IMPL_H

#include "retdec/capstone2llvmir/riscv/riscv.h"
#include "capstone2llvmir/capstone2llvmir_impl.h"

namespace retdec {
namespace capstone2llvmir {

class Capstone2LlvmIrTranslatorRiscv_impl :
		public Capstone2LlvmIrTranslator_impl<cs_riscv, cs_riscv_op>,
		public Capstone2LlvmIrTranslatorRiscv
{
	public:
		Capstone2LlvmIrTranslatorRiscv_impl(
				llvm::Module* m,
				cs_mode basic = CS_MODE_RISCV32,
				cs_mode extra = CS_MODE_LITTLE_ENDIAN);
//
//==============================================================================
// Mode query & modification methods - from Capstone2LlvmIrTranslator.
//==============================================================================
//
	public:
		virtual bool isAllowedBasicMode(cs_mode m) override;
		virtual bool isAllowedExtraMode(cs_mode m) override;
		virtual uint32_t getArchByteSize() override;
//
//==============================================================================
// Capstone related getters - from Capstone2LlvmIrTranslator.
//==============================================================================
//

//
//==============================================================================
// Pure virtual methods from Capstone2LlvmIrTranslator_impl
//==============================================================================
//
	protected:
		virtual void initializeArchSpecific() override;
		virtual void initializeRegNameMap() override;
		virtual void initializeRegTypeMap() override;
		virtual void initializePseudoCallInstructionIDs() override;
		virtual void generateEnvironmentArchSpecific() override;
		virtual void generateDataLayout() override;
		virtual void generateRegisters() override;
		virtual uint32_t getCarryRegister() override;

		virtual void translateInstruction(
				cs_insn* i,
				llvm::IRBuilder<>& irb) override;
//
//==============================================================================
// MIPS-specific methods.
//==============================================================================
//
	protected:
		llvm::Value* getCurrentPc(cs_insn* i);
		llvm::Value* getNextNextInsnAddress(cs_insn* i);
		llvm::Value* getUnpredictableValue();

		//uint32_t singlePrecisionToDoublePrecisionFpRegister(uint32_t r) const;

		virtual llvm::Value* loadRegister(
				uint32_t r,
				llvm::IRBuilder<>& irb,
				llvm::Type* dstType = nullptr,
				eOpConv ct = eOpConv::THROW) override;
		virtual llvm::Value* loadOp(
				cs_riscv_op& op,
				llvm::IRBuilder<>& irb,
				llvm::Type* ty = nullptr,
				bool lea = false) override;

		virtual llvm::StoreInst* storeRegister(
				uint32_t r,
				llvm::Value* val,
				llvm::IRBuilder<>& irb,
				eOpConv ct = eOpConv::SEXT_TRUNC_OR_BITCAST) override;
		virtual llvm::Instruction* storeOp(
				cs_riscv_op& op,
				llvm::Value* val,
				llvm::IRBuilder<>& irb,
				eOpConv ct = eOpConv::SEXT_TRUNC_OR_BITCAST) override;

		llvm::StoreInst* storeRegisterUnpredictable(
				uint32_t r,
				llvm::IRBuilder<>& irb);
		bool isFpInstructionVariant(cs_insn* i);

		virtual bool isOperandRegister(cs_riscv_op& op) override;
		bool isGeneralPurposeRegister(uint32_t r);
//
//==============================================================================
// RISCV implementation data.
//==============================================================================
//
	protected:
		static std::map<
			std::size_t,
			void (Capstone2LlvmIrTranslatorRiscv_impl::*)(
					cs_insn* i,
					cs_riscv*,
					llvm::IRBuilder<>&)> _i2fm;
//
//==============================================================================
// RISCV instruction translation methods.
//==============================================================================
//
	protected:
		void translateAdd(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb);
		void translateAnd(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb);

		void translateBreak(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb);

		void translateNop(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb);

		void translateSyscall(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb);

};

} // namespace capstone2llvmir
} // namespace retdec

#endif
