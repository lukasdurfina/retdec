/**
 * @file src/capstone2llvmir/riscv/riscv.cpp
 * @brief RISCV implementation of @c Capstone2LlvmIrTranslator.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#include <iomanip>
#include <iostream>

#include "capstone2llvmir/riscv/riscv_impl.h"

namespace retdec {
namespace capstone2llvmir {

Capstone2LlvmIrTranslatorRiscv_impl::Capstone2LlvmIrTranslatorRiscv_impl(
		llvm::Module* m,
		cs_mode basic,
		cs_mode extra)
		:
		Capstone2LlvmIrTranslator_impl(CS_ARCH_RISCV, basic, extra, m)
{
	// This needs to be called from concrete's class ctor, not abstract's
	// class ctor, so that virtual table is properly initialized.
	initialize();
}

//
//==============================================================================
// Mode query & modification methods - from Capstone2LlvmIrTranslator.
//==============================================================================
//

bool Capstone2LlvmIrTranslatorRiscv_impl::isAllowedBasicMode(cs_mode m)
{
	return m == CS_MODE_RISCV32 || m == CS_MODE_RISCV64;
}

bool Capstone2LlvmIrTranslatorRiscv_impl::isAllowedExtraMode(cs_mode m)
{
	return m == CS_MODE_LITTLE_ENDIAN;
}

uint32_t Capstone2LlvmIrTranslatorRiscv_impl::getArchByteSize()
{
	switch (_origBasicMode)
	{
		case CS_MODE_RISCV32:
			return 4;
		case CS_MODE_RISCV64:
			return 8;
		default:
		{
			throw GenericError("Unhandled mode in getArchByteSize().");
			break;
		}
	}
}

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

void Capstone2LlvmIrTranslatorRiscv_impl::generateEnvironmentArchSpecific()
{
	// Nothing.
}

void Capstone2LlvmIrTranslatorRiscv_impl::generateDataLayout()
{
	switch (_basicMode)
	{
		// TODO
		case CS_MODE_RISCV32:
			_module->setDataLayout("e-p:32:32:32-f80:32:32");
			break;
		case CS_MODE_RISCV64:
			_module->setDataLayout("e-p:64:64:64-i8:8:32-i16:16:32-i64:64-n32:64-S128");
			break;
		default:
		{
			throw GenericError("Unhandled mode in generateDataLayout().");
			break;
		}
	}
}

void Capstone2LlvmIrTranslatorRiscv_impl::generateRegisters()
{
	for (auto& p : _reg2type)
	{
		createRegister(p.first, _regLt);
	}
}

uint32_t Capstone2LlvmIrTranslatorRiscv_impl::getCarryRegister()
{
	return RISCV_REG_INVALID;
}

void Capstone2LlvmIrTranslatorRiscv_impl::translateInstruction(
		cs_insn* i,
		llvm::IRBuilder<>& irb)
{
	_insn = i;

	cs_detail* d = i->detail;
	cs_riscv* mi = &d->riscv;

	std::cout << "Translate instruction at " << std::hex << i->address
	  << ": " << i->mnemonic << "  " << i->op_str << " (" << std::dec << i->size << "b), "
	  << "id: " << i->id << std::endl;

	auto fIt = _i2fm.find(i->id);
	if (fIt != _i2fm.end() && fIt->second != nullptr)
	{
		auto f = fIt->second;
		(this->*f)(i, mi, irb);
	}
	else
	{
		std::cout << "ERROR - unhandled " << i->mnemonic << std::endl;
		throwUnhandledInstructions(i);
		translatePseudoAsmGeneric(i, mi, irb);
	}
}

//
//==============================================================================
// RISCV-specific methods.
//==============================================================================
//

llvm::Value* Capstone2LlvmIrTranslatorRiscv_impl::getCurrentPc(cs_insn* i)
{
	return getNextInsnAddress(i);
}


llvm::Value* Capstone2LlvmIrTranslatorRiscv_impl::loadRegister(
		uint32_t r,
		llvm::IRBuilder<>& irb,
		llvm::Type* dstType,
		eOpConv ct)
{
	if (r == RISCV_REG_INVALID)
	{
		return nullptr;
	}

	if (r == RISCV_REG_X0)
	{
		return llvm::ConstantInt::getSigned(getDefaultType(), 0);
	}

	llvm::Value* llvmReg = getRegister(r);
	if (llvmReg == nullptr)
	{
		throw GenericError("loadRegister() unhandled reg.");
	}

	llvmReg = generateTypeConversion(irb, llvmReg, dstType, ct);

	return irb.CreateLoad(llvmReg);
}

llvm::Value* Capstone2LlvmIrTranslatorRiscv_impl::loadOp(
		cs_riscv_op& op,
		llvm::IRBuilder<>& irb,
		llvm::Type* ty,
		bool lea)
{
	switch (op.type)
	{
		case RISCV_OP_REG:
		{
			return loadRegister(op.reg, irb);
		}
		case RISCV_OP_IMM:
		{
			return llvm::ConstantInt::getSigned(getDefaultType(), op.imm);
		}
		case RISCV_OP_MEM:
		{
			auto* baseR = loadRegister(op.mem.base, irb);
			auto* t = getDefaultType();
			llvm::Value* disp = llvm::ConstantInt::getSigned(t, op.mem.disp);

			llvm::Value* addr = nullptr;
			if (baseR == nullptr)
			{
				addr = disp;
			}
			else
			{
				if (op.mem.disp == 0)
				{
					addr = baseR;
				}
				else
				{
					disp = irb.CreateSExtOrTrunc(disp, baseR->getType());
					addr = irb.CreateAdd(baseR, disp);
				}
			}

			if (lea)
			{
				return addr;
			}
			else
			{
				auto* lty = ty ? ty : t;
				auto* pt = llvm::PointerType::get(lty, 0);
				addr = irb.CreateIntToPtr(addr, pt);
				return irb.CreateLoad(addr);
			}
		}
		case RISCV_OP_INVALID:
		default:
		{
			return llvm::UndefValue::get(ty ? ty : getDefaultType());
		}
	}
}

llvm::StoreInst* Capstone2LlvmIrTranslatorRiscv_impl::storeRegister(
		uint32_t r,
		llvm::Value* val,
		llvm::IRBuilder<>& irb,
		eOpConv ct)
{
	if (r == RISCV_REG_INVALID)
	{
		return nullptr;
	}
	// These registers should not be stored, or their store has no effect.
	//
	if (r == RISCV_REG_ZERO)
	{
		return nullptr;
	}


	auto* llvmReg = getRegister(r);
	if (llvmReg == nullptr)
	{
		throw GenericError("storeRegister() unhandled reg.");
	}
	val = generateTypeConversion(irb, val, llvmReg->getValueType(),  llvmReg->getValueType()->isFloatingPointTy()? eOpConv::FPCAST_OR_BITCAST : ct);

	return irb.CreateStore(val, llvmReg);
}


/**
 * @a ct is used when storing a value to register with a different type.
 * When storing to memory, value type is used -- therefore it needs to be
 * converted to the desired type prior to @c storeOp() call.
 */
