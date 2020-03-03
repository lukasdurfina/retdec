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

	std::cout << "Translate instruction at" << std::hex << i->address
	  << ": " << i->mnemonic << "  " << i->op_str << " (" << std::dec << i->size << "b)"<< std::endl;

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

/**
 * MIPS specifications often says something like:
 * "The return link is the address of the second instruction following the,
 * branch, at which location execution continues after a procedure call."
 * This method returns this address as an LLVM @c ConstantInt.
 */
// TODO is it  relevant for riscv
llvm::Value* Capstone2LlvmIrTranslatorRiscv_impl::getNextNextInsnAddress(cs_insn* i)
{
	return llvm::ConstantInt::get(getDefaultType(), i->address + (2 * i->size));
}

/**
 * @return @c Nullptr -- there is no value.
 *
 * @c Nullptr will cause all the consumers like @c storeRegisterUnpredictable()
 * not to generate any code that depends on unpredictable value.
 *
 * MIPS specifications says:
 * "... Software can never depend on results that are UNPREDICTABLE.
 * UNPREDICTABLE operations may cause a result to be generated or not. ..."
 *
 * Right now, we choose not to generate it. This may change in future.
 */
llvm::Value* Capstone2LlvmIrTranslatorRiscv_impl::getUnpredictableValue()
{
	return nullptr;
}

//uint32_t Capstone2LlvmIrTranslatorRiscv_impl::singlePrecisionToDoublePrecisionFpRegister(
//		uint32_t r) const
//{
	// Working with odd double reg (e.g. sdc1 $f21, -0x7ba3($v1)) may happen.
	// I have no idea why, and if this is ok, or it is simply caused by decoding
	// data. But it is a real example from real binary, IDA has the same thing.
	// Right now, we map odd numbers to even ones. But we would be able to
	// create their own double registers very easily.
	/*switch (r)
	{
		case MIPS_REG_F0: return MIPS_REG_FD0;
		case MIPS_REG_F1: return MIPS_REG_FD0;
		case MIPS_REG_F2: return MIPS_REG_FD2;
		case MIPS_REG_F3: return MIPS_REG_FD2;
		case MIPS_REG_F4: return MIPS_REG_FD4;
		case MIPS_REG_F5: return MIPS_REG_FD4;
		case MIPS_REG_F6: return MIPS_REG_FD6;
		case MIPS_REG_F7: return MIPS_REG_FD6;
		case MIPS_REG_F8: return MIPS_REG_FD8;
		case MIPS_REG_F9: return MIPS_REG_FD8;
		case MIPS_REG_F10: return MIPS_REG_FD10;
		case MIPS_REG_F11: return MIPS_REG_FD10;
		case MIPS_REG_F12: return MIPS_REG_FD12;
		case MIPS_REG_F13: return MIPS_REG_FD12;
		case MIPS_REG_F14: return MIPS_REG_FD14;
		case MIPS_REG_F15: return MIPS_REG_FD14;
		case MIPS_REG_F16: return MIPS_REG_FD16;
		case MIPS_REG_F17: return MIPS_REG_FD16;
		case MIPS_REG_F18: return MIPS_REG_FD18;
		case MIPS_REG_F19: return MIPS_REG_FD18;
		case MIPS_REG_F20: return MIPS_REG_FD20;
		case MIPS_REG_F21: return MIPS_REG_FD20;
		case MIPS_REG_F22: return MIPS_REG_FD22;
		case MIPS_REG_F23: return MIPS_REG_FD22;
		case MIPS_REG_F24: return MIPS_REG_FD24;
		case MIPS_REG_F25: return MIPS_REG_FD24;
		case MIPS_REG_F26: return MIPS_REG_FD26;
		case MIPS_REG_F27: return MIPS_REG_FD26;
		case MIPS_REG_F28: return MIPS_REG_FD28;
		case MIPS_REG_F29: return MIPS_REG_FD28;
		case MIPS_REG_F30: return MIPS_REG_FD30;
		case MIPS_REG_F31: return MIPS_REG_FD30;
		default:
			throw GenericError("Can not convert to double precision "
					"register.");
	}*/
//}

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

	/*if (cs_insn_group(_handle, _insn, MIPS_GRP_NOTFP64BIT)
			&& MIPS_REG_F0 <= r
			&& r <= MIPS_REG_F31)
	{
		r = singlePrecisionToDoublePrecisionFpRegister(r);
	}*/

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

	/*if (cs_insn_group(_handle, _insn, MIPS_GRP_NOTFP64BIT)
			&& MIPS_REG_F0 <= r
			&& r <= MIPS_REG_F31)
	{
		r = singlePrecisionToDoublePrecisionFpRegister(r);
	}*/

	auto* llvmReg = getRegister(r);
	if (llvmReg == nullptr)
	{
		throw GenericError("storeRegister() unhandled reg.");
	}
	val = generateTypeConversion(irb, val, llvmReg->getValueType(),  llvmReg->getValueType()->isFloatingPointTy()? eOpConv::FPCAST_OR_BITCAST : ct);

	return irb.CreateStore(val, llvmReg);
}

/**
 * Store unpredictable value to register @a r.
 * No store is generated if unpredictable value is set to @c nullptr (see
 * @c getUnpredictableValue()).
 */
llvm::StoreInst* Capstone2LlvmIrTranslatorRiscv_impl::storeRegisterUnpredictable(
		uint32_t r,
		llvm::IRBuilder<>& irb)
{
	auto* u = getUnpredictableValue();
	return u ? storeRegister(r, u, irb) : nullptr;
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
/*
bool Capstone2LlvmIrTranslatorRiscv_impl::isFpInstructionVariant(cs_insn* i)
{
	auto& mi = i->detail->mips;
	return mi.op_count > 0
			&& mi.operands[0].type == MIPS_OP_REG
			&& MIPS_REG_F0 <= mi.operands[0].reg
			&& mi.operands[0].reg <= MIPS_REG_F31;
}*/

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
 *
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
 *
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateAnd(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	EXPECT_IS_BINARY_OR_TERNARY(i, mi, irb);

	std::tie(op1, op2) = loadOpBinaryOrTernaryOp1Op2(mi, irb, eOpConv::ZEXT_TRUNC_OR_BITCAST);
	auto* a = irb.CreateAnd(op1, op2);
	storeOp(mi->operands[0], a, irb);
}


/**
 *
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateBreak(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	return;
}

/**
 *
 */
void Capstone2LlvmIrTranslatorRiscv_impl::translateNop(cs_insn* i, cs_riscv* mi, llvm::IRBuilder<>& irb)
{
	// Nothing.
}


/**
 *
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
