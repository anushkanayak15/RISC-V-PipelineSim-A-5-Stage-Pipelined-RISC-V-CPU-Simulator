# RISC-V PipelineSim: A 5-Stage Pipelined RISC-V CPU Simulator

## Overview  
RISC-V PipelineSim is a C++-based simulator for a **5-stage pipelined RISC-V processor**, implementing the **RV32-I base instruction set**. This project simulates a functional **instruction pipeline**, showcasing how modern RISC-V processors execute instructions efficiently.

This project was developed as part of **M151M Project #1** and includes implementations for **instruction decode, execution, and pipeline timing simulation**.

## Features  
- Implements a **5-stage pipelined RISC-V CPU**:
  1. Fetch  
  2. Decode  
  3. Execute  
  4. Memory Access  
  5. Write-back  
- Supports **RV32-I ISA** (Integer Instructions)  
- Implements **hazard handling techniques** (data forwarding, stall handling)  
- Includes **precompiled test cases** for verification  
- Debugging support with multiple verbosity levels  

---

## Implementation Details  

### Part 1: Decode  
- Implements **instruction decoding logic** in `decode.cpp`.  
- Extracts opcode, funct3, funct7, and operand registers from instructions.  

### Part 2: Execute  
- Implements **ALU operations** in `execute.cpp`.  
- Handles different instruction types (R, I, S, B, U, J formats).  

### Part 3: Timing Simulation  
- Implements **pipeline logic** in `core.cpp`.  
- Manages instruction flow through the pipeline, handling hazards and forwarding.  

---

## Building and Running the Simulator  

### Prerequisites  
- **Linux development environment** (Recommended: Ubuntu 18.04 or later)  
- **C++11 or later**  

### Build Instructions  
```bash
# Clone the repository
git clone https://github.com/your-username/riscv-pipelinesim.git
cd riscv-pipelinesim

# Build the simulator
make

# Run all test cases
make tests  

# Run a specific test case
./tinyrv -s tests/rv32ui-p-sub.hex

# Enable debug mode (Level 3)
DEBUG=3 make  
./tinyrv -s tests/rv32ui-p-sub.hex

make submit  # Generates submission.zip
```