llvm::Instruction* Capstone2LlvmIrTranslatorRiscv_impl::storeOp(
		cs_riscv_op& op,
		llvm::Value* val,
		llvm::IRBuilder<>& irb,
		eOpConv ct)
{
	switch (op.type)
	{
		case RISCV_OP_REG:
		{
			return storeRegister(op.reg, val, irb, ct);
		}
		case RISCV_OP_MEM:
		{
			auto* baseR = loadRegister(op.mem.base, irb);
			auto* t = getDefaultType();
			llvm::Value* disp = llvm::ConstantInt::getSigned(t, op.mem.disp);

			llvm::Value* addr = nullptr;
			if (baseR == nullptr)
			{
				addr = disp;
			}
			else
			{
				if (op.mem.disp == 0)
				{
					addr = baseR;
				}
				else
				{
					disp = irb.CreateSExtOrTrunc(disp, baseR->getType());
					addr = irb.CreateAdd(baseR, disp);
				}
			}

			auto* pt = llvm::PointerType::get(val->getType(), 0);
			addr = irb.CreateIntToPtr(addr, pt);
			return irb.CreateStore(val, addr);
		}
		case RISCV_OP_IMM:
		case RISCV_OP_INVALID:
		default:
		{
			throw GenericError("should not be possible");
		}
	}
}


bool Capstone2LlvmIrTranslatorRiscv_impl::isOperandRegister(cs_riscv_op& op)
{
	return op.type == RISCV_OP_REG;
}

