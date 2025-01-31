// Copyright 2025 Blaise Tine
//
// Licensed under the Apache License;
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <bitset>
#include <climits>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <util.h>
#include "core.h"

//responsible for executing instructions decoded in the previous stage
// Performing arithmetic/logical operations (ALU)
//Handling branch logic (Branch Unit)
//Interacting with memory (Memory Access)
using namespace tinyrv;

uint32_t Core::alu_unit(const Instr &instr, uint32_t rs1_data, uint32_t rs2_data, uint32_t PC) {
  //it will performs computations based on the instruction type
  auto exe_flags  = instr.getExeFlags();
  auto alu_op     = instr.getAluOp();

  uint32_t alu_s1 = exe_flags.alu_s1_PC ? PC : (exe_flags.alu_s1_rs1 ? instr.getRs1() :  rs1_data);
  uint32_t alu_s2 = exe_flags.alu_s2_imm ? instr.getImm() : rs2_data;

  if (exe_flags.alu_s1_inv) {
    alu_s1 = ~alu_s1; // Invert first operand if needed
  }

  uint32_t rd_data = 0;

  switch (alu_op) {
  case AluOp::NONE:
    break;
  case AluOp::ADD: {
        // TODO:
    rd_data = alu_s1 + alu_s2;
  
    break;
  }
  case AluOp::SUB: {
    // TODO:
    rd_data = alu_s1 - alu_s2;
    break;
  }
  case AluOp::AND: {
    // TODO:
    rd_data = alu_s1 & alu_s2;
    break;
  }
  case AluOp::OR: {
    // TODO:
    rd_data = alu_s1 | alu_s2;
    break;
  }
  case AluOp::XOR: {
    // TODO:
    rd_data = alu_s1 ^ alu_s2;
    break;
  }
  case AluOp::SLL: {
    // TODOshift left logical:
    rd_data = alu_s1 << (alu_s2 & 0x1F);
    break;
  }
  case AluOp::SRL: {
    // TODO shift right logical:
    rd_data = alu_s1 >> (alu_s2 & 0x1F);
    break;
  }
  case AluOp::SRA: {
    // TODO shift right arithmetic shift:
    rd_data = (int32_t)alu_s1 >> (alu_s2 & 0x1F); // Arithmetic shift
    break;
  }
  case AluOp::LTI: {
    // TODO less than  signed:
    rd_data = (int32_t)alu_s1 < (int32_t)alu_s2;
    break;
  }
  case AluOp::LTU: {
    // TODO less than unsigned:
    rd_data = alu_s1 < alu_s2;
    break;
  }
  default:
    std::abort();
  }

  return rd_data;
}

uint32_t Core::branch_unit(const Instr &instr, uint32_t rs1_data, uint32_t rs2_data, uint32_t rd_data, uint32_t PC) {
  //The Branch Unit evaluates branch conditions and determines whether the branch should be taken
  auto br_op = instr.getBrOp();

  bool br_taken = false;

  switch (br_op) {
  case BrOp::NONE:
    break;
  case BrOp::JAL: //?? Always takes?
  case BrOp::JALR: { //?? Always taken?
  // TODO:
    br_taken = true;
    break;
  }
  case BrOp::BEQ: {
    // TODO when they are equal:
    br_taken = (rs1_data == rs2_data);
    break;
  }
  case BrOp::BNE: {
    // TODO when they are not equal:
    br_taken = (rs1_data != rs2_data);
    break;
  }
  case BrOp::BLT: {
    // TODO less than signed:
    br_taken = (int32_t)rs1_data < (int32_t)rs2_data;
    break;
  }
  case BrOp::BGE: {
    // TODO greater than or equal to :
    br_taken = (int32_t)rs1_data >= (int32_t)rs2_data;
    break;
  }
  case BrOp::BLTU: {
    // TODO less than unsigned:
    br_taken = rs1_data < rs2_data;
    break;
  }
  case BrOp::BGEU: {
    // TODO greater than or equal to unsigned:
    br_taken = rs1_data >= rs2_data;
    break;
  }
  default:
    std::abort();
  }

  // resolve branches checks if a branch intruction is being processed
  if (br_op != BrOp::NONE) { // if instr is not a branch it does nothing
    auto br_target = rd_data; // branch target address is stored in rd_data, If the branch is taken, the PC will be updated to br_target
    if (br_taken) { // if branch condiiton is met
      uint32_t next_PC = PC + 4;
      if (br_op == BrOp::JAL || br_op == BrOp::JALR) {
        // TODO: Set rd_data to the next instruction address (PC + 4)
        rd_data = PC + 4; // increment the program counter by 4- the return address
      }
      // check misprediction 
      //br_target is not equal to the next sequential PC (PC + 4), then the CPU mispredicted the branch
      //so we have to set pc to the corrrect branch target
      if (br_op != BrOp::JAL && br_target != next_PC) {
         // TODO:
        PC_ = br_target;
        // flush pipeline to avoid executing incorrect instructions since the CPU fetched the wrong instructions it needs to be clear
        if_id_.reset(); //removes incorrrect instruciton waiting in the pipeline
        fetch_stalled_ = false; // new instruction fetched from the correct pc
        DT(2, "*** Branch misprediction: (#" << id_ex_.data().uuid << ")");
      }
    }
    DT(2, "Branch: " << (br_taken ? "taken" : "not-taken") << ", target=0x" << std::hex << br_target << std::dec << " (#" << id_ex_.data().uuid << ")");
  }

  return rd_data;
}