bool Capstone2LlvmIrTranslatorRiscv_impl::isGeneralPurposeRegister(uint32_t r)
{
	return RISCV_REG_X0 <= r && r <= RISCV_REG_X31;
}

//
//==============================================================================
// RISCV instruction translation methods.
//==============================================================================
//

/**
 * RISCV_INS_ADD, RISCV_INS_ADDI, RISCV_INS_ADDIW, RISCV_INS_ADDW
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateAdd(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_BINARY_OR_TERNARY(i, mi, irb);

	std::tie(op1, op2) = loadOpBinaryOrTernaryOp1Op2(mi, irb, eOpConv::SEXT_TRUNC_OR_BITCAST, eOpConv::FPCAST_OR_BITCAST);
	auto* add = op1->getType()->isFloatingPointTy()
			? irb.CreateFAdd(op1, op2)
			: irb.CreateAdd(op1, op2);
	storeOp(mi->operands[0], add, irb);
}

/**
 * RISCV_INS_AND, RISCV_INS_ANDI
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateAnd(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_BINARY_OR_TERNARY(i, mi, irb);

	std::tie(op1, op2) = loadOpBinaryOrTernaryOp1Op2(mi, irb, eOpConv::ZEXT_TRUNC_OR_BITCAST);
	auto* a = irb.CreateAnd(op1, op2);
	storeOp(mi->operands[0], a, irb);
}

/**
 * RISCV_INS_AUIPC
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateAuipc(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	// TODO
}

/**
 * RISCV_INS_EBREAK
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateBreak(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	return;
}

/**
 * RISCV_INS_BEQ, RISCV_INS_BNE
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateCondBranch(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
    if (mi->op_count == 2)
	{
		//EXPECT_IS_BINARY(i, mi, irb);
		std::tie(op0, op1) = loadOpBinary(mi, irb);
	    auto* zero = llvm::ConstantInt::get(op0->getType(), 0);

	    llvm::Value* cond = nullptr;
		switch (i->id)
	    {
		    case RISCV_INS_BEQ:    // beqz
		        cond = irb.CreateICmpSGE(op0, zero);
			    break;
			case RISCV_INS_BNE:
			    cond = irb.CreateICmpNE(op0, zero);
			    break;
		    default:
			    throw GenericError("Unhandled insn ID (2 ops) in translateCondBranch().");
	    }

	    generateCondBranchFunctionCall(irb, cond, op1);
	}
	else if (mi->op_count == 3)
	{
		//EXPECT_IS_TERNARY(i, mi, irb);
		std::tie(op0, op1, op2) = loadOpTernary(mi, irb);
		op1 = irb.CreateZExtOrTrunc(op1, op0->getType());

		llvm::Value* cond = nullptr;
		switch (i->id)
		{
			case RISCV_INS_BEQ:
				cond = irb.CreateICmpEQ(op0, op1);
				break;
			case RISCV_INS_BNE:
				cond = irb.CreateICmpNE(op0, op1);
				break;
			default:
				throw GenericError("Unhandled insn ID (3 ops) in translateCondBranch().");
		}

		generateCondBranchFunctionCall(irb, cond, op2);
	}
}

/**
 * RISCV_INS_JAL
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateJal(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_UNARY(i, mi, irb);

    std::cout << "Generating call function at " << std::hex << i->address << std::dec << std::endl;
	storeRegister(RISCV_REG_RA, getNextInsnAddress(i), irb);
	op0 = loadOpUnary(mi, irb);
	generateCallFunctionCall(irb, op0);
}

/**
 * RISCV_INS_JALR
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateJalr(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	if (std::string(i->mnemonic) == "ret") {
	    std::cout << "Generating return function call at " << std::hex << i->address << std::dec << std::endl;
		generateReturnFunctionCall(irb, op0);
		return;
	}
	EXPECT_IS_UNARY(i, mi, irb);

	// handle jr
	// TODO
	// verify if we need: storeRegister(RISCV_REG_RA, getNextInsnAddress(i), irb);
	op0 = loadOpUnary(mi, irb);
	std::cout << "Generating  function call at " << std::hex << i->address << std::dec << std::endl;
	generateCallFunctionCall(irb, op0);
}


/**
 * RISCV_INS_LD, RISCV_INS_LW, RISCV_INS_LH, RISCV_INS_LB, RISCV_INS_FLW, RISCV_INS_FLD
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateLoadMemory(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_BINARY(i, mi, irb);

	llvm::Type* ty = nullptr;
	eOpConv ct = eOpConv::THROW;

	switch (i->id)
	{
		case RISCV_INS_LB: ty = irb.getInt8Ty(); ct = eOpConv::SEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_LBU: ty = irb.getInt8Ty(); ct = eOpConv::ZEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_LH: ty = irb.getInt16Ty(); ct = eOpConv::SEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_LHU: ty = irb.getInt16Ty(); ct = eOpConv::ZEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_LW: ty = irb.getInt32Ty(); ct = eOpConv::SEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_LWU: ty = irb.getInt32Ty(); ct = eOpConv::ZEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_LD: ty = irb.getInt64Ty(); ct = eOpConv::SEXT_TRUNC_OR_BITCAST; break;
		case RISCV_INS_FLW: ty = irb.getFloatTy(); ct = eOpConv::FPCAST_OR_BITCAST; break;
		case RISCV_INS_FLD: ty = irb.getDoubleTy(); ct = eOpConv::FPCAST_OR_BITCAST; break;
		default:
			throw GenericError("Unhandled insn ID in translateLoadMemory().");
	}

	op1 = loadOp(mi->operands[1], irb, ty);
	storeOp(mi->operands[0], op1, irb, ct);
}

/**
 * RISCV_INS_LUI
 * LUI (load upper immediate) is used to build 32-bit constants and uses the U-type format.
 * LUI places the U-immediate value in the top 20 bits of the destination registerrd,
 * filling in the lowest 12 bits with zeros.
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateLui(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_BINARY(i, mi, irb);

	op1 = loadOp(mi->operands[1], irb);
	op1 = irb.CreateZExt(op1, getDefaultType());
	op1 = irb.CreateShl(op1, llvm::ConstantInt::get(op1->getType(), 12));
	storeOp(mi->operands[0], op1, irb);
}

/**
 * RISCV_INS_NOP
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateNop(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	// Nothing.
}

/**
 * RISCV_INS_SD, RISCV_INS_SW, RISCV_INS_SH, RISCV_INS_SB, RISCV_INS_FSW, RISCV_INS_FSD
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateStoreMemory(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_BINARY_OR_TERNARY(i, mi, irb);

	llvm::Type* ty = nullptr;
	switch (i->id)
	{
		case RISCV_INS_SB: ty = irb.getInt8Ty(); break;
		case RISCV_INS_SH: ty = irb.getInt16Ty(); break;
		case RISCV_INS_SW: ty = irb.getInt32Ty(); break;
		case RISCV_INS_SD: ty = irb.getInt64Ty(); break;
		case RISCV_INS_FSW: ty = irb.getFloatTy(); break;
		case RISCV_INS_FSD: ty = irb.getDoubleTy(); break;
		default:
		{
			throw GenericError("RISCV: unhandled store");
		}
	}

	op0 = loadOp(mi->operands[0], irb);
	if (ty->isFloatingPointTy())
	{
		// TODO verify
		// This is not exact, in 64-bit mode, only lower 32-bits of FPR should
		// be used -> truncate, not cast.
		op0 = irb.CreateFPCast(op0, ty);
	}
	else if (ty->isIntegerTy())
	{
		op0 = irb.CreateZExtOrTrunc(op0, ty);
	}
	else
	{
		throw GenericError("unhandled type");
	}
	storeOp(mi->operands[1], op0, irb);
}

/**
 * RISCV_INS_ECALL
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateSyscall(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_NULLARY_OR_UNARY(i, mi, irb);

	if (mi->op_count == 0)
	{
		op0 = llvm::ConstantInt::get(getDefaultType(), 0);
	}
	else if (mi->op_count == 1)
	{
		op0 = loadOpUnary(mi, irb);
	}

	op0 = irb.CreateZExtOrTrunc(op0, getDefaultType());

	llvm::Function* fnc = getPseudoAsmFunction(
			i,
			irb.getVoidTy(),
			llvm::ArrayRef<llvm::Type*>{op0->getType()});

	irb.CreateCall(fnc, {op0});
}


} // namespace capstone2llvmir
} // namespace retdec