uint32_t Core::mem_access(const Instr &instr, uint32_t rd_data, uint32_t rs2_data) {
  // the mem access unit handles loads stores and control status register operations
  auto exe_flags = instr.getExeFlags(); //extract the execution falg which stores the instrucion properties like load/store/csr
  auto func3     = instr.getFunc3(); //Determines load/store data size and sign extension

  // handle loads
  if (exe_flags.is_load) {
    uint64_t mem_addr = rd_data; //The address where data should be read from
    uint32_t data_bytes = 1 << (func3 & 0x3);// how many bytes to read
    uint32_t data_width = 8 * data_bytes; //bit-width of the value 
    uint32_t read_data = 0; //Stores the retrieved memory valu
    this->dmem_read(&read_data, mem_addr, data_bytes);
    switch (func3) {
    case 0: // RV32I: LB (Load Byte - Signed)
    case 1: // RV32I: LH (Load Halfword - Signed)
      rd_data = sext(read_data, data_width); // Sign-extend the read value to preserve negative val
      break;
    case 2: // RV32I: LW (Load Word) There is no need to sign extend here
      rd_data = read_data;// TODO:  this directly assign the value
      break;
    case 4: // RV32I: LBU (Load Byte - Unsigned)
    case 5: // RV32I: LHU (Load Halfword - Unsigned)
    //Zero-extend instead of sign-extending to prevent unintended negative values??
      rd_data = read_data & ((1 << data_width) - 1); // Zero-extend the value NOT SURE OF THIS??// TODO:
      break;
    default:
      std::abort();
    }
  }

  // handle stores
  if (exe_flags.is_store) {
    uint64_t mem_addr = rd_data;
    uint32_t data_bytes = 1 << (func3 & 0x3);
    switch (func3) {
    case 0:
    case 1:
    case 2:
      this->dmem_write(&rs2_data, mem_addr, data_bytes);
      break;
    default:
      std::abort();
    }
  }

  // handle CSR write
  if (exe_flags.is_csr) {
    if (rs2_data != rd_data) {
      this->set_csr(instr.getImm(), rd_data);
    }
    rd_data = rs2_data;
  }

  return rd_data;
}

void Core::dmem_read(void *data, uint64_t addr, uint32_t size) {
  auto type = get_addr_type(addr);
  __unused (type);
  mmu_.read(data, addr, size, 0);
  DTH(2, "Mem Read: addr=0x" << std::hex << addr << ", data=0x" << ByteStream(data, size) << " (size=" << size << ", type=" << type << ")");
}

void Core::dmem_write(const void* data, uint64_t addr, uint32_t size) {
  auto type = get_addr_type(addr);
  __unused (type);
  if (addr >= uint64_t(IO_COUT_ADDR)
   && addr < (uint64_t(IO_COUT_ADDR) + IO_COUT_SIZE)) {
     this->writeToStdOut(data);
  } else {
    mmu_.write(data, addr, size, 0);
  }
  DTH(2, "Mem Write: addr=0x" << std::hex << addr << ", data=0x" << ByteStream(data, size) << " (size=" << size << ", type=" << type << ")");
}

uint32_t Core::get_csr(uint32_t addr) {
  switch (addr) {
  case VX_CSR_MHARTID:
  case VX_CSR_SATP:
  case VX_CSR_PMPCFG0:
  case VX_CSR_PMPADDR0:
  case VX_CSR_MSTATUS:
  case VX_CSR_MISA:
  case VX_CSR_MEDELEG:
  case VX_CSR_MIDELEG:
  case VX_CSR_MIE:
  case VX_CSR_MTVEC:
  case VX_CSR_MEPC:
  case VX_CSR_MNSTATUS:
    return 0;
  case VX_CSR_MCYCLE: // NumCycles
    return perf_stats_.cycles & 0xffffffff;
  case VX_CSR_MCYCLE_H: // NumCycles
    return (uint32_t)(perf_stats_.cycles >> 32);
  case VX_CSR_MINSTRET: // NumInsts
    return perf_stats_.instrs & 0xffffffff;
  case VX_CSR_MINSTRET_H: // NumInsts
    return (uint32_t)(perf_stats_.instrs >> 32);
  default:
    std::cout << std::hex << "Error: invalid CSR read addr=0x" << addr << std::endl;
    std::abort();
    return 0;
  }
}

void Core::set_csr(uint32_t addr, uint32_t value) {
  switch (addr) {
  case VX_CSR_SATP:
  case VX_CSR_MSTATUS:
  case VX_CSR_MEDELEG:
  case VX_CSR_MIDELEG:
  case VX_CSR_MIE:
  case VX_CSR_MTVEC:
  case VX_CSR_MEPC:
  case VX_CSR_PMPCFG0:
  case VX_CSR_PMPADDR0:
  case VX_CSR_MNSTATUS:
    break;
  default: {
      std::cout << std::hex << "Error: invalid CSR write addr=0x" << addr << ", value=0x" << value << std::endl;
      std::abort();
    }
  }
}
