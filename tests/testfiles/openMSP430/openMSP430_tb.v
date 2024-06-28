//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: io_cell.v
//
// *Module Description:
//                       I/O cell model
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 103 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2011-03-05 15:44:48 +0100 (Sat, 05 Mar 2011) $
//----------------------------------------------------------------------------

module io_cell (

// INOUTs
    pad,                       // I/O Pad

// OUTPUTs
    data_in,                   // Input value

// INPUTs
    data_out_en,               // Output enable
    data_out                   // Output value
);

// INOUTs
//=========
inout          pad;            // I/O Pad

// OUTPUTs
//=========
output         data_in;        // Input value

// INPUTs
//=========
input          data_out_en;    // Output enable
input          data_out;       // Output value


//=============================================================================
// 1)  I/O CELL
//=============================================================================

assign  data_in  =  pad;
assign  pad      =  data_out_en ? data_out : 1'bz;


endmodule // io_cell


//----------------------------------------------------------------------------
// Copyright (C) 2001 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: tb_openMSP430.v
//
// *Module Description:
//                      openMSP430 testbench
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 205 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2015-07-15 22:59:52 +0200 (Wed, 15 Jul 2015) $
//----------------------------------------------------------------------------
`timescale 1ns / 100ps
`ifdef OMSP_NO_INCLUDE
`else
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: openMSP430_defines.v
//
// *Module Description:
//                      openMSP430 Configuration file
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 205 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2015-07-15 22:59:52 +0200 (Wed, 15 Jul 2015) $
//----------------------------------------------------------------------------
`define OMSP_NO_INCLUDE
`ifdef OMSP_NO_INCLUDE
`else
`include "openMSP430_undefines.v"
`endif

//============================================================================
//============================================================================
// BASIC SYSTEM CONFIGURATION
//============================================================================
//============================================================================
//
// Note: the sum of program, data and peripheral memory spaces must not
//      exceed 64 kB
//

// Program Memory Size:
//                     Uncomment the required memory size
//-------------------------------------------------------
//`define PMEM_SIZE_CUSTOM
//`define PMEM_SIZE_59_KB
//`define PMEM_SIZE_55_KB
//`define PMEM_SIZE_54_KB
//`define PMEM_SIZE_51_KB
//`define PMEM_SIZE_48_KB
//`define PMEM_SIZE_41_KB
//`define PMEM_SIZE_32_KB
//`define PMEM_SIZE_24_KB
//`define PMEM_SIZE_16_KB
//`define PMEM_SIZE_12_KB
//`define PMEM_SIZE_8_KB
`define PMEM_SIZE_4_KB
//`define PMEM_SIZE_2_KB
//`define PMEM_SIZE_1_KB


// Data Memory Size:
//                     Uncomment the required memory size
//-------------------------------------------------------
//`define DMEM_SIZE_CUSTOM
//`define DMEM_SIZE_32_KB
//`define DMEM_SIZE_24_KB
//`define DMEM_SIZE_16_KB
//`define DMEM_SIZE_10_KB
//`define DMEM_SIZE_8_KB
//`define DMEM_SIZE_5_KB
//`define DMEM_SIZE_4_KB
//`define DMEM_SIZE_2p5_KB
//`define DMEM_SIZE_2_KB
`define DMEM_SIZE_1_KB
//`define DMEM_SIZE_512_B
//`define DMEM_SIZE_256_B
//`define DMEM_SIZE_128_B


// Include/Exclude Hardware Multiplier
`define MULTIPLIER


// Include/Exclude Serial Debug interface
`define DBG_EN


//============================================================================
//============================================================================
// ADVANCED SYSTEM CONFIGURATION (FOR EXPERIENCED USERS)
//============================================================================
//============================================================================

//-------------------------------------------------------
// Custom user version number
//-------------------------------------------------------
// This 5 bit field can be freely used in order to allow
// custom identification of the system through the debug
// interface.
// (see CPU_ID.USER_VERSION field in the documentation)
//-------------------------------------------------------
`define USER_VERSION 5'b00000


//-------------------------------------------------------
// Include/Exclude Watchdog timer
//-------------------------------------------------------
// When excluded, the following functionality will be
// lost:
//        - Watchog (both interval and watchdog modes)
//        - NMI interrupt edge selection
//        - Possibility to generate a software PUC reset
//-------------------------------------------------------
`define WATCHDOG


//-------------------------------------------------------
// Include/Exclude DMA interface support
//-------------------------------------------------------
`define DMA_IF_EN


//-------------------------------------------------------
// Include/Exclude Non-Maskable-Interrupt support
//-------------------------------------------------------
`define NMI


//-------------------------------------------------------
// Number of available IRQs
//-------------------------------------------------------
// Indicates the number of interrupt vectors supported
// (16, 32 or 64).
//-------------------------------------------------------
`define IRQ_16
//`define IRQ_32
//`define IRQ_64


//-------------------------------------------------------
// Input synchronizers
//-------------------------------------------------------
// In some cases, the asynchronous input ports might
// already be synchronized externally.
// If an extensive CDC design review showed that this
// is really the case,  the individual synchronizers
// can be disabled with the following defines.
//
// Notes:
//        - all three signals are all sampled in the MCLK domain
//
//        - the dbg_en signal reset the debug interface
//         when 0. Therefore make sure it is glitch free.
//
//-------------------------------------------------------
`define SYNC_NMI
//`define SYNC_CPU_EN
//`define SYNC_DBG_EN


//-------------------------------------------------------
// Peripheral Memory Space:
//-------------------------------------------------------
// The original MSP430 architecture map the peripherals
// from 0x0000 to 0x01FF (i.e. 512B of the memory space).
// The following defines allow you to expand this space
// up to 32 kB (i.e. from 0x0000 to 0x7fff).
// As a consequence, the data memory mapping will be
// shifted up and a custom linker script will therefore
// be required by the GCC compiler.
//-------------------------------------------------------
//`define PER_SIZE_CUSTOM
//`define PER_SIZE_32_KB
//`define PER_SIZE_16_KB
//`define PER_SIZE_8_KB
//`define PER_SIZE_4_KB
//`define PER_SIZE_2_KB
//`define PER_SIZE_1_KB
`define PER_SIZE_512_B


//-------------------------------------------------------
// Defines the debugger CPU_CTL.RST_BRK_EN reset value
// (CPU break on PUC reset)
//-------------------------------------------------------
// When defined, the CPU will automatically break after
// a PUC occurrence by default. This is typically useful
// when the program memory can only be initialized through
// the serial debug interface.
//-------------------------------------------------------
`define DBG_RST_BRK_EN


//============================================================================
//============================================================================
// EXPERT SYSTEM CONFIGURATION ( !!!! EXPERTS ONLY !!!! )
//============================================================================
//============================================================================
//
// IMPORTANT NOTE:  Please update following configuration options ONLY if
//                 you have a good reason to do so... and if you know what
//                 you are doing :-P
//
//============================================================================

//-------------------------------------------------------
// Select serial debug interface protocol
//-------------------------------------------------------
//    DBG_UART -> Enable UART (8N1) debug interface
//    DBG_I2C  -> Enable I2C debug interface
//-------------------------------------------------------
`define DBG_UART
//`define DBG_I2C


//-------------------------------------------------------
// Enable the I2C broadcast address
//-------------------------------------------------------
// For multicore systems, a common I2C broadcast address
// can be given to all oMSP cores in order to
// synchronously RESET, START, STOP, or STEP all CPUs
// at once with a single I2C command.
// If you have a single openMSP430 in your system,
// this option can stay commented-out.
//-------------------------------------------------------
//`define DBG_I2C_BROADCAST


//-------------------------------------------------------
// Number of hardware breakpoint/watchpoint units
// (each unit contains two hardware addresses available
// for breakpoints or watchpoints):
//   - DBG_HWBRK_0 -> Include hardware breakpoints unit 0
//   - DBG_HWBRK_1 -> Include hardware breakpoints unit 1
//   - DBG_HWBRK_2 -> Include hardware breakpoints unit 2
//   - DBG_HWBRK_3 -> Include hardware breakpoints unit 3
//-------------------------------------------------------
// Please keep in mind that hardware breakpoints only
// make sense whenever the program memory is not an SRAM
// (i.e. Flash/OTP/ROM/...) or when you are interested
// in data breakpoints.
//-------------------------------------------------------
//`define  DBG_HWBRK_0
//`define  DBG_HWBRK_1
//`define  DBG_HWBRK_2
//`define  DBG_HWBRK_3


//-------------------------------------------------------
// Enable/Disable the hardware breakpoint RANGE mode
//-------------------------------------------------------
// When enabled this feature allows the hardware breakpoint
// units to stop the cpu whenever an instruction or data
// access lays within an address range.
// Note that this feature is not supported by GDB.
//-------------------------------------------------------
//`define DBG_HWBRK_RANGE


//-------------------------------------------------------
// Custom Program/Data and Peripheral Memory Spaces
//-------------------------------------------------------
// The following values are valid only if the
// corresponding *_SIZE_CUSTOM defines are uncommented:
//
//  - *_SIZE   : size of the section in bytes.
//  - *_AWIDTH : address port width, this value must allow
//               to address all WORDS of the section
//               (i.e. the *_SIZE divided by 2)
//-------------------------------------------------------

// Custom Program memory    (enabled with PMEM_SIZE_CUSTOM)
`define PMEM_CUSTOM_AWIDTH      10
`define PMEM_CUSTOM_SIZE      2048

// Custom Data memory       (enabled with DMEM_SIZE_CUSTOM)
`define DMEM_CUSTOM_AWIDTH       6
`define DMEM_CUSTOM_SIZE       128

// Custom Peripheral memory (enabled with PER_SIZE_CUSTOM)
`define PER_CUSTOM_AWIDTH        8
`define PER_CUSTOM_SIZE        512


//-------------------------------------------------------
// ASIC version
//-------------------------------------------------------
// When uncommented, this define will enable the
// ASIC system configuration section (see below) and
// will activate scan support for production test.
//
// WARNING: if you target an FPGA, leave this define
//          commented.
//-------------------------------------------------------
`define ASIC


//============================================================================
//============================================================================
// ASIC SYSTEM CONFIGURATION ( !!!! EXPERTS/PROFESSIONALS ONLY !!!! )
//============================================================================
//============================================================================
`ifdef ASIC

//===============================================================
// FINE GRAINED CLOCK GATING
//===============================================================

//-------------------------------------------------------
// When uncommented, this define will enable the fine
// grained clock gating of all registers in the core.
//-------------------------------------------------------
`define CLOCK_GATING


//===============================================================
// ASIC CLOCKING
//===============================================================

//-------------------------------------------------------
// When uncommented, this define will enable the ASIC
// architectural clock gating as well as the advanced low
// power modes support (most common).
// Comment this out in order to get FPGA-like clocking.
//-------------------------------------------------------
`define ASIC_CLOCKING


`ifdef ASIC_CLOCKING
//===============================================================
// LFXT CLOCK DOMAIN
//===============================================================

//-------------------------------------------------------
// When uncommented, this define will enable the lfxt_clk
// clock domain.
// When commented out, the whole chip is clocked with dco_clk.
//-------------------------------------------------------
`define LFXT_DOMAIN


//===============================================================
// CLOCK MUXES
//===============================================================

//-------------------------------------------------------
// MCLK: Clock Mux
//-------------------------------------------------------
// When uncommented, this define will enable the
// MCLK clock MUX allowing the selection between
// DCO_CLK and LFXT_CLK with the BCSCTL2.SELMx register.
// When commented, DCO_CLK is selected.
//-------------------------------------------------------
//`define MCLK_MUX

//-------------------------------------------------------
// SMCLK: Clock Mux
//-------------------------------------------------------
// When uncommented, this define will enable the
// SMCLK clock MUX allowing the selection between
// DCO_CLK and LFXT_CLK with the BCSCTL2.SELS register.
// When commented, DCO_CLK is selected.
//-------------------------------------------------------
//`define SMCLK_MUX

//-------------------------------------------------------
// WATCHDOG: Clock Mux
//-------------------------------------------------------
// When uncommented, this define will enable the
// Watchdog clock MUX allowing the selection between
// ACLK and SMCLK with the WDTCTL.WDTSSEL register.
// When commented out, ACLK is selected if the
// WATCHDOG_NOMUX_ACLK define is uncommented, SMCLK is
// selected otherwise.
//-------------------------------------------------------
//`define WATCHDOG_MUX
//`define WATCHDOG_NOMUX_ACLK


//===============================================================
// CLOCK DIVIDERS
//===============================================================

//-------------------------------------------------------
// MCLK: Clock divider
//-------------------------------------------------------
// When uncommented, this define will enable the
// MCLK clock divider (/1/2/4/8)
//-------------------------------------------------------
`define MCLK_DIVIDER

//-------------------------------------------------------
// SMCLK: Clock divider (/1/2/4/8)
//-------------------------------------------------------
// When uncommented, this define will enable the
// SMCLK clock divider
//-------------------------------------------------------
`define SMCLK_DIVIDER

//-------------------------------------------------------
// ACLK: Clock divider (/1/2/4/8)
//-------------------------------------------------------
// When uncommented, this define will enable the
// ACLK clock divider
//-------------------------------------------------------
`define ACLK_DIVIDER


//===============================================================
// LOW POWER MODES
//===============================================================

//-------------------------------------------------------
// LOW POWER MODE: CPUOFF
//-------------------------------------------------------
// When uncommented, this define will include the
// clock gate allowing to switch off MCLK in
// all low power modes: LPM0, LPM1, LPM2, LPM3, LPM4
//-------------------------------------------------------
`define CPUOFF_EN

//-------------------------------------------------------
// LOW POWER MODE: SCG0
//-------------------------------------------------------
// When uncommented, this define will enable the
// DCO_ENABLE/WKUP port control (always 1 when commented).
// This allows to switch off the DCO oscillator in the
// following low power modes: LPM1, LPM3, LPM4
//-------------------------------------------------------
`define SCG0_EN

//-------------------------------------------------------
// LOW POWER MODE: SCG1
//-------------------------------------------------------
// When uncommented, this define will include the
// clock gate allowing to switch off SMCLK in
// the following low power modes: LPM2, LPM3, LPM4
//-------------------------------------------------------
`define SCG1_EN

//-------------------------------------------------------
// LOW POWER MODE: OSCOFF
//-------------------------------------------------------
// When uncommented, this define will include the
// LFXT_CLK clock gate and enable the LFXT_ENABLE/WKUP
// port control (always 1 when commented).
// This allows to switch off the low frequency oscillator
// in the following low power modes: LPM4
//-------------------------------------------------------
`define OSCOFF_EN

//-------------------------------------------------------
// SCAN REPAIR NEG-EDGE CLOCKED FLIP-FLOPS
//-------------------------------------------------------
// When uncommented, a scan mux will be infered to
// replace all inverted clocks with regular ones when
// in scan mode.
//
// Note: standard scan insertion tool can usually deal
//       with mixed rising/falling edge FF... so there
//       is usually no need to uncomment this.
//-------------------------------------------------------
//`define SCAN_REPAIR_INV_CLOCKS

`endif
`endif

//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//=====        SYSTEM CONSTANTS --- !!!!!!!! DO NOT EDIT !!!!!!!!      =====//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

//
// PROGRAM, DATA & PERIPHERAL MEMORY CONFIGURATION
//==================================================

// Program Memory Size
`ifdef PMEM_SIZE_59_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     60416
`endif
`ifdef PMEM_SIZE_55_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     56320
`endif
`ifdef PMEM_SIZE_54_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     55296
`endif
`ifdef PMEM_SIZE_51_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     52224
`endif
`ifdef PMEM_SIZE_48_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     49152
`endif
`ifdef PMEM_SIZE_41_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     41984
`endif
`ifdef PMEM_SIZE_32_KB
  `define PMEM_AWIDTH      14
  `define PMEM_SIZE     32768
`endif
`ifdef PMEM_SIZE_24_KB
  `define PMEM_AWIDTH      14
  `define PMEM_SIZE     24576
`endif
`ifdef PMEM_SIZE_16_KB
  `define PMEM_AWIDTH      13
  `define PMEM_SIZE     16384
`endif
`ifdef PMEM_SIZE_12_KB
  `define PMEM_AWIDTH      13
  `define PMEM_SIZE     12288
`endif
`ifdef PMEM_SIZE_8_KB
  `define PMEM_AWIDTH      12
  `define PMEM_SIZE      8192
`endif
`ifdef PMEM_SIZE_4_KB
  `define PMEM_AWIDTH      11
  `define PMEM_SIZE      4096
`endif
`ifdef PMEM_SIZE_2_KB
  `define PMEM_AWIDTH      10
  `define PMEM_SIZE      2048
`endif
`ifdef PMEM_SIZE_1_KB
  `define PMEM_AWIDTH       9
  `define PMEM_SIZE      1024
`endif
`ifdef PMEM_SIZE_CUSTOM
  `define PMEM_AWIDTH       `PMEM_CUSTOM_AWIDTH
  `define PMEM_SIZE         `PMEM_CUSTOM_SIZE
`endif

// Data Memory Size
`ifdef DMEM_SIZE_32_KB
  `define DMEM_AWIDTH       14
  `define DMEM_SIZE      32768
`endif
`ifdef DMEM_SIZE_24_KB
  `define DMEM_AWIDTH       14
  `define DMEM_SIZE      24576
`endif
`ifdef DMEM_SIZE_16_KB
  `define DMEM_AWIDTH       13
  `define DMEM_SIZE      16384
`endif
`ifdef DMEM_SIZE_10_KB
  `define DMEM_AWIDTH       13
  `define DMEM_SIZE      10240
`endif
`ifdef DMEM_SIZE_8_KB
  `define DMEM_AWIDTH       12
  `define DMEM_SIZE       8192
`endif
`ifdef DMEM_SIZE_5_KB
  `define DMEM_AWIDTH       12
  `define DMEM_SIZE       5120
`endif
`ifdef DMEM_SIZE_4_KB
  `define DMEM_AWIDTH       11
  `define DMEM_SIZE       4096
`endif
`ifdef DMEM_SIZE_2p5_KB
  `define DMEM_AWIDTH       11
  `define DMEM_SIZE       2560
`endif
`ifdef DMEM_SIZE_2_KB
  `define DMEM_AWIDTH       10
  `define DMEM_SIZE       2048
`endif
`ifdef DMEM_SIZE_1_KB
  `define DMEM_AWIDTH        9
  `define DMEM_SIZE       1024
`endif
`ifdef DMEM_SIZE_512_B
  `define DMEM_AWIDTH        8
  `define DMEM_SIZE        512
`endif
`ifdef DMEM_SIZE_256_B
  `define DMEM_AWIDTH        7
  `define DMEM_SIZE        256
`endif
`ifdef DMEM_SIZE_128_B
  `define DMEM_AWIDTH        6
  `define DMEM_SIZE        128
`endif
`ifdef DMEM_SIZE_CUSTOM
  `define DMEM_AWIDTH       `DMEM_CUSTOM_AWIDTH
  `define DMEM_SIZE         `DMEM_CUSTOM_SIZE
`endif

// Peripheral Memory Size
`ifdef PER_SIZE_32_KB
  `define PER_AWIDTH        14
  `define PER_SIZE       32768
`endif
`ifdef PER_SIZE_16_KB
  `define PER_AWIDTH        13
  `define PER_SIZE       16384
`endif
`ifdef PER_SIZE_8_KB
  `define PER_AWIDTH        12
  `define PER_SIZE        8192
`endif
`ifdef PER_SIZE_4_KB
  `define PER_AWIDTH        11
  `define PER_SIZE        4096
`endif
`ifdef PER_SIZE_2_KB
  `define PER_AWIDTH        10
  `define PER_SIZE        2048
`endif
`ifdef PER_SIZE_1_KB
  `define PER_AWIDTH         9
  `define PER_SIZE        1024
`endif
`ifdef PER_SIZE_512_B
  `define PER_AWIDTH         8
  `define PER_SIZE         512
`endif
`ifdef PER_SIZE_CUSTOM
  `define PER_AWIDTH        `PER_CUSTOM_AWIDTH
  `define PER_SIZE          `PER_CUSTOM_SIZE
`endif

// Data Memory Base Adresses
`define DMEM_BASE  `PER_SIZE

// Program & Data Memory most significant address bit (for 16 bit words)
`define PMEM_MSB   `PMEM_AWIDTH-1
`define DMEM_MSB   `DMEM_AWIDTH-1
`define PER_MSB    `PER_AWIDTH-1

// Number of available IRQs
`ifdef  IRQ_16
`define IRQ_NR 16
`endif
`ifdef  IRQ_32
`define IRQ_NR 32
`define IRQ_NR_GE_32
`endif
`ifdef  IRQ_64
`define IRQ_NR 64
`define IRQ_NR_GE_32
`endif

//
// STATES, REGISTER FIELDS, ...
//======================================

// Instructions type
`define INST_SO  0
`define INST_JMP 1
`define INST_TO  2

// Single-operand arithmetic
`define RRC    0
`define SWPB   1
`define RRA    2
`define SXT    3
`define PUSH   4
`define CALL   5
`define RETI   6
`define IRQ    7

// Conditional jump
`define JNE    0
`define JEQ    1
`define JNC    2
`define JC     3
`define JN     4
`define JGE    5
`define JL     6
`define JMP    7

// Two-operand arithmetic
`define MOV    0
`define ADD    1
`define ADDC   2
`define SUBC   3
`define SUB    4
`define CMP    5
`define DADD   6
`define BIT    7
`define BIC    8
`define BIS    9
`define XOR   10
`define AND   11

// Addressing modes
`define DIR      0
`define IDX      1
`define INDIR    2
`define INDIR_I  3
`define SYMB     4
`define IMM      5
`define ABS      6
`define CONST    7

// Instruction state machine
`define I_IRQ_FETCH 3'h0
`define I_IRQ_DONE  3'h1
`define I_DEC       3'h2
`define I_EXT1      3'h3
`define I_EXT2      3'h4
`define I_IDLE      3'h5

// Execution state machine
// (swapped E_IRQ_0 and E_IRQ_2 values to suppress glitch generation warning from lint tool)
`define E_IRQ_0     4'h2
`define E_IRQ_1     4'h1
`define E_IRQ_2     4'h0
`define E_IRQ_3     4'h3
`define E_IRQ_4     4'h4
`define E_SRC_AD    4'h5
`define E_SRC_RD    4'h6
`define E_SRC_WR    4'h7
`define E_DST_AD    4'h8
`define E_DST_RD    4'h9
`define E_DST_WR    4'hA
`define E_EXEC      4'hB
`define E_JUMP      4'hC
`define E_IDLE      4'hD

// ALU control signals
`define ALU_SRC_INV   0
`define ALU_INC       1
`define ALU_INC_C     2
`define ALU_ADD       3
`define ALU_AND       4
`define ALU_OR        5
`define ALU_XOR       6
`define ALU_DADD      7
`define ALU_STAT_7    8
`define ALU_STAT_F    9
`define ALU_SHIFT    10
`define EXEC_NO_WR   11

// Debug interface
`define DBG_UART_WR   18
`define DBG_UART_BW   17
`define DBG_UART_ADDR 16:11

// Debug interface CPU_CTL register
`define HALT        0
`define RUN         1
`define ISTEP       2
`define SW_BRK_EN   3
`define FRZ_BRK_EN  4
`define RST_BRK_EN  5
`define CPU_RST     6

// Debug interface CPU_STAT register
`define HALT_RUN    0
`define PUC_PND     1
`define SWBRK_PND   3
`define HWBRK0_PND  4
`define HWBRK1_PND  5

// Debug interface BRKx_CTL register
`define BRK_MODE_RD 0
`define BRK_MODE_WR 1
`define BRK_MODE    1:0
`define BRK_EN      2
`define BRK_I_EN    3
`define BRK_RANGE   4

// Basic clock module: BCSCTL1 Control Register
`define DIVAx       5:4
`define DMA_CPUOFF  0
`define DMA_OSCOFF  1
`define DMA_SCG0    2
`define DMA_SCG1    3

// Basic clock module: BCSCTL2 Control Register
`define SELMx       7
`define DIVMx       5:4
`define SELS        3
`define DIVSx       2:1

// MCLK Clock gate
`ifdef CPUOFF_EN
  `define MCLK_CGATE
`else
`ifdef MCLK_DIVIDER
  `define MCLK_CGATE
`endif
`endif

// SMCLK Clock gate
`ifdef SCG1_EN
  `define SMCLK_CGATE
`else
`ifdef SMCLK_DIVIDER
  `define SMCLK_CGATE
`endif
`endif

//
// DEBUG INTERFACE EXTRA CONFIGURATION
//======================================

// Debug interface: CPU version
//   1 - FPGA support only (Pre-BSD licence era)
//   2 - Add ASIC support
//   3 - Add DMA interface support
`define CPU_VERSION   3'h3

// Debug interface: Software breakpoint opcode
`define DBG_SWBRK_OP 16'h4343

// Debug UART interface auto data synchronization
// If the following define is commented out, then
// the DBG_UART_BAUD and DBG_DCO_FREQ need to be properly
// defined.
`define DBG_UART_AUTO_SYNC

// Debug UART interface data rate
//      In order to properly setup the UART debug interface, you
//      need to specify the DCO_CLK frequency (DBG_DCO_FREQ) and
//      the chosen BAUD rate from the UART interface.
//
//`define DBG_UART_BAUD    9600
//`define DBG_UART_BAUD   19200
//`define DBG_UART_BAUD   38400
//`define DBG_UART_BAUD   57600
//`define DBG_UART_BAUD  115200
//`define DBG_UART_BAUD  230400
//`define DBG_UART_BAUD  460800
//`define DBG_UART_BAUD  576000
//`define DBG_UART_BAUD  921600
`define DBG_UART_BAUD 2000000
`define DBG_DCO_FREQ  20000000
`define DBG_UART_CNT ((`DBG_DCO_FREQ/`DBG_UART_BAUD)-1)

// Debug interface input synchronizer
`define SYNC_DBG_UART_RXD

// Enable/Disable the hardware breakpoint RANGE mode
`ifdef DBG_HWBRK_RANGE
 `define HWBRK_RANGE 1'b1
`else
 `define HWBRK_RANGE 1'b0
`endif

// Counter width for the debug interface UART
`define DBG_UART_XFER_CNT_W 16

// Check configuration
`ifdef DBG_EN
 `ifdef DBG_UART
   `ifdef DBG_I2C
CONFIGURATION ERROR: I2C AND UART DEBUG INTERFACE ARE BOTH ENABLED
   `endif
 `else
   `ifdef DBG_I2C
   `else
CONFIGURATION ERROR: I2C OR UART DEBUG INTERFACE SHOULD BE ENABLED
   `endif
 `endif
`endif

//
// MULTIPLIER CONFIGURATION
//======================================

// If uncommented, the following define selects
// the 16x16 multiplier (1 cycle) instead of the
// default 16x8 multplier (2 cycles)
//`define MPY_16x16

//======================================
// CONFIGURATION CHECKS
//======================================

`ifdef  IRQ_16
  `ifdef  IRQ_32
CONFIGURATION ERROR: ONLY ONE OF THE IRQ NUMBER OPTION CAN BE SELECTED
  `endif
  `ifdef  IRQ_64
CONFIGURATION ERROR: ONLY ONE OF THE IRQ NUMBER OPTION CAN BE SELECTED
  `endif
`endif
`ifdef  IRQ_32
  `ifdef  IRQ_64
CONFIGURATION ERROR: ONLY ONE OF THE IRQ NUMBER OPTION CAN BE SELECTED
  `endif
`endif
`ifdef LFXT_DOMAIN
`else
 `ifdef MCLK_MUX
CONFIGURATION ERROR: THE MCLK_MUX CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `endif
 `ifdef SMCLK_MUX
CONFIGURATION ERROR: THE SMCLK_MUX CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `endif
 `ifdef WATCHDOG_MUX
CONFIGURATION ERROR: THE WATCHDOG_MUX CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `else
   `ifdef WATCHDOG_NOMUX_ACLK
CONFIGURATION ERROR: THE WATCHDOG_NOMUX_ACLK CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
   `endif
 `endif
 `ifdef OSCOFF_EN
CONFIGURATION ERROR: THE OSCOFF LOW POWER MODE CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `endif
`endif
`endif

module  openMSP430_tb;

//
// Wire & Register definition
//------------------------------

// Data Memory interface
wire [`DMEM_MSB:0] dmem_addr;
wire               dmem_cen;
wire        [15:0] dmem_din;
wire         [1:0] dmem_wen;
wire        [15:0] dmem_dout;

// Program Memory interface
wire [`PMEM_MSB:0] pmem_addr;
wire               pmem_cen;
wire        [15:0] pmem_din;
wire         [1:0] pmem_wen;
wire        [15:0] pmem_dout;

// Peripherals interface
wire        [13:0] per_addr;
wire        [15:0] per_din;
wire        [15:0] per_dout;
wire         [1:0] per_we;
wire               per_en;

// Direct Memory Access interface
wire        [15:0] dma_dout;
wire               dma_ready;
wire               dma_resp;
reg         [15:1] dma_addr;
reg         [15:0] dma_din;
reg                dma_en;
reg                dma_priority;
reg          [1:0] dma_we;
reg                dma_wkup;

// Digital I/O
wire               irq_port1;
wire               irq_port2;
wire        [15:0] per_dout_dio;
wire         [7:0] p1_dout;
wire         [7:0] p1_dout_en;
wire         [7:0] p1_sel;
wire         [7:0] p2_dout;
wire         [7:0] p2_dout_en;
wire         [7:0] p2_sel;
wire         [7:0] p3_dout;
wire         [7:0] p3_dout_en;
wire         [7:0] p3_sel;
wire         [7:0] p4_dout;
wire         [7:0] p4_dout_en;
wire         [7:0] p4_sel;
wire         [7:0] p5_dout;
wire         [7:0] p5_dout_en;
wire         [7:0] p5_sel;
wire         [7:0] p6_dout;
wire         [7:0] p6_dout_en;
wire         [7:0] p6_sel;
reg          [7:0] p1_din;
reg          [7:0] p2_din;
reg          [7:0] p3_din;
reg          [7:0] p4_din;
reg          [7:0] p5_din;
reg          [7:0] p6_din;

// Peripheral templates
wire        [15:0] per_dout_temp_8b;
wire        [15:0] per_dout_temp_16b;

// Timer A
wire               irq_ta0;
wire               irq_ta1;
wire        [15:0] per_dout_timerA;
reg                inclk;
reg                taclk;
reg                ta_cci0a;
reg                ta_cci0b;
reg                ta_cci1a;
reg                ta_cci1b;
reg                ta_cci2a;
reg                ta_cci2b;
wire               ta_out0;
wire               ta_out0_en;
wire               ta_out1;
wire               ta_out1_en;
wire               ta_out2;
wire               ta_out2_en;

// Clock / Reset & Interrupts
reg                dco_clk;
wire               dco_enable;
wire               dco_wkup;
reg                dco_local_enable;
reg                lfxt_clk;
wire               lfxt_enable;
wire               lfxt_wkup;
reg                lfxt_local_enable;
wire               mclk;
wire               aclk;
wire               aclk_en;
wire               smclk;
wire               smclk_en;
reg                reset_n;
wire               puc_rst;
reg                nmi;
reg  [`IRQ_NR-3:0] irq;
wire [`IRQ_NR-3:0] irq_acc;
wire [`IRQ_NR-3:0] irq_in;
reg                cpu_en;
reg         [13:0] wkup;
wire        [13:0] wkup_in;

// Scan (ASIC version only)
reg                scan_enable;
reg                scan_mode;

// Debug interface: UART
reg                dbg_en;
wire               dbg_freeze;
wire               dbg_uart_txd;
wire               dbg_uart_rxd;
reg                dbg_uart_rxd_sel;
reg                dbg_uart_rxd_dly;
reg                dbg_uart_rxd_pre;
reg                dbg_uart_rxd_meta;
reg         [15:0] dbg_uart_buf;
reg                dbg_uart_rx_busy;
reg                dbg_uart_tx_busy;

// Debug interface: I2C
wire               dbg_scl;
wire               dbg_sda;
wire               dbg_scl_slave;
wire               dbg_scl_master;
reg                dbg_scl_master_sel;
reg                dbg_scl_master_dly;
reg                dbg_scl_master_pre;
reg                dbg_scl_master_meta;
wire               dbg_sda_slave_out;
wire               dbg_sda_slave_in;
wire               dbg_sda_master_out;
reg                dbg_sda_master_out_sel;
reg                dbg_sda_master_out_dly;
reg                dbg_sda_master_out_pre;
reg                dbg_sda_master_out_meta;
wire               dbg_sda_master_in;
reg         [15:0] dbg_i2c_buf;
reg     [8*32-1:0] dbg_i2c_string;

// Core testbench debuging signals
wire    [8*32-1:0] i_state;
wire    [8*32-1:0] e_state;
wire        [31:0] inst_cycle;
wire    [8*32-1:0] inst_full;
wire        [31:0] inst_number;
wire        [15:0] inst_pc;
wire    [8*32-1:0] inst_short;

// Testbench variables
integer            tb_idx;
integer            tmp_seed;
integer            error;
reg                stimulus_done;


//
// Include files
//------------------------------

// CPU & Memory registers
//----------------------------------------------------------------------------
// Copyright (C) 2001 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: registers.v
//
// *Module Description:
//                      Direct connections to internal registers & memory.
//
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 145 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2012-05-30 23:03:05 +0200 (Wed, 30 May 2012) $
//----------------------------------------------------------------------------

// CPU registers
//======================

wire       [15:0] r0    = dut.execution_unit_0.register_file_0.r0;
wire       [15:0] r1    = dut.execution_unit_0.register_file_0.r1;
wire       [15:0] r2    = dut.execution_unit_0.register_file_0.r2;
wire       [15:0] r3    = dut.execution_unit_0.register_file_0.r3;
wire       [15:0] r4    = dut.execution_unit_0.register_file_0.r4;
wire       [15:0] r5    = dut.execution_unit_0.register_file_0.r5;
wire       [15:0] r6    = dut.execution_unit_0.register_file_0.r6;
wire       [15:0] r7    = dut.execution_unit_0.register_file_0.r7;
wire       [15:0] r8    = dut.execution_unit_0.register_file_0.r8;
wire       [15:0] r9    = dut.execution_unit_0.register_file_0.r9;
wire       [15:0] r10   = dut.execution_unit_0.register_file_0.r10;
wire       [15:0] r11   = dut.execution_unit_0.register_file_0.r11;
wire       [15:0] r12   = dut.execution_unit_0.register_file_0.r12;
wire       [15:0] r13   = dut.execution_unit_0.register_file_0.r13;
wire       [15:0] r14   = dut.execution_unit_0.register_file_0.r14;
wire       [15:0] r15   = dut.execution_unit_0.register_file_0.r15;


// RAM cells
//======================

wire       [15:0] mem200 = dmem_0.mem[0];
wire       [15:0] mem202 = dmem_0.mem[1];
wire       [15:0] mem204 = dmem_0.mem[2];
wire       [15:0] mem206 = dmem_0.mem[3];
wire       [15:0] mem208 = dmem_0.mem[4];
wire       [15:0] mem20A = dmem_0.mem[5];
wire       [15:0] mem20C = dmem_0.mem[6];
wire       [15:0] mem20E = dmem_0.mem[7];
wire       [15:0] mem210 = dmem_0.mem[8];
wire       [15:0] mem212 = dmem_0.mem[9];
wire       [15:0] mem214 = dmem_0.mem[10];
wire       [15:0] mem216 = dmem_0.mem[11];
wire       [15:0] mem218 = dmem_0.mem[12];
wire       [15:0] mem21A = dmem_0.mem[13];
wire       [15:0] mem21C = dmem_0.mem[14];
wire       [15:0] mem21E = dmem_0.mem[15];
wire       [15:0] mem220 = dmem_0.mem[16];
wire       [15:0] mem222 = dmem_0.mem[17];
wire       [15:0] mem224 = dmem_0.mem[18];
wire       [15:0] mem226 = dmem_0.mem[19];
wire       [15:0] mem228 = dmem_0.mem[20];
wire       [15:0] mem22A = dmem_0.mem[21];
wire       [15:0] mem22C = dmem_0.mem[22];
wire       [15:0] mem22E = dmem_0.mem[23];
wire       [15:0] mem230 = dmem_0.mem[24];
wire       [15:0] mem232 = dmem_0.mem[25];
wire       [15:0] mem234 = dmem_0.mem[26];
wire       [15:0] mem236 = dmem_0.mem[27];
wire       [15:0] mem238 = dmem_0.mem[28];
wire       [15:0] mem23A = dmem_0.mem[29];
wire       [15:0] mem23C = dmem_0.mem[30];
wire       [15:0] mem23E = dmem_0.mem[31];
wire       [15:0] mem240 = dmem_0.mem[32];
wire       [15:0] mem242 = dmem_0.mem[33];
wire       [15:0] mem244 = dmem_0.mem[34];
wire       [15:0] mem246 = dmem_0.mem[35];
wire       [15:0] mem248 = dmem_0.mem[36];
wire       [15:0] mem24A = dmem_0.mem[37];
wire       [15:0] mem24C = dmem_0.mem[38];
wire       [15:0] mem24E = dmem_0.mem[39];
wire       [15:0] mem250 = dmem_0.mem[40];
wire       [15:0] mem252 = dmem_0.mem[41];
wire       [15:0] mem254 = dmem_0.mem[42];
wire       [15:0] mem256 = dmem_0.mem[43];
wire       [15:0] mem258 = dmem_0.mem[44];
wire       [15:0] mem25A = dmem_0.mem[45];
wire       [15:0] mem25C = dmem_0.mem[46];
wire       [15:0] mem25E = dmem_0.mem[47];
wire       [15:0] mem260 = dmem_0.mem[48];
wire       [15:0] mem262 = dmem_0.mem[49];
wire       [15:0] mem264 = dmem_0.mem[50];
wire       [15:0] mem266 = dmem_0.mem[51];
wire       [15:0] mem268 = dmem_0.mem[52];
wire       [15:0] mem26A = dmem_0.mem[53];
wire       [15:0] mem26C = dmem_0.mem[54];
wire       [15:0] mem26E = dmem_0.mem[55];
wire       [15:0] mem270 = dmem_0.mem[56];
wire       [15:0] mem272 = dmem_0.mem[57];
wire       [15:0] mem274 = dmem_0.mem[58];
wire       [15:0] mem276 = dmem_0.mem[59];
wire       [15:0] mem278 = dmem_0.mem[60];
wire       [15:0] mem27A = dmem_0.mem[61];
wire       [15:0] mem27C = dmem_0.mem[62];
wire       [15:0] mem27E = dmem_0.mem[63];


// Interrupt vectors
//======================

wire       [15:0] irq_vect_15 = pmem_0.mem[(`PMEM_SIZE>>1)-1];  // RESET Vector
wire       [15:0] irq_vect_14 = pmem_0.mem[(`PMEM_SIZE>>1)-2];  // NMI
wire       [15:0] irq_vect_13 = pmem_0.mem[(`PMEM_SIZE>>1)-3];  // IRQ 13
wire       [15:0] irq_vect_12 = pmem_0.mem[(`PMEM_SIZE>>1)-4];  // IRQ 12
wire       [15:0] irq_vect_11 = pmem_0.mem[(`PMEM_SIZE>>1)-5];  // IRQ 11
wire       [15:0] irq_vect_10 = pmem_0.mem[(`PMEM_SIZE>>1)-6];  // IRQ 10
wire       [15:0] irq_vect_09 = pmem_0.mem[(`PMEM_SIZE>>1)-7];  // IRQ  9
wire       [15:0] irq_vect_08 = pmem_0.mem[(`PMEM_SIZE>>1)-8];  // IRQ  8
wire       [15:0] irq_vect_07 = pmem_0.mem[(`PMEM_SIZE>>1)-9];  // IRQ  7
wire       [15:0] irq_vect_06 = pmem_0.mem[(`PMEM_SIZE>>1)-10]; // IRQ  6
wire       [15:0] irq_vect_05 = pmem_0.mem[(`PMEM_SIZE>>1)-11]; // IRQ  5
wire       [15:0] irq_vect_04 = pmem_0.mem[(`PMEM_SIZE>>1)-12]; // IRQ  4
wire       [15:0] irq_vect_03 = pmem_0.mem[(`PMEM_SIZE>>1)-13]; // IRQ  3
wire       [15:0] irq_vect_02 = pmem_0.mem[(`PMEM_SIZE>>1)-14]; // IRQ  2
wire       [15:0] irq_vect_01 = pmem_0.mem[(`PMEM_SIZE>>1)-15]; // IRQ  1
wire       [15:0] irq_vect_00 = pmem_0.mem[(`PMEM_SIZE>>1)-16]; // IRQ  0

// Interrupt detection
wire              nmi_detect  = dut.frontend_0.nmi_pnd;
wire              irq_detect  = dut.frontend_0.irq_detect;

// Debug interface
wire              dbg_clk     = dut.clock_module_0.dbg_clk;
wire              dbg_rst     = dut.clock_module_0.dbg_rst;


// CPU ID
//======================

 wire  [2:0] dbg_cpu_version  =  `CPU_VERSION;
`ifdef ASIC
 wire        dbg_cpu_asic     =  1'b1;
`else
 wire        dbg_cpu_asic     =  1'b0;
`endif
 wire  [4:0] dbg_user_version =  `USER_VERSION;
 wire  [6:0] dbg_per_space    = (`PER_SIZE  >> 9);
`ifdef MULTIPLIER
 wire        dbg_mpy_info     =  1'b1;
`else
 wire        dbg_mpy_info     =  1'b0;
`endif
 wire  [8:0] dbg_dmem_size    = (`DMEM_SIZE >> 7);
 wire  [5:0] dbg_pmem_size    = (`PMEM_SIZE >> 10);

 wire [31:0] dbg_cpu_id       = {dbg_pmem_size,
                                 dbg_dmem_size,
                                 dbg_mpy_info,
                                 dbg_per_space,
                                 dbg_user_version,
                                 dbg_cpu_asic,
                                 dbg_cpu_version};


// Debug interface tasks
//----------------------------------------------------------------------------
// Copyright (C) 2001 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: dbg_uart_tasks.v
//
// *Module Description:
//                      openMSP430 debug interface UART tasks
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 154 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2012-10-15 22:44:20 +0200 (Mon, 15 Oct 2012) $
//----------------------------------------------------------------------------

// Register B/W and addresses
parameter           CPU_ID_LO    =  (8'h00 | 8'h00);
parameter           CPU_ID_HI    =  (8'h00 | 8'h01);
parameter           CPU_CTL      =  (8'h40 | 8'h02);
parameter           CPU_STAT     =  (8'h40 | 8'h03);
parameter           MEM_CTL      =  (8'h40 | 8'h04);
parameter           MEM_ADDR     =  (8'h00 | 8'h05);
parameter           MEM_DATA     =  (8'h00 | 8'h06);
parameter           MEM_CNT      =  (8'h00 | 8'h07);
parameter           BRK0_CTL     =  (8'h40 | 8'h08);
parameter           BRK0_STAT    =  (8'h40 | 8'h09);
parameter           BRK0_ADDR0   =  (8'h00 | 8'h0A);
parameter           BRK0_ADDR1   =  (8'h00 | 8'h0B);
parameter           BRK1_CTL     =  (8'h40 | 8'h0C);
parameter           BRK1_STAT    =  (8'h40 | 8'h0D);
parameter           BRK1_ADDR0   =  (8'h00 | 8'h0E);
parameter           BRK1_ADDR1   =  (8'h00 | 8'h0F);
parameter           BRK2_CTL     =  (8'h40 | 8'h10);
parameter           BRK2_STAT    =  (8'h40 | 8'h11);
parameter           BRK2_ADDR0   =  (8'h00 | 8'h12);
parameter           BRK2_ADDR1   =  (8'h00 | 8'h13);
parameter           BRK3_CTL     =  (8'h40 | 8'h14);
parameter           BRK3_STAT    =  (8'h40 | 8'h15);
parameter           BRK3_ADDR0   =  (8'h00 | 8'h16);
parameter           BRK3_ADDR1   =  (8'h00 | 8'h17);
parameter           CPU_NR       =  (8'h00 | 8'h18);

// Read / Write commands
parameter           DBG_WR       =   8'h80;
parameter           DBG_RD       =   8'h00;

// Synchronization value
parameter           DBG_SYNC     =   8'h80;


//----------------------------------------------------------------------------
// UART COMMUNICATION DATA RATE CONFIGURATION
//----------------------------------------------------------------------------
// If the auto synchronization mode is set, then the communication speed
// is configured by the testbench.
// If not, the values from the openMSP430.inc file are taken over.
`ifdef DBG_UART_AUTO_SYNC
parameter UART_BAUD   = 4000000;
integer   UART_PERIOD = 1000000000/UART_BAUD;
`else
integer   UART_PERIOD = `DBG_UART_CNT;
`endif

//----------------------------------------------------------------------------
// Receive UART frame from CPU Debug interface (8N1)
//----------------------------------------------------------------------------

task dbg_uart_rx;
   output [7:0] dbg_rxbuf;

   reg [7:0] 	dbg_rxbuf;
   reg [7:0] 	rxbuf;
   integer 	rxcnt;
   begin
      #(1);
      dbg_uart_rx_busy = 1'b1;
      @(negedge dbg_uart_txd);
      dbg_rxbuf = 0;
      rxbuf     = 0;
      #(3*UART_PERIOD/2);
      for (rxcnt = 0; rxcnt < 8; rxcnt = rxcnt + 1)
	begin
	   rxbuf = {dbg_uart_txd, rxbuf[7:1]};
	   #(UART_PERIOD);
	end
      dbg_rxbuf        = rxbuf;
      dbg_uart_rx_busy = 1'b0;
   end
endtask

task dbg_uart_rx16;

   reg [7:0] rxbuf_lo;
   reg [7:0] rxbuf_hi;
   begin
      rxbuf_lo = 8'h00;
      rxbuf_hi = 8'h00;
      dbg_uart_rx(rxbuf_lo);
      dbg_uart_rx(rxbuf_hi);
      dbg_uart_buf = {rxbuf_hi, rxbuf_lo};
   end
endtask

task dbg_uart_rx8;

   reg [7:0] rxbuf;
   begin
      rxbuf = 8'h00;
      dbg_uart_rx(rxbuf);
      dbg_uart_buf = {8'h00, rxbuf};
   end
endtask

//----------------------------------------------------------------------------
// Transmit UART frame to CPU Debug interface (8N1)
//----------------------------------------------------------------------------
task dbg_uart_tx;
   input  [7:0] txbuf;

   reg [9:0] 	txbuf_full;
   integer 	txcnt;
   begin
      #(1);
      dbg_uart_tx_busy = 1'b1;
      dbg_uart_rxd_pre = 1'b1;
      txbuf_full       = {1'b1, txbuf, 1'b0};
      for (txcnt = 0; txcnt < 10; txcnt = txcnt + 1)
	begin
	   #(UART_PERIOD);
	   dbg_uart_rxd_pre =  txbuf_full[txcnt];
	end
      dbg_uart_tx_busy = 1'b0;
   end
endtask

task dbg_uart_tx16;
   input  [15:0] txbuf;

   begin
      dbg_uart_tx(txbuf[7:0]);
      dbg_uart_tx(txbuf[15:8]);
   end
endtask

always @(posedge mclk or posedge dbg_rst)
  if (dbg_rst)
    begin
       dbg_uart_rxd_sel <= 1'b0;
       dbg_uart_rxd_dly <= 1'b1;
    end
  else if (dbg_en)
    begin
       dbg_uart_rxd_sel <= dbg_uart_rxd_meta ? $random : 1'b0;
       dbg_uart_rxd_dly <= dbg_uart_rxd_pre;
    end

assign dbg_uart_rxd = dbg_uart_rxd_sel ? dbg_uart_rxd_dly : dbg_uart_rxd_pre;


//----------------------------------------------------------------------------
// Write to Debug register
//----------------------------------------------------------------------------
task dbg_uart_wr;
   input  [7:0] dbg_reg;
   input [15:0] dbg_data;

   begin
      dbg_uart_tx(DBG_WR | dbg_reg);
      dbg_uart_tx(dbg_data[7:0]);
      if (~dbg_reg[6])
	dbg_uart_tx(dbg_data[15:8]);
   end
endtask


//----------------------------------------------------------------------------
// Read Debug register
//----------------------------------------------------------------------------
task dbg_uart_rd;
   input  [7:0] dbg_reg;

   reg [7:0] 	rxbuf_lo;
   reg [7:0] 	rxbuf_hi;
   begin
      rxbuf_lo = 8'h00;
      rxbuf_hi = 8'h00;
      dbg_uart_tx(DBG_RD | dbg_reg);
      dbg_uart_rx(rxbuf_lo);
      if (~dbg_reg[6])
	dbg_uart_rx(rxbuf_hi);

      dbg_uart_buf = {rxbuf_hi, rxbuf_lo};
      end
endtask

//----------------------------------------------------------------------------
// Send synchronization frame
//----------------------------------------------------------------------------
task dbg_uart_sync;
   begin
      dbg_uart_tx(DBG_SYNC);
      repeat(10) @(posedge mclk);
   end
endtask



//----------------------------------------------------------------------------
// Copyright (C) 2001 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: dbg_i2c_tasks.v
//
// *Module Description:
//                      openMSP430 debug interface I2C tasks
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 17 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2009-08-04 23:15:39 +0200 (Tue, 04 Aug 2009) $
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// I2C COMMUNICATION CONFIGURATION
//----------------------------------------------------------------------------

// Data rate
parameter I2C_FREQ      = 2000000;
integer   I2C_PERIOD    = 1000000000/I2C_FREQ;

// Address
parameter I2C_ADDR      = 7'h45;
parameter I2C_BROADCAST = 7'h67;


//----------------------------------------------------------------------------
// Generate START / STOP conditions
//----------------------------------------------------------------------------
task dbg_i2c_start;
   begin
      dbg_i2c_string          = "Start";
      dbg_sda_master_out_pre  = 1'b0;
      #(I2C_PERIOD/2);
      dbg_scl_master_pre      = 1'b0;
      #(I2C_PERIOD/4);
      dbg_i2c_string          = "";
   end
endtask

task dbg_i2c_stop;
   begin
      dbg_i2c_string          = "Stop";
      dbg_sda_master_out_pre  = 1'b0;
      #(I2C_PERIOD/4);
      dbg_scl_master_pre      = 1'b1;
      #(I2C_PERIOD/4);
      dbg_sda_master_out_pre  = 1'b1;
      #(I2C_PERIOD/2);
      dbg_i2c_string          = "";
   end
endtask

//----------------------------------------------------------------------------
// Send a byte on the I2C bus
//----------------------------------------------------------------------------
task dbg_i2c_send;
   input  [7:0] txbuf;

   reg [9:0] 	txbuf_full;
   integer 	txcnt;
   begin
      #(1);
      txbuf_full = txbuf;
      for (txcnt = 0; txcnt < 8; txcnt = txcnt + 1)
	begin
	   $sformat(dbg_i2c_string, "TX_%-d", txcnt);
      	   dbg_sda_master_out_pre = txbuf_full[7-txcnt];
	   #(I2C_PERIOD/4);
	   dbg_scl_master_pre     = 1'b1;
	   #(I2C_PERIOD/2);
	   dbg_scl_master_pre     = 1'b0;
	   #(I2C_PERIOD/4);
	end
      dbg_sda_master_out_pre = 1'b1;
      dbg_i2c_string         = "";
   end
endtask

//----------------------------------------------------------------------------
// Read ACK / NACK
//----------------------------------------------------------------------------
task dbg_i2c_ack_rd;
   begin
      dbg_i2c_string      = "ACK (rd)";
      #(I2C_PERIOD/4);
      dbg_scl_master_pre  = 1'b1;
      #(I2C_PERIOD/2);
      dbg_scl_master_pre  = 1'b0;
      #(I2C_PERIOD/4);
      dbg_i2c_string      = "";
   end
endtask


//----------------------------------------------------------------------------
// Read a byte from the I2C bus
//----------------------------------------------------------------------------
task dbg_i2c_receive;
   output [7:0] rxbuf;

   reg [9:0] 	rxbuf_full;
   integer 	rxcnt;
   begin
      #(1);
      rxbuf_full = 0;
      for (rxcnt = 0; rxcnt < 8; rxcnt = rxcnt + 1)
	begin
	   $sformat(dbg_i2c_string, "RX_%-d", rxcnt);
	   #(I2C_PERIOD/4);
	   dbg_scl_master_pre  = 1'b1;
	   #(I2C_PERIOD/4);
	   rxbuf_full[7-rxcnt] = dbg_sda;
	   #(I2C_PERIOD/4);
	   dbg_scl_master_pre  = 1'b0;
	   #(I2C_PERIOD/4);
	end
      dbg_i2c_string      = "";
      rxbuf = rxbuf_full;
   end
endtask


//----------------------------------------------------------------------------
// Write ACK
//----------------------------------------------------------------------------
task dbg_i2c_ack_wr;
   begin
      dbg_i2c_string          = "ACK (wr)";
      dbg_sda_master_out_pre  = 1'b0;
      #(I2C_PERIOD/4);
      dbg_scl_master_pre      = 1'b1;
      #(I2C_PERIOD/2);
      dbg_scl_master_pre      = 1'b0;
      #(I2C_PERIOD/4);
      dbg_sda_master_out_pre  = 1'b1;
      dbg_i2c_string          = "";
   end
endtask

//----------------------------------------------------------------------------
// Write NACK
//----------------------------------------------------------------------------
task dbg_i2c_nack_wr;
   begin
      dbg_i2c_string          = "NACK (wr)";
      dbg_sda_master_out_pre  = 1'b1;
      #(I2C_PERIOD/4);
      dbg_scl_master_pre      = 1'b1;
      #(I2C_PERIOD/2);
      dbg_scl_master_pre      = 1'b0;
      #(I2C_PERIOD/4);
      dbg_sda_master_out_pre  = 1'b1;
      dbg_i2c_string          = "";
   end
endtask

//----------------------------------------------------------------------------
// Start Burst
//----------------------------------------------------------------------------
task dbg_i2c_burst_start;
   input        read;
   begin
      dbg_i2c_start;                     // START
      dbg_i2c_send({I2C_ADDR, read});    // Device Address + Write access
      dbg_i2c_ack_rd;
   end
endtask


//----------------------------------------------------------------------------
// Read 16 bits
//----------------------------------------------------------------------------
task dbg_i2c_rx16;
   input        is_last;

   reg [7:0] 	rxbuf_lo;
   reg [7:0] 	rxbuf_hi;
   begin
      rxbuf_lo = 8'h00;
      rxbuf_hi = 8'h00;

      dbg_i2c_receive(rxbuf_lo);        // Data (low)
      dbg_i2c_ack_wr;
      dbg_i2c_receive(rxbuf_hi);        // Data (high)
      if (is_last)
	begin
	   dbg_i2c_nack_wr;
	   dbg_i2c_stop;                // STOP
	end
      else
	begin
	   dbg_i2c_ack_wr;
      	end

      dbg_i2c_buf = {rxbuf_hi, rxbuf_lo};
      end
endtask

//----------------------------------------------------------------------------
// Transmit 16 bits
//----------------------------------------------------------------------------
task dbg_i2c_tx16;
   input [15:0] dbg_data;
   input        is_last;

   begin
      dbg_i2c_send(dbg_data[7:0]);  // write LSB
      dbg_i2c_ack_rd;
      dbg_i2c_send(dbg_data[15:8]); // write MSB
      dbg_i2c_ack_rd;
      if (is_last)
	dbg_i2c_stop;               // STOP CONDITION
   end
endtask

//----------------------------------------------------------------------------
// Read 8 bits
//----------------------------------------------------------------------------
task dbg_i2c_rx8;
   input        is_last;

   reg [7:0] 	rxbuf;
   begin
      rxbuf = 8'h00;

      dbg_i2c_receive(rxbuf);        // Data (low)
      if (is_last)
	begin
	   dbg_i2c_nack_wr;
	   dbg_i2c_stop;             // STOP
	end
      else
	begin
	   dbg_i2c_ack_wr;
      	end

      dbg_i2c_buf = {8'h00, rxbuf};
      end
endtask

//----------------------------------------------------------------------------
// Transmit 8 bits
//----------------------------------------------------------------------------
task dbg_i2c_tx8;
   input [7:0] dbg_data;
   input       is_last;

   begin
      dbg_i2c_send(dbg_data);  // write LSB
      dbg_i2c_ack_rd;
      if (is_last)
	dbg_i2c_stop;          // STOP CONDITION
   end
endtask

//----------------------------------------------------------------------------
// Write to Debug register
//----------------------------------------------------------------------------
task dbg_i2c_wr;
   input  [7:0] dbg_reg;
   input [15:0] dbg_data;

   begin
      dbg_i2c_start;                     // START
      dbg_i2c_tx8({I2C_ADDR, 1'b0}, 0);  // Device Address + Write access
      dbg_i2c_tx8(DBG_WR | dbg_reg, 0);  // Command

      if (~dbg_reg[6])
	dbg_i2c_tx16(dbg_data,      1);
      else
	dbg_i2c_tx8 (dbg_data[7:0], 1);

   end
endtask

//----------------------------------------------------------------------------
// Read Debug register
//----------------------------------------------------------------------------
task dbg_i2c_rd;
   input  [7:0] dbg_reg;

   reg [7:0] 	rxbuf_lo;
   reg [7:0] 	rxbuf_hi;
   begin
      rxbuf_lo = 8'h00;
      rxbuf_hi = 8'h00;

      dbg_i2c_start;                     // START
      dbg_i2c_tx8({I2C_ADDR, 1'b0}, 0);  // Device Address + Write access
      dbg_i2c_tx8(DBG_RD | dbg_reg, 1);  // Command

      dbg_i2c_start;                     // START
      dbg_i2c_tx8({I2C_ADDR, 1'b1}, 0);  // Device Address + Read access

      if (~dbg_reg[6])
	dbg_i2c_rx16(1);
      else
	dbg_i2c_rx8(1);

   end
endtask

//----------------------------------------------------------------------------
// Build random delay insertion on SCL_MASTER and SDA_MASTER_OUT in order to
// simulate synchronization mechanism
//----------------------------------------------------------------------------

always @(posedge mclk or posedge dbg_rst)
  if (dbg_rst)
    begin
       dbg_sda_master_out_sel <= 1'b0;
       dbg_sda_master_out_dly <= 1'b1;

       dbg_scl_master_sel     <= 1'b0;
       dbg_scl_master_dly     <= 1'b1;
    end
  else if (dbg_en)
    begin
       dbg_sda_master_out_sel <= dbg_sda_master_out_meta ? $random : 1'b0;
       dbg_sda_master_out_dly <= dbg_sda_master_out_pre;

       dbg_scl_master_sel     <= dbg_scl_master_meta     ? $random : 1'b0;
       dbg_scl_master_dly     <= dbg_scl_master_pre;
    end

assign dbg_sda_master_out = dbg_sda_master_out_sel ? dbg_sda_master_out_dly : dbg_sda_master_out_pre;

assign dbg_scl_master     = dbg_scl_master_sel     ? dbg_scl_master_dly     : dbg_scl_master_pre;



// Direct Memory Access interface tasks
//----------------------------------------------------------------------------
// Copyright (C) 2014 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: dma_tasks.v
//
// *Module Description:
//                      generic tasks for using the Direct Memory Access interface
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev$
// $LastChangedBy$
// $LastChangedDate$
//----------------------------------------------------------------------------

//============================================================================
// DMA Write access
//============================================================================
integer    dma_cnt_wr;
integer    dma_cnt_rd;
integer    dma_wr_error;
integer    dma_rd_error;
reg        dma_tfx_cancel;

//---------------------
// Generic write task
//---------------------
task dma_write;
   input  [15:0] addr;   // Address
   input  [15:0] data;   // Data
   input         resp;   // Expected transfer response (0: Okay / 1: Error)
   input         size;   // Access size (0: 8-bit / 1: 16-bit)

   begin
      dma_addr   = addr[15:1];
      dma_en     = 1'b1;
      dma_we     = size    ? 2'b11  :
                   addr[0] ? 2'b10  :  2'b01;
      dma_din    = data;
      @(posedge mclk or posedge dma_tfx_cancel);
      while(~dma_ready & ~dma_tfx_cancel) @(posedge mclk or posedge dma_tfx_cancel);
      dma_en     = 1'b0;
      dma_we     = 2'b00;
      dma_addr   = 15'h0000;
      dma_din    = 16'h0000;
      if (~dma_tfx_cancel) dma_cnt_wr = dma_cnt_wr+1;

      // Check transfer response
      if (~dma_tfx_cancel & (dma_resp != resp))
	begin
	   $display("ERROR: DMA interface write response check -- address: 0x%h -- response: %h / expected: %h (%t ns)", addr, dma_resp, resp, $time);
	   dma_wr_error = dma_wr_error+1;
	end
   end
endtask

//---------------------
// Write 16b task
//---------------------
task dma_write_16b;
   input  [15:0] addr;   // Address
   input  [15:0] data;   // Data
   input         resp;   // Expected transfer response (0: Okay / 1: Error)

   begin
      dma_write(addr, data, resp, 1'b1);
   end
endtask

//---------------------
// Write 8b task
//---------------------
task dma_write_8b;
   input  [15:0] addr;   // Address
   input   [7:0] data;   // Data
   input         resp;   // Expected transfer response (0: Okay / 1: Error)

   begin
      if (addr[0]) dma_write(addr, {data,  8'h00}, resp, 1'b0);
      else         dma_write(addr, {8'h00, data }, resp, 1'b0);
   end
endtask


//============================================================================
// DMA read access
//============================================================================

//---------------------
// Read check process
//---------------------
reg        dma_read_check_active;
reg [15:0] dma_read_check_addr;
reg [15:0] dma_read_check_data;
reg [15:0] dma_read_check_mask;

initial
  begin
     dma_read_check_active =  1'b0;
     dma_read_check_addr   = 16'h0000;
     dma_read_check_data   = 16'h0000;
     dma_read_check_mask   = 16'h0000;
     forever
       begin
	  @(negedge (mclk & dma_read_check_active) or posedge dma_tfx_cancel);
	  if (~dma_tfx_cancel & (dma_read_check_data !== (dma_read_check_mask & dma_dout)) & ~puc_rst)
	    begin
	       $display("ERROR: DMA interface read check -- address: 0x%h -- read: 0x%h / expected: 0x%h (%t ns)", dma_read_check_addr, (dma_read_check_mask & dma_dout), dma_read_check_data, $time);
	       dma_rd_error = dma_rd_error+1;
	    end
	  dma_read_check_active =  1'b0;
       end
  end

//---------------------
// Generic read task
//---------------------
task dma_read;
   input  [15:0] addr;   // Address
   input  [15:0] data;   // Data to check against
   input         resp;   // Expected transfer response (0: Okay / 1: Error)
   input         check;  // Enable/disable read value check
   input         size;   // Access size (0: 8-bit / 1: 16-bit)

   begin
      // Perform read transfer
      dma_addr = addr[15:1];
      dma_en   = 1'b1;
      dma_we   = 2'b00;
      dma_din  = 16'h0000;
      @(posedge mclk or posedge dma_tfx_cancel);
      while(~dma_ready & ~dma_tfx_cancel) @(posedge mclk or posedge dma_tfx_cancel);
      dma_en   = 1'b0;
      dma_addr = 15'h0000;

      // Trigger read check
      dma_read_check_active =  check;
      dma_read_check_addr   =  addr;
      dma_read_check_data   =  data;
      dma_read_check_mask   =  size    ? 16'hFFFF :
                              (addr[0] ? 16'hFF00 : 16'h00FF);
      if (~dma_tfx_cancel) dma_cnt_rd = dma_cnt_rd+1;

      // Check transfer response
      if (~dma_tfx_cancel & (dma_resp != resp))
	begin
	   $display("ERROR: DMA interface read response check -- address: 0x%h -- response: %h / expected: %h (%t ns)", addr, dma_resp, resp, $time);
	   dma_rd_error = dma_rd_error+1;
	end
   end
endtask

//---------------------
// Read 16b task
//---------------------
task dma_read_16b;
   input  [15:0] addr;   // Address
   input  [15:0] data;   // Data to check against
   input         resp;   // Expected transfer response (0: Okay / 1: Error)

   begin
      dma_read(addr, data, resp, 1'b1, 1'b1);
   end
endtask

//---------------------
// Read 8b task
//---------------------
task dma_read_8b;
   input  [15:0] addr;   // Address
   input   [7:0] data;   // Data to check against
   input         resp;   // Expected transfer response (0: Okay / 1: Error)

   begin
      if (addr[0]) dma_read(addr, {data,  8'h00}, resp, 1'b1, 1'b0);
      else         dma_read(addr, {8'h00, data }, resp, 1'b1, 1'b0);
   end
endtask

//--------------------------------
// Read 16b value task (no check)
//--------------------------------
task dma_read_val_16b;
   input  [15:0] addr;   // Address
   input         resp;   // Expected transfer response (0: Okay / 1: Error)

   begin
      dma_read(addr, 16'h0000, resp, 1'b0, 1'b1);
   end
endtask


//============================================================================
// Ramdom DMA access process
//============================================================================

integer    dma_rand_wait;
integer    dma_rand_wait_disable;
reg        dma_rand_rdwr;
reg        dma_rand_if;
integer	   dma_rand_data;
reg  [6:0] dma_rand_addr;
reg [15:0] dma_rand_addr_full;
integer    dma_mem_ref_idx;
reg [15:0] dma_pmem_reference[0:127];
reg [15:0] dma_dmem_reference[0:127];
reg	   dma_verif_on;
reg	   dma_verif_verbose;

initial
  begin
     // Initialize
   `ifdef NO_DMA_VERIF
     dma_verif_on      = 0;
   `else
     `ifdef DMA_IF_EN
     dma_verif_on      = 1;
     `else
     dma_verif_on      = 0;
     `endif
   `endif
     dma_rand_wait_disable = 0;
     dma_verif_verbose = 0;
     dma_cnt_wr        = 0;
     dma_cnt_rd        = 0;
     dma_wr_error      = 0;
     dma_rd_error      = 0;
     #20;
     dma_rand_wait     = $urandom;
     for (dma_mem_ref_idx=0; dma_mem_ref_idx < 128; dma_mem_ref_idx=dma_mem_ref_idx+1)
       begin
	  dma_pmem_reference[dma_mem_ref_idx]             = $urandom;
	  dma_dmem_reference[dma_mem_ref_idx]		  = $urandom;
	  if (dma_verif_on && (`PMEM_SIZE>=4092) && (`DMEM_SIZE>=1024))
	    begin
	       pmem_0.mem[(`PMEM_SIZE-512)/2+dma_mem_ref_idx] = dma_pmem_reference[dma_mem_ref_idx];
	       dmem_0.mem[(`DMEM_SIZE-256)/2+dma_mem_ref_idx] = dma_dmem_reference[dma_mem_ref_idx];
	    end
       end

     // Wait for reset release
     repeat(1) @(posedge dco_clk);
     @(negedge puc_rst);

     // Perform random read/write 16b memory accesses
     if (dma_verif_on && (`PMEM_SIZE>=4092) && (`DMEM_SIZE>=1024))
       begin
	  forever
	    begin
	       // Randomize 1 or 0 wait states between accesses
	       // (1/3 proba of getting 1 wait state)
	       dma_rand_wait = dma_rand_wait_disable ? 0 : ($urandom_range(2,0)==0);
	       repeat(dma_rand_wait) @(posedge mclk);

	       // Randomize read/write accesses
	       // (1/3 proba of getting a read access)
	       dma_rand_rdwr = ($urandom_range(2,0)==0);

	       // Randomize address to be accessed (between 128 addresses)
	       dma_rand_addr = $urandom;

	       // Randomize access through PMEM or DMEM memories
	       dma_rand_if   = $urandom_range(1,0);

	       // Make sure the core is not in reset
	       while(puc_rst) @(posedge mclk);

	       if (dma_rand_rdwr)
		 begin
		    if (dma_rand_if)            // Read from Program Memory
		      begin
			 dma_rand_addr_full = 16'hFE00+dma_rand_addr*2;
			 if (dma_verif_verbose) $display("READ  DMA interface -- address: 0x%h -- expected data: 0x%h", dma_rand_addr_full, dma_pmem_reference[dma_rand_addr]);
			 dma_read_16b(dma_rand_addr_full,  dma_pmem_reference[dma_rand_addr], 1'b0);
		      end
		    else                        // Read from Data Memory
		      begin
			 dma_rand_addr_full = `PER_SIZE+`DMEM_SIZE-256+dma_rand_addr*2;
			 if (dma_verif_verbose) $display("READ  DMA interface -- address: 0x%h -- expected data: 0x%h", dma_rand_addr_full, dma_dmem_reference[dma_rand_addr]);
			 dma_read_16b(dma_rand_addr_full,  dma_dmem_reference[dma_rand_addr], 1'b0);
		      end
		 end
	       else
		 begin
		    dma_rand_data = $urandom;

		    if (dma_rand_if)            // Write to Program memory
		      begin
			 dma_rand_addr_full = 16'hFE00+dma_rand_addr*2;
			 if (dma_verif_verbose) $display("WRITE DMA interface -- address: 0x%h -- data: 0x%h", dma_rand_addr_full, dma_rand_data[15:0]);
			 dma_write_16b(dma_rand_addr_full, dma_rand_data[15:0], 1'b0);
			 dma_pmem_reference[dma_rand_addr] = dma_rand_data[15:0];
			 #1;
			 if (pmem_0.mem[(`PMEM_SIZE-512)/2+dma_rand_addr] !== dma_rand_data[15:0])
			   begin
			      $display("ERROR: DMA interface write -- address: 0x%h -- wrote: 0x%h / expected: 0x%h (%t ns)", dma_rand_addr_full, dma_rand_data[15:0], pmem_0.mem[(`PMEM_SIZE-512)/2+dma_rand_addr], $time);
			      dma_wr_error = dma_wr_error+1;
			   end
		      end
		    else                        // Write to Data Memory
		      begin
			 dma_rand_addr_full = `PER_SIZE+`DMEM_SIZE-256+dma_rand_addr*2;
			 if (dma_verif_verbose) $display("WRITE DMA interface -- address: 0x%h -- data: 0x%h", dma_rand_addr_full, dma_rand_data[15:0]);
			 dma_write_16b(dma_rand_addr_full, dma_rand_data[15:0], 1'b0);
			 dma_dmem_reference[dma_rand_addr] = dma_rand_data[15:0];
			 #1;
			 if (dmem_0.mem[(`DMEM_SIZE-256)/2+dma_rand_addr] !== dma_rand_data[15:0])
			   begin
			      $display("ERROR: DMA interface write -- address: 0x%h -- wrote: 0x%h / expected: 0x%h (%t ns)", dma_rand_addr_full, dma_rand_data[15:0], dmem_0.mem[(`DMEM_SIZE-256)/2+dma_rand_addr], $time);
			      dma_wr_error = dma_wr_error+1;
			   end
		      end
		 end
	    end
       end
  end


// Verilog stimulus
/*===========================================================================*/
/* Copyright (C) 2001 Authors                                                */
/*                                                                           */
/* This source file may be used and distributed without restriction provided */
/* that this copyright statement is not removed from the file and that any   */
/* derivative work contains the original copyright notice and the associated */
/* disclaimer.                                                               */
/*                                                                           */
/* This source file is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU Lesser General Public License as published  */
/* by the Free Software Foundation; either version 2.1 of the License, or    */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This source is distributed in the hope that it will be useful, but WITHOUT*/
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     */
/* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public       */
/* License for more details.                                                 */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with this source; if not, write to the Free Software Foundation,    */
/* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA        */
/*                                                                           */
/*===========================================================================*/
/*                   TWO-OPERAND ARITHMETIC: MOV INSTRUCTION                 */
/*---------------------------------------------------------------------------*/
/* Test the MOV instruction with all addressing modes                        */
/*                                                                           */
/* Author(s):                                                                */
/*             - Olivier Girard,    olgirard@gmail.com                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/* $Rev: 134 $                                                                */
/* $LastChangedBy: olivier.girard $                                          */
/* $LastChangedDate: 2012-03-22 21:31:06 +0100 (Thu, 22 Mar 2012) $          */
/*===========================================================================*/

initial
   begin
      $display(" ===============================================");
      $display("|                 START SIMULATION              |");
      $display(" ===============================================");
      repeat(5) @(posedge mclk);
      stimulus_done = 0;

      // Check reset values
      //--------------------------------------------------------
      if (r2 !==16'h0000) tb_error("R2  reset value");
      if (r3 !==16'h0000) tb_error("R3  reset value");
      if (r4 !==16'h0000) tb_error("R4  reset value");
      if (r5 !==16'h0000) tb_error("R5  reset value");
      if (r6 !==16'h0000) tb_error("R6  reset value");
      if (r7 !==16'h0000) tb_error("R7  reset value");
      if (r8 !==16'h0000) tb_error("R8  reset value");
      if (r9 !==16'h0000) tb_error("R9  reset value");
      if (r10!==16'h0000) tb_error("R10 reset value");
      if (r11!==16'h0000) tb_error("R11 reset value");
      if (r12!==16'h0000) tb_error("R12 reset value");
      if (r13!==16'h0000) tb_error("R13 reset value");
      if (r14!==16'h0000) tb_error("R14 reset value");
      if (r15!==16'h0000) tb_error("R15 reset value");


      // Make sure initialization worked fine
      //--------------------------------------------------------
      @(r15==16'h1000);

      if (r2 !==16'h0002) tb_error("R2  initialization");
      if (r3 !==16'h3333) tb_error("R3  initialization");
      if (r4 !==16'h4444) tb_error("R4  initialization");
      if (r5 !==16'h5555) tb_error("R5  initialization");
      if (r6 !==16'h6666) tb_error("R6  initialization");
      if (r7 !==16'h7777) tb_error("R7  initialization");
      if (r8 !==16'h8888) tb_error("R8  initialization");
      if (r9 !==16'h9999) tb_error("R9  initialization");
      if (r10!==16'haaaa) tb_error("R10 initialization");
      if (r11!==16'hbbbb) tb_error("R11 initialization");
      if (r12!==16'hcccc) tb_error("R12 initialization");
      if (r13!==16'hdddd) tb_error("R13 initialization");
      if (r14!==16'heeee) tb_error("R14 initialization");

      // MOV: Check when source is Rn
      //--------------------------------------------------------
      @(r15==16'h2000);

      if (r3     !==16'h4444) tb_error("====== MOV Rn Rm    =====");
      if (r4     ===16'h0000) tb_error("====== MOV Rn PC    =====");
      if (mem210 !==16'h1234) tb_error("====== MOV Rn x(Rm) =====");
      if (mem200 !==16'h5678) tb_error("====== MOV Rn EDE   =====");
      if (mem204 !==16'h9abc) tb_error("====== MOV Rn &EDE  =====");

      // MOV: Check when source is @Rn
      //--------------------------------------------------------
      @(r15==16'h3000);

      if (r3     !==16'h1234) tb_error("====== MOV @Rn Rm    =====");
      if (r4     ===16'h0000) tb_error("====== MOV @Rn PC    =====");
      if (mem210 !==16'h9abc) tb_error("====== MOV @Rn x(Rm) =====");
      if (mem200 !==16'hfedc) tb_error("====== MOV @Rn EDE   =====");
      if (mem204 !==16'hf1d2) tb_error("====== MOV @Rn &EDE  =====");


      // MOV: Check when source is @Rn+
      //--------------------------------------------------------
      @(r15==16'h4000);

      if (r4     !==(`PER_SIZE+16'h0002)) tb_error("====== MOV @Rn+ Rm    =====");
      if (r5     !==16'h1111)             tb_error("====== MOV @Rn+ Rm    =====");

      if (r6     !==(`PER_SIZE+16'h0006)) tb_error("====== MOV @Rn+ PC    =====");

      if (r7     !==(`PER_SIZE+16'h0018)) tb_error("====== MOV @Rn+ x(Rm) =====");
      if (mem220 !==16'hdef0)             tb_error("====== MOV @Rn+ x(Rm) =====");

      if (r8     !==(`PER_SIZE+16'h0014)) tb_error("====== MOV @Rn+ EDE   =====");
      if (mem200 !==16'h5678)             tb_error("====== MOV @Rn+ EDE   =====");

      if (r9     !==(`PER_SIZE+16'h0012)) tb_error("====== MOV @Rn+ &EDE  =====");
      if (mem214 !==16'h1234)             tb_error("====== MOV @Rn+ &EDE  =====");


      // MOV: Check when source is #N
      //--------------------------------------------------------
      @(r15==16'h5000);

      if (r4     !==16'h3210) tb_error("====== MOV #N  Rm    =====");
      if (r5     ===16'h0000) tb_error("====== MOV #N  PC    =====");
      if (mem230 !==16'h5a5a) tb_error("====== MOV #N  x(Rm) =====");
      if (mem210 !==16'h1a2b) tb_error("====== MOV #N  EDE   =====");
      if (mem206 !==16'h3c4d) tb_error("====== MOV #N  &EDE  =====");


      // MOV: Check when source is x(Rn)
      //--------------------------------------------------------
      @(r15==16'h6000);

      if (r5     !==16'h8347) tb_error("====== MOV x(Rn) Rm    =====");
      if (r6     ===16'h0000) tb_error("====== MOV x(Rn) PC    =====");
      if (mem214 !==16'h4231) tb_error("====== MOV x(Rn) x(Rm) =====");
      if (mem220 !==16'h7238) tb_error("====== MOV x(Rn) EDE   =====");
      if (mem208 !==16'h98b2) tb_error("====== MOV x(Rn) &EDE  =====");

      // MOV: Check when source is EDE
      //--------------------------------------------------------
      @(r15==16'h7000);

      if (r4     !==16'hc3d6) tb_error("====== MOV EDE Rm    =====");
      if (r6     ===16'h0000) tb_error("====== MOV EDE PC    =====");
      if (mem214 !==16'hf712) tb_error("====== MOV EDE x(Rm) =====");
      if (mem216 !==16'hb3a9) tb_error("====== MOV EDE EDE   =====");
      if (mem212 !==16'h837a) tb_error("====== MOV EDE &EDE  =====");


      // MOV: Check when source is &EDE
      //--------------------------------------------------------
      @(r15==16'h8000);

      if (r4     !==16'h23d4) tb_error("====== MOV &EDE Rm    =====");
      if (r6     ===16'h0000) tb_error("====== MOV &EDE PC    =====");
      if (mem214 !==16'h481c) tb_error("====== MOV &EDE x(Rm) =====");
      if (mem218 !==16'h5c1f) tb_error("====== MOV &EDE EDE   =====");
      if (mem202 !==16'hc16e) tb_error("====== MOV &EDE &EDE  =====");


      // MOV: Check when source is CONST
      //--------------------------------------------------------
      @(r15==16'h9000);

      if (r4     !==16'h0000) tb_error("====== MOV #+0 Rm    =====");
      if (r5     !==16'h0001) tb_error("====== MOV #+1 Rm    =====");
      if (r6     !==16'h0002) tb_error("====== MOV #+2 Rm    =====");
      if (r7     !==16'h0004) tb_error("====== MOV #+4 Rm    =====");
      if (r8     !==16'h0008) tb_error("====== MOV #+8 Rm    =====");
      if (r9     !==16'hffff) tb_error("====== MOV #-1 Rm    =====");

      if (mem210 !==16'h0000) tb_error("====== MOV #+0 x(Rm) =====");
      if (mem212 !==16'h0001) tb_error("====== MOV #+1 x(Rm) =====");
      if (mem214 !==16'h0002) tb_error("====== MOV #+2 x(Rm) =====");
      if (mem216 !==16'h0004) tb_error("====== MOV #+4 x(Rm) =====");
      if (mem218 !==16'h0008) tb_error("====== MOV #+8 x(Rm) =====");
      if (mem21A !==16'hffff) tb_error("====== MOV #-1 x(Rm) =====");

      if (mem220 !==16'h0000) tb_error("====== MOV #+0 EDE =====");
      if (mem222 !==16'h0001) tb_error("====== MOV #+1 EDE =====");
      if (mem224 !==16'h0002) tb_error("====== MOV #+2 EDE =====");
      if (mem226 !==16'h0004) tb_error("====== MOV #+4 EDE =====");
      if (mem228 !==16'h0008) tb_error("====== MOV #+8 EDE =====");
      if (mem22A !==16'hffff) tb_error("====== MOV #-1 EDE =====");

      if (mem230 !==16'h0000) tb_error("====== MOV #+0 &EDE =====");
      if (mem232 !==16'h0001) tb_error("====== MOV #+1 &EDE =====");
      if (mem234 !==16'h0002) tb_error("====== MOV #+2 &EDE =====");
      if (mem236 !==16'h0004) tb_error("====== MOV #+4 &EDE =====");
      if (mem238 !==16'h0008) tb_error("====== MOV #+8 &EDE =====");
      if (mem23A !==16'hffff) tb_error("====== MOV #-1 &EDE =====");

//    ---------------- TEST WHEN SOURCE IS CONSTANT IN BYTE MODE ------ */
//    #
//    # NOTE: The following section would not fit in the smallest ROM
//    #       configuration for the "two-op_mov-b.v" pattern.
//    #       It is therefore executed here.
//    #
      @(r15==16'hA000);

      if (mem250 !==16'h4400) tb_error("====== MOV.B #+0 &EDE =====");
      if (mem252 !==16'h5501) tb_error("====== MOV.B #+1 &EDE =====");
      if (mem254 !==16'h6602) tb_error("====== MOV.B #+2 &EDE =====");
      if (mem256 !==16'h7704) tb_error("====== MOV.B #+4 &EDE =====");
      if (mem258 !==16'h3508) tb_error("====== MOV.B #+8 &EDE =====");
      if (mem25A !==16'h99ff) tb_error("====== MOV.B #-1 &EDE =====");
      if (mem25C !==16'h00aa) tb_error("====== MOV.B #+0 &EDE =====");
      if (mem25E !==16'h01bb) tb_error("====== MOV.B #+1 &EDE =====");
      if (mem260 !==16'h02cc) tb_error("====== MOV.B #+2 &EDE =====");
      if (mem262 !==16'h04dd) tb_error("====== MOV.B #+4 &EDE =====");
      if (mem264 !==16'h08ee) tb_error("====== MOV.B #+8 &EDE =====");
      if (mem266 !==16'hff33) tb_error("====== MOV.B #-1 &EDE =====");


      stimulus_done = 1;
   end



//
// Initialize Memory
//------------------------------
initial
  begin
     // Initialize data memory
     for (tb_idx=0; tb_idx < `DMEM_SIZE/2; tb_idx=tb_idx+1)
       dmem_0.mem[tb_idx] = 16'h0000;

     // Initialize program memory
     #10 $readmemh("./pmem.mem", pmem_0.mem);
  end


//
// Generate Clock & Reset
//------------------------------
initial
  begin
     dco_clk          = 1'b0;
     dco_local_enable = 1'b0;
     forever
       begin
          #25;   // 20 MHz
          dco_local_enable = (dco_enable===1) ? dco_enable : (dco_wkup===1);
          if (dco_local_enable | scan_mode)
            dco_clk = ~dco_clk;
       end
  end

initial
  begin
     lfxt_clk          = 1'b0;
     lfxt_local_enable = 1'b0;
     forever
       begin
          #763;  // 655 kHz
          lfxt_local_enable = (lfxt_enable===1) ? lfxt_enable : (lfxt_wkup===1);
          if (lfxt_local_enable)
            lfxt_clk = ~lfxt_clk;
       end
  end

initial
  begin
     reset_n       = 1'b1;
     #93;
     reset_n       = 1'b0;
     #593;
     reset_n       = 1'b1;
  end

initial
  begin
     tmp_seed                = -1981345606;//`SEED;
     tmp_seed                = $urandom(tmp_seed);
     error                   = 0;
     stimulus_done           = 1;
     irq                     = {`IRQ_NR-2{1'b0}};
     nmi                     = 1'b0;
     wkup                    = 14'h0000;
     dma_addr                = 15'h0000;
     dma_din                 = 16'h0000;
     dma_en                  = 1'b0;
     dma_priority            = 1'b0;
     dma_we                  = 2'b00;
     dma_wkup                = 1'b0;
     dma_tfx_cancel          = 1'b0;
     cpu_en                  = 1'b1;
     dbg_en                  = 1'b0;
     dbg_uart_rxd_sel        = 1'b0;
     dbg_uart_rxd_dly        = 1'b1;
     dbg_uart_rxd_pre        = 1'b1;
     dbg_uart_rxd_meta       = 1'b0;
     dbg_uart_buf            = 16'h0000;
     dbg_uart_rx_busy        = 1'b0;
     dbg_uart_tx_busy        = 1'b0;
     dbg_scl_master_sel      = 1'b0;
     dbg_scl_master_dly      = 1'b1;
     dbg_scl_master_pre      = 1'b1;
     dbg_scl_master_meta     = 1'b0;
     dbg_sda_master_out_sel  = 1'b0;
     dbg_sda_master_out_dly  = 1'b1;
     dbg_sda_master_out_pre  = 1'b1;
     dbg_sda_master_out_meta = 1'b0;
     dbg_i2c_string          = "";
     p1_din                  = 8'h00;
     p2_din                  = 8'h00;
     p3_din                  = 8'h00;
     p4_din                  = 8'h00;
     p5_din                  = 8'h00;
     p6_din                  = 8'h00;
     inclk                   = 1'b0;
     taclk                   = 1'b0;
     ta_cci0a                = 1'b0;
     ta_cci0b                = 1'b0;
     ta_cci1a                = 1'b0;
     ta_cci1b                = 1'b0;
     ta_cci2a                = 1'b0;
     ta_cci2b                = 1'b0;
     scan_enable             = 1'b0;
     scan_mode               = 1'b0;
  end


//
// Program Memory
//----------------------------------

ram #(`PMEM_MSB, `PMEM_SIZE) pmem_0 (

// OUTPUTs
    .ram_dout          (pmem_dout),            // Program Memory data output

// INPUTs
    .ram_addr          (pmem_addr),            // Program Memory address
    .ram_cen           (pmem_cen),             // Program Memory chip enable (low active)
    .ram_clk           (mclk),                 // Program Memory clock
    .ram_din           (pmem_din),             // Program Memory data input
    .ram_wen           (pmem_wen)              // Program Memory write enable (low active)
);


//
// Data Memory
//----------------------------------

ram #(`DMEM_MSB, `DMEM_SIZE) dmem_0 (

// OUTPUTs
    .ram_dout          (dmem_dout),            // Data Memory data output

// INPUTs
    .ram_addr          (dmem_addr),            // Data Memory address
    .ram_cen           (dmem_cen),             // Data Memory chip enable (low active)
    .ram_clk           (mclk),                 // Data Memory clock
    .ram_din           (dmem_din),             // Data Memory data input
    .ram_wen           (dmem_wen)              // Data Memory write enable (low active)
);


//
// openMSP430 Instance
//----------------------------------

openMSP430 dut (

// OUTPUTs
    .aclk              (aclk),                 // ASIC ONLY: ACLK
    .aclk_en           (aclk_en),              // FPGA ONLY: ACLK enable
    .dbg_freeze        (dbg_freeze),           // Freeze peripherals
    .dbg_i2c_sda_out   (dbg_sda_slave_out),    // Debug interface: I2C SDA OUT
    .dbg_uart_txd      (dbg_uart_txd),         // Debug interface: UART TXD
    .dco_enable        (dco_enable),           // ASIC ONLY: Fast oscillator enable
    .dco_wkup          (dco_wkup),             // ASIC ONLY: Fast oscillator wake-up (asynchronous)
    .dmem_addr         (dmem_addr),            // Data Memory address
    .dmem_cen          (dmem_cen),             // Data Memory chip enable (low active)
    .dmem_din          (dmem_din),             // Data Memory data input
    .dmem_wen          (dmem_wen),             // Data Memory write byte enable (low active)
    .irq_acc           (irq_acc),              // Interrupt request accepted (one-hot signal)
    .lfxt_enable       (lfxt_enable),          // ASIC ONLY: Low frequency oscillator enable
    .lfxt_wkup         (lfxt_wkup),            // ASIC ONLY: Low frequency oscillator wake-up (asynchronous)
    .mclk              (mclk),                 // Main system clock
    .dma_dout          (dma_dout),             // Direct Memory Access data output
    .dma_ready         (dma_ready),            // Direct Memory Access is complete
    .dma_resp          (dma_resp),             // Direct Memory Access response (0:Okay / 1:Error)
    .per_addr          (per_addr),             // Peripheral address
    .per_din           (per_din),              // Peripheral data input
    .per_en            (per_en),               // Peripheral enable (high active)
    .per_we            (per_we),               // Peripheral write byte enable (high active)
    .pmem_addr         (pmem_addr),            // Program Memory address
    .pmem_cen          (pmem_cen),             // Program Memory chip enable (low active)
    .pmem_din          (pmem_din),             // Program Memory data input (optional)
    .pmem_wen          (pmem_wen),             // Program Memory write byte enable (low active) (optional)
    .puc_rst           (puc_rst),              // Main system reset
    .smclk             (smclk),                // ASIC ONLY: SMCLK
    .smclk_en          (smclk_en),             // FPGA ONLY: SMCLK enable

// INPUTs
    .cpu_en            (cpu_en),               // Enable CPU code execution (asynchronous)
    .dbg_en            (dbg_en),               // Debug interface enable (asynchronous)
    .dbg_i2c_addr      (I2C_ADDR),             // Debug interface: I2C Address
    .dbg_i2c_broadcast (I2C_BROADCAST),        // Debug interface: I2C Broadcast Address (for multicore systems)
    .dbg_i2c_scl       (dbg_scl_slave),        // Debug interface: I2C SCL
    .dbg_i2c_sda_in    (dbg_sda_slave_in),     // Debug interface: I2C SDA IN
    .dbg_uart_rxd      (dbg_uart_rxd),         // Debug interface: UART RXD (asynchronous)
    .dco_clk           (dco_clk),              // Fast oscillator (fast clock)
    .dmem_dout         (dmem_dout),            // Data Memory data output
    .irq               (irq_in),               // Maskable interrupts
    .lfxt_clk          (lfxt_clk),             // Low frequency oscillator (typ 32kHz)
    .dma_addr          (dma_addr),             // Direct Memory Access address
    .dma_din           (dma_din),              // Direct Memory Access data input
    .dma_en            (dma_en),               // Direct Memory Access enable (high active)
    .dma_priority      (dma_priority),         // Direct Memory Access priority (0:low / 1:high)
    .dma_we            (dma_we),               // Direct Memory Access write byte enable (high active)
    .dma_wkup          (dma_wkup),             // ASIC ONLY: DMA Sub-System Wake-up (asynchronous and non-glitchy)
    .nmi               (nmi),                  // Non-maskable interrupt (asynchronous)
    .per_dout          (per_dout),             // Peripheral data output
    .pmem_dout         (pmem_dout),            // Program Memory data output
    .reset_n           (reset_n),              // Reset Pin (low active, asynchronous)
    .scan_enable       (scan_enable),          // ASIC ONLY: Scan enable (active during scan shifting)
    .scan_mode         (scan_mode),            // ASIC ONLY: Scan mode
    .wkup              (|wkup_in)              // ASIC ONLY: System Wake-up (asynchronous)
);

//
// Digital I/O
//----------------------------------

`ifdef CVER
omsp_gpio #(1,
            1,
            1,
            1,
            1,
            1)         gpio_0 (
`else
omsp_gpio #(.P1_EN(1),
            .P2_EN(1),
            .P3_EN(1),
            .P4_EN(1),
            .P5_EN(1),
            .P6_EN(1)) gpio_0 (
`endif

// OUTPUTs
    .irq_port1         (irq_port1),            // Port 1 interrupt
    .irq_port2         (irq_port2),            // Port 2 interrupt
    .p1_dout           (p1_dout),              // Port 1 data output
    .p1_dout_en        (p1_dout_en),           // Port 1 data output enable
    .p1_sel            (p1_sel),               // Port 1 function select
    .p2_dout           (p2_dout),              // Port 2 data output
    .p2_dout_en        (p2_dout_en),           // Port 2 data output enable
    .p2_sel            (p2_sel),               // Port 2 function select
    .p3_dout           (p3_dout),              // Port 3 data output
    .p3_dout_en        (p3_dout_en),           // Port 3 data output enable
    .p3_sel            (p3_sel),               // Port 3 function select
    .p4_dout           (p4_dout),              // Port 4 data output
    .p4_dout_en        (p4_dout_en),           // Port 4 data output enable
    .p4_sel            (p4_sel),               // Port 4 function select
    .p5_dout           (p5_dout),              // Port 5 data output
    .p5_dout_en        (p5_dout_en),           // Port 5 data output enable
    .p5_sel            (p5_sel),               // Port 5 function select
    .p6_dout           (p6_dout),              // Port 6 data output
    .p6_dout_en        (p6_dout_en),           // Port 6 data output enable
    .p6_sel            (p6_sel),               // Port 6 function select
    .per_dout          (per_dout_dio),         // Peripheral data output

// INPUTs
    .mclk              (mclk),                 // Main system clock
    .p1_din            (p1_din),               // Port 1 data input
    .p2_din            (p2_din),               // Port 2 data input
    .p3_din            (p3_din),               // Port 3 data input
    .p4_din            (p4_din),               // Port 4 data input
    .p5_din            (p5_din),               // Port 5 data input
    .p6_din            (p6_din),               // Port 6 data input
    .per_addr          (per_addr),             // Peripheral address
    .per_din           (per_din),              // Peripheral data input
    .per_en            (per_en),               // Peripheral enable (high active)
    .per_we            (per_we),               // Peripheral write enable (high active)
    .puc_rst           (puc_rst)               // Main system reset
);

//
// Timers
//----------------------------------

omsp_timerA timerA_0 (

// OUTPUTs
    .irq_ta0           (irq_ta0),              // Timer A interrupt: TACCR0
    .irq_ta1           (irq_ta1),              // Timer A interrupt: TAIV, TACCR1, TACCR2
    .per_dout          (per_dout_timerA),      // Peripheral data output
    .ta_out0           (ta_out0),              // Timer A output 0
    .ta_out0_en        (ta_out0_en),           // Timer A output 0 enable
    .ta_out1           (ta_out1),              // Timer A output 1
    .ta_out1_en        (ta_out1_en),           // Timer A output 1 enable
    .ta_out2           (ta_out2),              // Timer A output 2
    .ta_out2_en        (ta_out2_en),           // Timer A output 2 enable

// INPUTs
    .aclk_en           (aclk_en),              // ACLK enable (from CPU)
    .dbg_freeze        (dbg_freeze),           // Freeze Timer A counter
    .inclk             (inclk),                // INCLK external timer clock (SLOW)
    .irq_ta0_acc       (irq_acc[`IRQ_NR-7]),   // Interrupt request TACCR0 accepted
    .mclk              (mclk),                 // Main system clock
    .per_addr          (per_addr),             // Peripheral address
    .per_din           (per_din),              // Peripheral data input
    .per_en            (per_en),               // Peripheral enable (high active)
    .per_we            (per_we),               // Peripheral write enable (high active)
    .puc_rst           (puc_rst),              // Main system reset
    .smclk_en          (smclk_en),             // SMCLK enable (from CPU)
    .ta_cci0a          (ta_cci0a),             // Timer A compare 0 input A
    .ta_cci0b          (ta_cci0b),             // Timer A compare 0 input B
    .ta_cci1a          (ta_cci1a),             // Timer A compare 1 input A
    .ta_cci1b          (ta_cci1b),             // Timer A compare 1 input B
    .ta_cci2a          (ta_cci2a),             // Timer A compare 2 input A
    .ta_cci2b          (ta_cci2b),             // Timer A compare 2 input B
    .taclk             (taclk)                 // TACLK external timer clock (SLOW)
);

//
// Peripheral templates
//----------------------------------

template_periph_8b template_periph_8b_0 (

// OUTPUTs
    .per_dout          (per_dout_temp_8b),     // Peripheral data output

// INPUTs
    .mclk              (mclk),                 // Main system clock
    .per_addr          (per_addr),             // Peripheral address
    .per_din           (per_din),              // Peripheral data input
    .per_en            (per_en),               // Peripheral enable (high active)
    .per_we            (per_we),               // Peripheral write enable (high active)
    .puc_rst           (puc_rst)               // Main system reset
);

`ifdef CVER
template_periph_16b #(15'h0190)                                        template_periph_16b_0 (
`else
template_periph_16b #(.BASE_ADDR((15'd`PER_SIZE-15'h0070) & 15'h7ff8)) template_periph_16b_0 (
`endif
// OUTPUTs
    .per_dout          (per_dout_temp_16b),    // Peripheral data output

// INPUTs
    .mclk              (mclk),                 // Main system clock
    .per_addr          (per_addr),             // Peripheral address
    .per_din           (per_din),              // Peripheral data input
    .per_en            (per_en),               // Peripheral enable (high active)
    .per_we            (per_we),               // Peripheral write enable (high active)
    .puc_rst           (puc_rst)               // Main system reset
);


//
// Combine peripheral data bus
//----------------------------------

assign per_dout = per_dout_dio       |
                  per_dout_timerA    |
                  per_dout_temp_8b   |
                  per_dout_temp_16b;


//
// Map peripheral interrupts & wakeups
//----------------------------------------

assign irq_in  = irq  | {1'b0,                 // Vector 13  (0xFFFA)
                         1'b0,                 // Vector 12  (0xFFF8)
                         1'b0,                 // Vector 11  (0xFFF6)
                         1'b0,                 // Vector 10  (0xFFF4) - Watchdog -
                         irq_ta0,              // Vector  9  (0xFFF2)
                         irq_ta1,              // Vector  8  (0xFFF0)
                         1'b0,                 // Vector  7  (0xFFEE)
                         1'b0,                 // Vector  6  (0xFFEC)
                         1'b0,                 // Vector  5  (0xFFEA)
                         1'b0,                 // Vector  4  (0xFFE8)
                         irq_port2,            // Vector  3  (0xFFE6)
                         irq_port1,            // Vector  2  (0xFFE4)
                         1'b0,                 // Vector  1  (0xFFE2)
                         {`IRQ_NR-15{1'b0}}};  // Vector  0  (0xFFE0)

assign wkup_in = wkup | {1'b0,                 // Vector 13  (0xFFFA)
                         1'b0,                 // Vector 12  (0xFFF8)
                         1'b0,                 // Vector 11  (0xFFF6)
                         1'b0,                 // Vector 10  (0xFFF4) - Watchdog -
                         1'b0,                 // Vector  9  (0xFFF2)
                         1'b0,                 // Vector  8  (0xFFF0)
                         1'b0,                 // Vector  7  (0xFFEE)
                         1'b0,                 // Vector  6  (0xFFEC)
                         1'b0,                 // Vector  5  (0xFFEA)
                         1'b0,                 // Vector  4  (0xFFE8)
                         1'b0,                 // Vector  3  (0xFFE6)
                         1'b0,                 // Vector  2  (0xFFE4)
                         1'b0,                 // Vector  1  (0xFFE2)
                         1'b0};                // Vector  0  (0xFFE0)


//
// I2C serial debug interface
//----------------------------------

// I2C Bus
//.........................
pullup dbg_scl_inst (dbg_scl);
pullup dbg_sda_inst (dbg_sda);

// I2C Slave (openMSP430)
//.........................
io_cell scl_slave_inst (
    .pad               (dbg_scl),              // I/O pad
    .data_in           (dbg_scl_slave),        // Input
    .data_out_en       (1'b0),                 // Output enable
    .data_out          (1'b0)                  // Output
);

io_cell sda_slave_inst (
    .pad               (dbg_sda),              // I/O pad
    .data_in           (dbg_sda_slave_in),     // Input
    .data_out_en       (!dbg_sda_slave_out),   // Output enable
    .data_out          (1'b0)                  // Output
);

// I2C Master (Debugger)
//.........................
io_cell scl_master_inst (
    .pad               (dbg_scl),              // I/O pad
    .data_in           (),                     // Input
    .data_out_en       (!dbg_scl_master),      // Output enable
    .data_out          (1'b0)                  // Output
);

io_cell sda_master_inst (
    .pad               (dbg_sda),              // I/O pad
    .data_in           (dbg_sda_master_in),    // Input
    .data_out_en       (!dbg_sda_master_out),  // Output enable
    .data_out          (1'b0)                  // Output
);


//
// Debug utility signals
//----------------------------------------
msp_debug msp_debug_0 (

// OUTPUTs
    .e_state           (e_state),              // Execution state
    .i_state           (i_state),              // Instruction fetch state
    .inst_cycle        (inst_cycle),           // Cycle number within current instruction
    .inst_full         (inst_full),            // Currently executed instruction (full version)
    .inst_number       (inst_number),          // Instruction number since last system reset
    .inst_pc           (inst_pc),              // Instruction Program counter
    .inst_short        (inst_short),           // Currently executed instruction (short version)

// INPUTs
    .mclk              (mclk),                 // Main system clock
    .puc_rst           (puc_rst)               // Main system reset
);


//
// Generate Waveform
//----------------------------------------
initial
  begin
   `ifdef NODUMP
   `else
     `ifdef VPD_FILE
        $vcdplusfile("tb_openMSP430.vpd");
        $vcdpluson();
     `else
       `ifdef TRN_FILE
          $recordfile ("tb_openMSP430.trn");
          $recordvars;
       `else
          $dumpfile("tb_openMSP430.vcd");
          $dumpvars(0, tb_openMSP430);
       `endif
     `endif
   `endif
  end

//
// End of simulation
//----------------------------------------

initial // Timeout
  begin
   `ifdef NO_TIMEOUT
   `else
     `ifdef VERY_LONG_TIMEOUT
       #500000000;
     `else
     `ifdef LONG_TIMEOUT
       #5000000;
     `else
       #500000;
     `endif
     `endif
       $display(" ===============================================");
       $display("|               SIMULATION FAILED               |");
       $display("|              (simulation Timeout)             |");
       $display(" ===============================================");
       $display("");
       tb_extra_report;
       $finish;
   `endif
  end

initial // Normal end of test
  begin
     @(negedge stimulus_done);
     wait(inst_pc=='hffff);

     $display(" ===============================================");
     if ((dma_rd_error!=0) || (dma_wr_error!=0))
       begin
          $display("|               SIMULATION FAILED               |");
          $display("|           (some DMA transfer failed)          |");
       end
     else if (error!=0)
       begin
          $display("|               SIMULATION FAILED               |");
          $display("|     (some verilog stimulus checks failed)     |");
       end
     else if (~stimulus_done)
       begin
          $display("|               SIMULATION FAILED               |");
          $display("|     (the verilog stimulus didn't complete)    |");
       end
     else
       begin
          $display("|               SIMULATION PASSED               |");
       end
     $display(" ===============================================");
     $display("");
     tb_extra_report;
     $finish;
  end


//
// Tasks Definition
//------------------------------

   task tb_error;
      input [65*8:0] error_string;
      begin
         $display("ERROR: %s %t", error_string, $time);
         error = error+1;
      end
   endtask

   task tb_extra_report;
      begin
         $display("DMA REPORT: Total Accesses: %-d Total RD: %-d Total WR: %-d", dma_cnt_rd+dma_cnt_wr,     dma_cnt_rd,   dma_cnt_wr);
         $display("            Total Errors:   %-d Error RD: %-d Error WR: %-d", dma_rd_error+dma_wr_error, dma_rd_error, dma_wr_error);
         if (!((`PMEM_SIZE>=4092) && (`DMEM_SIZE>=1024)))
           begin
	      $display("");
              $display("Note: DMA if verification disabled (PMEM must be 4kB or bigger, DMEM must be 1kB or bigger)");
           end
         $display("");
         $display("SIMULATION SEED: %d", -1981345606);//`SEED);
         $display("");
      end
   endtask

   task tb_skip_finish;
      input [65*8-1:0] skip_string;
      begin
         $display(" ===============================================");
         $display("|               SIMULATION SKIPPED              |");
         $display("%s", skip_string);
         $display(" ===============================================");
         $display("");
         tb_extra_report;
         $finish;
      end
   endtask
 initial
      begin
         $dumpfile(`DUMP_FILE_NAME);
        $dumpvars(1);		//writing the vcd file
      end
endmodule

//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: openMSP430_defines.v
//
// *Module Description:
//                      openMSP430 Configuration file
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 205 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2015-07-15 22:59:52 +0200 (Wed, 15 Jul 2015) $
//----------------------------------------------------------------------------
`define OMSP_NO_INCLUDE
`ifdef OMSP_NO_INCLUDE
`else
`include "openMSP430_undefines.v"
`endif

//============================================================================
//============================================================================
// BASIC SYSTEM CONFIGURATION
//============================================================================
//============================================================================
//
// Note: the sum of program, data and peripheral memory spaces must not
//      exceed 64 kB
//

// Program Memory Size:
//                     Uncomment the required memory size
//-------------------------------------------------------
//`define PMEM_SIZE_CUSTOM
//`define PMEM_SIZE_59_KB
//`define PMEM_SIZE_55_KB
//`define PMEM_SIZE_54_KB
//`define PMEM_SIZE_51_KB
//`define PMEM_SIZE_48_KB
//`define PMEM_SIZE_41_KB
//`define PMEM_SIZE_32_KB
//`define PMEM_SIZE_24_KB
//`define PMEM_SIZE_16_KB
//`define PMEM_SIZE_12_KB
//`define PMEM_SIZE_8_KB
`define PMEM_SIZE_4_KB
//`define PMEM_SIZE_2_KB
//`define PMEM_SIZE_1_KB


// Data Memory Size:
//                     Uncomment the required memory size
//-------------------------------------------------------
//`define DMEM_SIZE_CUSTOM
//`define DMEM_SIZE_32_KB
//`define DMEM_SIZE_24_KB
//`define DMEM_SIZE_16_KB
//`define DMEM_SIZE_10_KB
//`define DMEM_SIZE_8_KB
//`define DMEM_SIZE_5_KB
//`define DMEM_SIZE_4_KB
//`define DMEM_SIZE_2p5_KB
//`define DMEM_SIZE_2_KB
`define DMEM_SIZE_1_KB
//`define DMEM_SIZE_512_B
//`define DMEM_SIZE_256_B
//`define DMEM_SIZE_128_B


// Include/Exclude Hardware Multiplier
`define MULTIPLIER


// Include/Exclude Serial Debug interface
`define DBG_EN


//============================================================================
//============================================================================
// ADVANCED SYSTEM CONFIGURATION (FOR EXPERIENCED USERS)
//============================================================================
//============================================================================

//-------------------------------------------------------
// Custom user version number
//-------------------------------------------------------
// This 5 bit field can be freely used in order to allow
// custom identification of the system through the debug
// interface.
// (see CPU_ID.USER_VERSION field in the documentation)
//-------------------------------------------------------
`define USER_VERSION 5'b00000


//-------------------------------------------------------
// Include/Exclude Watchdog timer
//-------------------------------------------------------
// When excluded, the following functionality will be
// lost:
//        - Watchog (both interval and watchdog modes)
//        - NMI interrupt edge selection
//        - Possibility to generate a software PUC reset
//-------------------------------------------------------
`define WATCHDOG


//-------------------------------------------------------
// Include/Exclude DMA interface support
//-------------------------------------------------------
`define DMA_IF_EN


//-------------------------------------------------------
// Include/Exclude Non-Maskable-Interrupt support
//-------------------------------------------------------
`define NMI


//-------------------------------------------------------
// Number of available IRQs
//-------------------------------------------------------
// Indicates the number of interrupt vectors supported
// (16, 32 or 64).
//-------------------------------------------------------
`define IRQ_16
//`define IRQ_32
//`define IRQ_64


//-------------------------------------------------------
// Input synchronizers
//-------------------------------------------------------
// In some cases, the asynchronous input ports might
// already be synchronized externally.
// If an extensive CDC design review showed that this
// is really the case,  the individual synchronizers
// can be disabled with the following defines.
//
// Notes:
//        - all three signals are all sampled in the MCLK domain
//
//        - the dbg_en signal reset the debug interface
//         when 0. Therefore make sure it is glitch free.
//
//-------------------------------------------------------
`define SYNC_NMI
//`define SYNC_CPU_EN
//`define SYNC_DBG_EN


//-------------------------------------------------------
// Peripheral Memory Space:
//-------------------------------------------------------
// The original MSP430 architecture map the peripherals
// from 0x0000 to 0x01FF (i.e. 512B of the memory space).
// The following defines allow you to expand this space
// up to 32 kB (i.e. from 0x0000 to 0x7fff).
// As a consequence, the data memory mapping will be
// shifted up and a custom linker script will therefore
// be required by the GCC compiler.
//-------------------------------------------------------
//`define PER_SIZE_CUSTOM
//`define PER_SIZE_32_KB
//`define PER_SIZE_16_KB
//`define PER_SIZE_8_KB
//`define PER_SIZE_4_KB
//`define PER_SIZE_2_KB
//`define PER_SIZE_1_KB
`define PER_SIZE_512_B


//-------------------------------------------------------
// Defines the debugger CPU_CTL.RST_BRK_EN reset value
// (CPU break on PUC reset)
//-------------------------------------------------------
// When defined, the CPU will automatically break after
// a PUC occurrence by default. This is typically useful
// when the program memory can only be initialized through
// the serial debug interface.
//-------------------------------------------------------
`define DBG_RST_BRK_EN


//============================================================================
//============================================================================
// EXPERT SYSTEM CONFIGURATION ( !!!! EXPERTS ONLY !!!! )
//============================================================================
//============================================================================
//
// IMPORTANT NOTE:  Please update following configuration options ONLY if
//                 you have a good reason to do so... and if you know what
//                 you are doing :-P
//
//============================================================================

//-------------------------------------------------------
// Select serial debug interface protocol
//-------------------------------------------------------
//    DBG_UART -> Enable UART (8N1) debug interface
//    DBG_I2C  -> Enable I2C debug interface
//-------------------------------------------------------
`define DBG_UART
//`define DBG_I2C


//-------------------------------------------------------
// Enable the I2C broadcast address
//-------------------------------------------------------
// For multicore systems, a common I2C broadcast address
// can be given to all oMSP cores in order to
// synchronously RESET, START, STOP, or STEP all CPUs
// at once with a single I2C command.
// If you have a single openMSP430 in your system,
// this option can stay commented-out.
//-------------------------------------------------------
//`define DBG_I2C_BROADCAST


//-------------------------------------------------------
// Number of hardware breakpoint/watchpoint units
// (each unit contains two hardware addresses available
// for breakpoints or watchpoints):
//   - DBG_HWBRK_0 -> Include hardware breakpoints unit 0
//   - DBG_HWBRK_1 -> Include hardware breakpoints unit 1
//   - DBG_HWBRK_2 -> Include hardware breakpoints unit 2
//   - DBG_HWBRK_3 -> Include hardware breakpoints unit 3
//-------------------------------------------------------
// Please keep in mind that hardware breakpoints only
// make sense whenever the program memory is not an SRAM
// (i.e. Flash/OTP/ROM/...) or when you are interested
// in data breakpoints.
//-------------------------------------------------------
//`define  DBG_HWBRK_0
//`define  DBG_HWBRK_1
//`define  DBG_HWBRK_2
//`define  DBG_HWBRK_3


//-------------------------------------------------------
// Enable/Disable the hardware breakpoint RANGE mode
//-------------------------------------------------------
// When enabled this feature allows the hardware breakpoint
// units to stop the cpu whenever an instruction or data
// access lays within an address range.
// Note that this feature is not supported by GDB.
//-------------------------------------------------------
//`define DBG_HWBRK_RANGE


//-------------------------------------------------------
// Custom Program/Data and Peripheral Memory Spaces
//-------------------------------------------------------
// The following values are valid only if the
// corresponding *_SIZE_CUSTOM defines are uncommented:
//
//  - *_SIZE   : size of the section in bytes.
//  - *_AWIDTH : address port width, this value must allow
//               to address all WORDS of the section
//               (i.e. the *_SIZE divided by 2)
//-------------------------------------------------------

// Custom Program memory    (enabled with PMEM_SIZE_CUSTOM)
`define PMEM_CUSTOM_AWIDTH      10
`define PMEM_CUSTOM_SIZE      2048

// Custom Data memory       (enabled with DMEM_SIZE_CUSTOM)
`define DMEM_CUSTOM_AWIDTH       6
`define DMEM_CUSTOM_SIZE       128

// Custom Peripheral memory (enabled with PER_SIZE_CUSTOM)
`define PER_CUSTOM_AWIDTH        8
`define PER_CUSTOM_SIZE        512


//-------------------------------------------------------
// ASIC version
//-------------------------------------------------------
// When uncommented, this define will enable the
// ASIC system configuration section (see below) and
// will activate scan support for production test.
//
// WARNING: if you target an FPGA, leave this define
//          commented.
//-------------------------------------------------------
`define ASIC


//============================================================================
//============================================================================
// ASIC SYSTEM CONFIGURATION ( !!!! EXPERTS/PROFESSIONALS ONLY !!!! )
//============================================================================
//============================================================================
`ifdef ASIC

//===============================================================
// FINE GRAINED CLOCK GATING
//===============================================================

//-------------------------------------------------------
// When uncommented, this define will enable the fine
// grained clock gating of all registers in the core.
//-------------------------------------------------------
`define CLOCK_GATING


//===============================================================
// ASIC CLOCKING
//===============================================================

//-------------------------------------------------------
// When uncommented, this define will enable the ASIC
// architectural clock gating as well as the advanced low
// power modes support (most common).
// Comment this out in order to get FPGA-like clocking.
//-------------------------------------------------------
`define ASIC_CLOCKING


`ifdef ASIC_CLOCKING
//===============================================================
// LFXT CLOCK DOMAIN
//===============================================================

//-------------------------------------------------------
// When uncommented, this define will enable the lfxt_clk
// clock domain.
// When commented out, the whole chip is clocked with dco_clk.
//-------------------------------------------------------
`define LFXT_DOMAIN


//===============================================================
// CLOCK MUXES
//===============================================================

//-------------------------------------------------------
// MCLK: Clock Mux
//-------------------------------------------------------
// When uncommented, this define will enable the
// MCLK clock MUX allowing the selection between
// DCO_CLK and LFXT_CLK with the BCSCTL2.SELMx register.
// When commented, DCO_CLK is selected.
//-------------------------------------------------------
//`define MCLK_MUX

//-------------------------------------------------------
// SMCLK: Clock Mux
//-------------------------------------------------------
// When uncommented, this define will enable the
// SMCLK clock MUX allowing the selection between
// DCO_CLK and LFXT_CLK with the BCSCTL2.SELS register.
// When commented, DCO_CLK is selected.
//-------------------------------------------------------
//`define SMCLK_MUX

//-------------------------------------------------------
// WATCHDOG: Clock Mux
//-------------------------------------------------------
// When uncommented, this define will enable the
// Watchdog clock MUX allowing the selection between
// ACLK and SMCLK with the WDTCTL.WDTSSEL register.
// When commented out, ACLK is selected if the
// WATCHDOG_NOMUX_ACLK define is uncommented, SMCLK is
// selected otherwise.
//-------------------------------------------------------
//`define WATCHDOG_MUX
//`define WATCHDOG_NOMUX_ACLK


//===============================================================
// CLOCK DIVIDERS
//===============================================================

//-------------------------------------------------------
// MCLK: Clock divider
//-------------------------------------------------------
// When uncommented, this define will enable the
// MCLK clock divider (/1/2/4/8)
//-------------------------------------------------------
`define MCLK_DIVIDER

//-------------------------------------------------------
// SMCLK: Clock divider (/1/2/4/8)
//-------------------------------------------------------
// When uncommented, this define will enable the
// SMCLK clock divider
//-------------------------------------------------------
`define SMCLK_DIVIDER

//-------------------------------------------------------
// ACLK: Clock divider (/1/2/4/8)
//-------------------------------------------------------
// When uncommented, this define will enable the
// ACLK clock divider
//-------------------------------------------------------
`define ACLK_DIVIDER


//===============================================================
// LOW POWER MODES
//===============================================================

//-------------------------------------------------------
// LOW POWER MODE: CPUOFF
//-------------------------------------------------------
// When uncommented, this define will include the
// clock gate allowing to switch off MCLK in
// all low power modes: LPM0, LPM1, LPM2, LPM3, LPM4
//-------------------------------------------------------
`define CPUOFF_EN

//-------------------------------------------------------
// LOW POWER MODE: SCG0
//-------------------------------------------------------
// When uncommented, this define will enable the
// DCO_ENABLE/WKUP port control (always 1 when commented).
// This allows to switch off the DCO oscillator in the
// following low power modes: LPM1, LPM3, LPM4
//-------------------------------------------------------
`define SCG0_EN

//-------------------------------------------------------
// LOW POWER MODE: SCG1
//-------------------------------------------------------
// When uncommented, this define will include the
// clock gate allowing to switch off SMCLK in
// the following low power modes: LPM2, LPM3, LPM4
//-------------------------------------------------------
`define SCG1_EN

//-------------------------------------------------------
// LOW POWER MODE: OSCOFF
//-------------------------------------------------------
// When uncommented, this define will include the
// LFXT_CLK clock gate and enable the LFXT_ENABLE/WKUP
// port control (always 1 when commented).
// This allows to switch off the low frequency oscillator
// in the following low power modes: LPM4
//-------------------------------------------------------
`define OSCOFF_EN

//-------------------------------------------------------
// SCAN REPAIR NEG-EDGE CLOCKED FLIP-FLOPS
//-------------------------------------------------------
// When uncommented, a scan mux will be infered to
// replace all inverted clocks with regular ones when
// in scan mode.
//
// Note: standard scan insertion tool can usually deal
//       with mixed rising/falling edge FF... so there
//       is usually no need to uncomment this.
//-------------------------------------------------------
//`define SCAN_REPAIR_INV_CLOCKS

`endif
`endif

//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//=====        SYSTEM CONSTANTS --- !!!!!!!! DO NOT EDIT !!!!!!!!      =====//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

//
// PROGRAM, DATA & PERIPHERAL MEMORY CONFIGURATION
//==================================================

// Program Memory Size
`ifdef PMEM_SIZE_59_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     60416
`endif
`ifdef PMEM_SIZE_55_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     56320
`endif
`ifdef PMEM_SIZE_54_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     55296
`endif
`ifdef PMEM_SIZE_51_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     52224
`endif
`ifdef PMEM_SIZE_48_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     49152
`endif
`ifdef PMEM_SIZE_41_KB
  `define PMEM_AWIDTH      15
  `define PMEM_SIZE     41984
`endif
`ifdef PMEM_SIZE_32_KB
  `define PMEM_AWIDTH      14
  `define PMEM_SIZE     32768
`endif
`ifdef PMEM_SIZE_24_KB
  `define PMEM_AWIDTH      14
  `define PMEM_SIZE     24576
`endif
`ifdef PMEM_SIZE_16_KB
  `define PMEM_AWIDTH      13
  `define PMEM_SIZE     16384
`endif
`ifdef PMEM_SIZE_12_KB
  `define PMEM_AWIDTH      13
  `define PMEM_SIZE     12288
`endif
`ifdef PMEM_SIZE_8_KB
  `define PMEM_AWIDTH      12
  `define PMEM_SIZE      8192
`endif
`ifdef PMEM_SIZE_4_KB
  `define PMEM_AWIDTH      11
  `define PMEM_SIZE      4096
`endif
`ifdef PMEM_SIZE_2_KB
  `define PMEM_AWIDTH      10
  `define PMEM_SIZE      2048
`endif
`ifdef PMEM_SIZE_1_KB
  `define PMEM_AWIDTH       9
  `define PMEM_SIZE      1024
`endif
`ifdef PMEM_SIZE_CUSTOM
  `define PMEM_AWIDTH       `PMEM_CUSTOM_AWIDTH
  `define PMEM_SIZE         `PMEM_CUSTOM_SIZE
`endif

// Data Memory Size
`ifdef DMEM_SIZE_32_KB
  `define DMEM_AWIDTH       14
  `define DMEM_SIZE      32768
`endif
`ifdef DMEM_SIZE_24_KB
  `define DMEM_AWIDTH       14
  `define DMEM_SIZE      24576
`endif
`ifdef DMEM_SIZE_16_KB
  `define DMEM_AWIDTH       13
  `define DMEM_SIZE      16384
`endif
`ifdef DMEM_SIZE_10_KB
  `define DMEM_AWIDTH       13
  `define DMEM_SIZE      10240
`endif
`ifdef DMEM_SIZE_8_KB
  `define DMEM_AWIDTH       12
  `define DMEM_SIZE       8192
`endif
`ifdef DMEM_SIZE_5_KB
  `define DMEM_AWIDTH       12
  `define DMEM_SIZE       5120
`endif
`ifdef DMEM_SIZE_4_KB
  `define DMEM_AWIDTH       11
  `define DMEM_SIZE       4096
`endif
`ifdef DMEM_SIZE_2p5_KB
  `define DMEM_AWIDTH       11
  `define DMEM_SIZE       2560
`endif
`ifdef DMEM_SIZE_2_KB
  `define DMEM_AWIDTH       10
  `define DMEM_SIZE       2048
`endif
`ifdef DMEM_SIZE_1_KB
  `define DMEM_AWIDTH        9
  `define DMEM_SIZE       1024
`endif
`ifdef DMEM_SIZE_512_B
  `define DMEM_AWIDTH        8
  `define DMEM_SIZE        512
`endif
`ifdef DMEM_SIZE_256_B
  `define DMEM_AWIDTH        7
  `define DMEM_SIZE        256
`endif
`ifdef DMEM_SIZE_128_B
  `define DMEM_AWIDTH        6
  `define DMEM_SIZE        128
`endif
`ifdef DMEM_SIZE_CUSTOM
  `define DMEM_AWIDTH       `DMEM_CUSTOM_AWIDTH
  `define DMEM_SIZE         `DMEM_CUSTOM_SIZE
`endif

// Peripheral Memory Size
`ifdef PER_SIZE_32_KB
  `define PER_AWIDTH        14
  `define PER_SIZE       32768
`endif
`ifdef PER_SIZE_16_KB
  `define PER_AWIDTH        13
  `define PER_SIZE       16384
`endif
`ifdef PER_SIZE_8_KB
  `define PER_AWIDTH        12
  `define PER_SIZE        8192
`endif
`ifdef PER_SIZE_4_KB
  `define PER_AWIDTH        11
  `define PER_SIZE        4096
`endif
`ifdef PER_SIZE_2_KB
  `define PER_AWIDTH        10
  `define PER_SIZE        2048
`endif
`ifdef PER_SIZE_1_KB
  `define PER_AWIDTH         9
  `define PER_SIZE        1024
`endif
`ifdef PER_SIZE_512_B
  `define PER_AWIDTH         8
  `define PER_SIZE         512
`endif
`ifdef PER_SIZE_CUSTOM
  `define PER_AWIDTH        `PER_CUSTOM_AWIDTH
  `define PER_SIZE          `PER_CUSTOM_SIZE
`endif

// Data Memory Base Adresses
`define DMEM_BASE  `PER_SIZE

// Program & Data Memory most significant address bit (for 16 bit words)
`define PMEM_MSB   `PMEM_AWIDTH-1
`define DMEM_MSB   `DMEM_AWIDTH-1
`define PER_MSB    `PER_AWIDTH-1

// Number of available IRQs
`ifdef  IRQ_16
`define IRQ_NR 16
`endif
`ifdef  IRQ_32
`define IRQ_NR 32
`define IRQ_NR_GE_32
`endif
`ifdef  IRQ_64
`define IRQ_NR 64
`define IRQ_NR_GE_32
`endif

//
// STATES, REGISTER FIELDS, ...
//======================================

// Instructions type
`define INST_SO  0
`define INST_JMP 1
`define INST_TO  2

// Single-operand arithmetic
`define RRC    0
`define SWPB   1
`define RRA    2
`define SXT    3
`define PUSH   4
`define CALL   5
`define RETI   6
`define IRQ    7

// Conditional jump
`define JNE    0
`define JEQ    1
`define JNC    2
`define JC     3
`define JN     4
`define JGE    5
`define JL     6
`define JMP    7

// Two-operand arithmetic
`define MOV    0
`define ADD    1
`define ADDC   2
`define SUBC   3
`define SUB    4
`define CMP    5
`define DADD   6
`define BIT    7
`define BIC    8
`define BIS    9
`define XOR   10
`define AND   11

// Addressing modes
`define DIR      0
`define IDX      1
`define INDIR    2
`define INDIR_I  3
`define SYMB     4
`define IMM      5
`define ABS      6
`define CONST    7

// Instruction state machine
`define I_IRQ_FETCH 3'h0
`define I_IRQ_DONE  3'h1
`define I_DEC       3'h2
`define I_EXT1      3'h3
`define I_EXT2      3'h4
`define I_IDLE      3'h5

// Execution state machine
// (swapped E_IRQ_0 and E_IRQ_2 values to suppress glitch generation warning from lint tool)
`define E_IRQ_0     4'h2
`define E_IRQ_1     4'h1
`define E_IRQ_2     4'h0
`define E_IRQ_3     4'h3
`define E_IRQ_4     4'h4
`define E_SRC_AD    4'h5
`define E_SRC_RD    4'h6
`define E_SRC_WR    4'h7
`define E_DST_AD    4'h8
`define E_DST_RD    4'h9
`define E_DST_WR    4'hA
`define E_EXEC      4'hB
`define E_JUMP      4'hC
`define E_IDLE      4'hD

// ALU control signals
`define ALU_SRC_INV   0
`define ALU_INC       1
`define ALU_INC_C     2
`define ALU_ADD       3
`define ALU_AND       4
`define ALU_OR        5
`define ALU_XOR       6
`define ALU_DADD      7
`define ALU_STAT_7    8
`define ALU_STAT_F    9
`define ALU_SHIFT    10
`define EXEC_NO_WR   11

// Debug interface
`define DBG_UART_WR   18
`define DBG_UART_BW   17
`define DBG_UART_ADDR 16:11

// Debug interface CPU_CTL register
`define HALT        0
`define RUN         1
`define ISTEP       2
`define SW_BRK_EN   3
`define FRZ_BRK_EN  4
`define RST_BRK_EN  5
`define CPU_RST     6

// Debug interface CPU_STAT register
`define HALT_RUN    0
`define PUC_PND     1
`define SWBRK_PND   3
`define HWBRK0_PND  4
`define HWBRK1_PND  5

// Debug interface BRKx_CTL register
`define BRK_MODE_RD 0
`define BRK_MODE_WR 1
`define BRK_MODE    1:0
`define BRK_EN      2
`define BRK_I_EN    3
`define BRK_RANGE   4

// Basic clock module: BCSCTL1 Control Register
`define DIVAx       5:4
`define DMA_CPUOFF  0
`define DMA_OSCOFF  1
`define DMA_SCG0    2
`define DMA_SCG1    3

// Basic clock module: BCSCTL2 Control Register
`define SELMx       7
`define DIVMx       5:4
`define SELS        3
`define DIVSx       2:1

// MCLK Clock gate
`ifdef CPUOFF_EN
  `define MCLK_CGATE
`else
`ifdef MCLK_DIVIDER
  `define MCLK_CGATE
`endif
`endif

// SMCLK Clock gate
`ifdef SCG1_EN
  `define SMCLK_CGATE
`else
`ifdef SMCLK_DIVIDER
  `define SMCLK_CGATE
`endif
`endif

//
// DEBUG INTERFACE EXTRA CONFIGURATION
//======================================

// Debug interface: CPU version
//   1 - FPGA support only (Pre-BSD licence era)
//   2 - Add ASIC support
//   3 - Add DMA interface support
`define CPU_VERSION   3'h3

// Debug interface: Software breakpoint opcode
`define DBG_SWBRK_OP 16'h4343

// Debug UART interface auto data synchronization
// If the following define is commented out, then
// the DBG_UART_BAUD and DBG_DCO_FREQ need to be properly
// defined.
`define DBG_UART_AUTO_SYNC

// Debug UART interface data rate
//      In order to properly setup the UART debug interface, you
//      need to specify the DCO_CLK frequency (DBG_DCO_FREQ) and
//      the chosen BAUD rate from the UART interface.
//
//`define DBG_UART_BAUD    9600
//`define DBG_UART_BAUD   19200
//`define DBG_UART_BAUD   38400
//`define DBG_UART_BAUD   57600
//`define DBG_UART_BAUD  115200
//`define DBG_UART_BAUD  230400
//`define DBG_UART_BAUD  460800
//`define DBG_UART_BAUD  576000
//`define DBG_UART_BAUD  921600
`define DBG_UART_BAUD 2000000
`define DBG_DCO_FREQ  20000000
`define DBG_UART_CNT ((`DBG_DCO_FREQ/`DBG_UART_BAUD)-1)

// Debug interface input synchronizer
`define SYNC_DBG_UART_RXD

// Enable/Disable the hardware breakpoint RANGE mode
`ifdef DBG_HWBRK_RANGE
 `define HWBRK_RANGE 1'b1
`else
 `define HWBRK_RANGE 1'b0
`endif

// Counter width for the debug interface UART
`define DBG_UART_XFER_CNT_W 16

// Check configuration
`ifdef DBG_EN
 `ifdef DBG_UART
   `ifdef DBG_I2C
CONFIGURATION ERROR: I2C AND UART DEBUG INTERFACE ARE BOTH ENABLED
   `endif
 `else
   `ifdef DBG_I2C
   `else
CONFIGURATION ERROR: I2C OR UART DEBUG INTERFACE SHOULD BE ENABLED
   `endif
 `endif
`endif

//
// MULTIPLIER CONFIGURATION
//======================================

// If uncommented, the following define selects
// the 16x16 multiplier (1 cycle) instead of the
// default 16x8 multplier (2 cycles)
//`define MPY_16x16

//======================================
// CONFIGURATION CHECKS
//======================================

`ifdef  IRQ_16
  `ifdef  IRQ_32
CONFIGURATION ERROR: ONLY ONE OF THE IRQ NUMBER OPTION CAN BE SELECTED
  `endif
  `ifdef  IRQ_64
CONFIGURATION ERROR: ONLY ONE OF THE IRQ NUMBER OPTION CAN BE SELECTED
  `endif
`endif
`ifdef  IRQ_32
  `ifdef  IRQ_64
CONFIGURATION ERROR: ONLY ONE OF THE IRQ NUMBER OPTION CAN BE SELECTED
  `endif
`endif
`ifdef LFXT_DOMAIN
`else
 `ifdef MCLK_MUX
CONFIGURATION ERROR: THE MCLK_MUX CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `endif
 `ifdef SMCLK_MUX
CONFIGURATION ERROR: THE SMCLK_MUX CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `endif
 `ifdef WATCHDOG_MUX
CONFIGURATION ERROR: THE WATCHDOG_MUX CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `else
   `ifdef WATCHDOG_NOMUX_ACLK
CONFIGURATION ERROR: THE WATCHDOG_NOMUX_ACLK CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
   `endif
 `endif
 `ifdef OSCOFF_EN
CONFIGURATION ERROR: THE OSCOFF LOW POWER MODE CAN ONLY BE ENABLED IF THE LFXT_DOMAIN IS ENABLED AS WELL
 `endif
`endif
//----------------------------------------------------------------------------
// Copyright (C) 2001 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: ram.v
//
// *Module Description:
//                      Scalable RAM model
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 103 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2011-03-05 15:44:48 +0100 (Sat, 05 Mar 2011) $
//----------------------------------------------------------------------------

module ram (

// OUTPUTs
    ram_dout,                      // RAM data output

// INPUTs
    ram_addr,                      // RAM address
    ram_cen,                       // RAM chip enable (low active)
    ram_clk,                       // RAM clock
    ram_din,                       // RAM data input
    ram_wen                        // RAM write enable (low active)
);

// PARAMETERs
//============
parameter ADDR_MSB   =  6;         // MSB of the address bus
parameter MEM_SIZE   =  256;       // Memory size in bytes

// OUTPUTs
//============
output      [15:0] ram_dout;       // RAM data output

// INPUTs
//============
input [ADDR_MSB:0] ram_addr;       // RAM address
input              ram_cen;        // RAM chip enable (low active)
input              ram_clk;        // RAM clock
input       [15:0] ram_din;        // RAM data input
input        [1:0] ram_wen;        // RAM write enable (low active)


// RAM
//============

reg         [15:0] mem [0:(MEM_SIZE/2)-1];
reg   [ADDR_MSB:0] ram_addr_reg;

wire        [15:0] mem_val = mem[ram_addr];


always @(posedge ram_clk)
  if (~ram_cen & ram_addr<(MEM_SIZE/2))
    begin
      if      (ram_wen==2'b00) mem[ram_addr] <= ram_din;
      else if (ram_wen==2'b01) mem[ram_addr] <= {ram_din[15:8], mem_val[7:0]};
      else if (ram_wen==2'b10) mem[ram_addr] <= {mem_val[15:8], ram_din[7:0]};
      ram_addr_reg <= ram_addr;
    end

assign ram_dout = mem[ram_addr_reg];


endmodule // ram
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_gpio.v
//
// *Module Description:
//                       Digital I/O interface
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 134 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2012-03-22 21:31:06 +0100 (Thu, 22 Mar 2012) $
//----------------------------------------------------------------------------

module  omsp_gpio (

// OUTPUTs
    irq_port1,                      // Port 1 interrupt
    irq_port2,                      // Port 2 interrupt
    p1_dout,                        // Port 1 data output
    p1_dout_en,                     // Port 1 data output enable
    p1_sel,                         // Port 1 function select
    p2_dout,                        // Port 2 data output
    p2_dout_en,                     // Port 2 data output enable
    p2_sel,                         // Port 2 function select
    p3_dout,                        // Port 3 data output
    p3_dout_en,                     // Port 3 data output enable
    p3_sel,                         // Port 3 function select
    p4_dout,                        // Port 4 data output
    p4_dout_en,                     // Port 4 data output enable
    p4_sel,                         // Port 4 function select
    p5_dout,                        // Port 5 data output
    p5_dout_en,                     // Port 5 data output enable
    p5_sel,                         // Port 5 function select
    p6_dout,                        // Port 6 data output
    p6_dout_en,                     // Port 6 data output enable
    p6_sel,                         // Port 6 function select
    per_dout,                       // Peripheral data output

// INPUTs
    mclk,                           // Main system clock
    p1_din,                         // Port 1 data input
    p2_din,                         // Port 2 data input
    p3_din,                         // Port 3 data input
    p4_din,                         // Port 4 data input
    p5_din,                         // Port 5 data input
    p6_din,                         // Port 6 data input
    per_addr,                       // Peripheral address
    per_din,                        // Peripheral data input
    per_en,                         // Peripheral enable (high active)
    per_we,                         // Peripheral write enable (high active)
    puc_rst                         // Main system reset
);

// PARAMETERs
//============
parameter           P1_EN = 1'b1;   // Enable Port 1
parameter           P2_EN = 1'b1;   // Enable Port 2
parameter           P3_EN = 1'b0;   // Enable Port 3
parameter           P4_EN = 1'b0;   // Enable Port 4
parameter           P5_EN = 1'b0;   // Enable Port 5
parameter           P6_EN = 1'b0;   // Enable Port 6


// OUTPUTs
//=========
output              irq_port1;      // Port 1 interrupt
output              irq_port2;      // Port 2 interrupt
output        [7:0] p1_dout;        // Port 1 data output
output        [7:0] p1_dout_en;     // Port 1 data output enable
output        [7:0] p1_sel;         // Port 1 function select
output        [7:0] p2_dout;        // Port 2 data output
output        [7:0] p2_dout_en;     // Port 2 data output enable
output        [7:0] p2_sel;         // Port 2 function select
output        [7:0] p3_dout;        // Port 3 data output
output        [7:0] p3_dout_en;     // Port 3 data output enable
output        [7:0] p3_sel;         // Port 3 function select
output        [7:0] p4_dout;        // Port 4 data output
output        [7:0] p4_dout_en;     // Port 4 data output enable
output        [7:0] p4_sel;         // Port 4 function select
output        [7:0] p5_dout;        // Port 5 data output
output        [7:0] p5_dout_en;     // Port 5 data output enable
output        [7:0] p5_sel;         // Port 5 function select
output        [7:0] p6_dout;        // Port 6 data output
output        [7:0] p6_dout_en;     // Port 6 data output enable
output        [7:0] p6_sel;         // Port 6 function select
output       [15:0] per_dout;       // Peripheral data output

// INPUTs
//=========
input               mclk;           // Main system clock
input         [7:0] p1_din;         // Port 1 data input
input         [7:0] p2_din;         // Port 2 data input
input         [7:0] p3_din;         // Port 3 data input
input         [7:0] p4_din;         // Port 4 data input
input         [7:0] p5_din;         // Port 5 data input
input         [7:0] p6_din;         // Port 6 data input
input        [13:0] per_addr;       // Peripheral address
input        [15:0] per_din;        // Peripheral data input
input               per_en;         // Peripheral enable (high active)
input         [1:0] per_we;         // Peripheral write enable (high active)
input               puc_rst;        // Main system reset


//=============================================================================
// 1)  PARAMETER DECLARATION
//=============================================================================

// Masks
parameter              P1_EN_MSK   = {8{P1_EN[0]}};
parameter              P2_EN_MSK   = {8{P2_EN[0]}};
parameter              P3_EN_MSK   = {8{P3_EN[0]}};
parameter              P4_EN_MSK   = {8{P4_EN[0]}};
parameter              P5_EN_MSK   = {8{P5_EN[0]}};
parameter              P6_EN_MSK   = {8{P6_EN[0]}};

// Register base address (must be aligned to decoder bit width)
parameter       [14:0] BASE_ADDR   = 15'h0000;

// Decoder bit width (defines how many bits are considered for address decoding)
parameter              DEC_WD      =  6;

// Register addresses offset
parameter [DEC_WD-1:0] P1IN        = 'h20,                    // Port 1
                       P1OUT       = 'h21,
                       P1DIR       = 'h22,
                       P1IFG       = 'h23,
                       P1IES       = 'h24,
                       P1IE        = 'h25,
                       P1SEL       = 'h26,
                       P2IN        = 'h28,                    // Port 2
                       P2OUT       = 'h29,
                       P2DIR       = 'h2A,
                       P2IFG       = 'h2B,
                       P2IES       = 'h2C,
                       P2IE        = 'h2D,
                       P2SEL       = 'h2E,
                       P3IN        = 'h18,                    // Port 3
                       P3OUT       = 'h19,
                       P3DIR       = 'h1A,
                       P3SEL       = 'h1B,
                       P4IN        = 'h1C,                    // Port 4
                       P4OUT       = 'h1D,
                       P4DIR       = 'h1E,
                       P4SEL       = 'h1F,
                       P5IN        = 'h30,                    // Port 5
                       P5OUT       = 'h31,
                       P5DIR       = 'h32,
                       P5SEL       = 'h33,
                       P6IN        = 'h34,                    // Port 6
                       P6OUT       = 'h35,
                       P6DIR       = 'h36,
                       P6SEL       = 'h37;

// Register one-hot decoder utilities
parameter              DEC_SZ      =  (1 << DEC_WD);
parameter [DEC_SZ-1:0] BASE_REG    =  {{DEC_SZ-1{1'b0}}, 1'b1};

// Register one-hot decoder
parameter [DEC_SZ-1:0] P1IN_D      =  (BASE_REG << P1IN),     // Port 1
                       P1OUT_D     =  (BASE_REG << P1OUT),
                       P1DIR_D     =  (BASE_REG << P1DIR),
                       P1IFG_D     =  (BASE_REG << P1IFG),
                       P1IES_D     =  (BASE_REG << P1IES),
                       P1IE_D      =  (BASE_REG << P1IE),
                       P1SEL_D     =  (BASE_REG << P1SEL),
                       P2IN_D      =  (BASE_REG << P2IN),     // Port 2
                       P2OUT_D     =  (BASE_REG << P2OUT),
                       P2DIR_D     =  (BASE_REG << P2DIR),
                       P2IFG_D     =  (BASE_REG << P2IFG),
                       P2IES_D     =  (BASE_REG << P2IES),
                       P2IE_D      =  (BASE_REG << P2IE),
                       P2SEL_D     =  (BASE_REG << P2SEL),
                       P3IN_D      =  (BASE_REG << P3IN),     // Port 3
                       P3OUT_D     =  (BASE_REG << P3OUT),
                       P3DIR_D     =  (BASE_REG << P3DIR),
                       P3SEL_D     =  (BASE_REG << P3SEL),
                       P4IN_D      =  (BASE_REG << P4IN),     // Port 4
                       P4OUT_D     =  (BASE_REG << P4OUT),
                       P4DIR_D     =  (BASE_REG << P4DIR),
                       P4SEL_D     =  (BASE_REG << P4SEL),
                       P5IN_D      =  (BASE_REG << P5IN),     // Port 5
                       P5OUT_D     =  (BASE_REG << P5OUT),
                       P5DIR_D     =  (BASE_REG << P5DIR),
                       P5SEL_D     =  (BASE_REG << P5SEL),
                       P6IN_D      =  (BASE_REG << P6IN),     // Port 6
                       P6OUT_D     =  (BASE_REG << P6OUT),
                       P6DIR_D     =  (BASE_REG << P6DIR),
                       P6SEL_D     =  (BASE_REG << P6SEL);


//============================================================================
// 2)  REGISTER DECODER
//============================================================================

// Local register selection
wire              reg_sel      =  per_en & (per_addr[13:DEC_WD-1]==BASE_ADDR[14:DEC_WD]);

// Register local address
wire [DEC_WD-1:0] reg_addr     =  {1'b0, per_addr[DEC_WD-2:0]};

// Register address decode
wire [DEC_SZ-1:0] reg_dec      =  (P1IN_D   &  {DEC_SZ{(reg_addr==(P1IN  >>1))  &  P1_EN[0]}})  |
                                  (P1OUT_D  &  {DEC_SZ{(reg_addr==(P1OUT >>1))  &  P1_EN[0]}})  |
                                  (P1DIR_D  &  {DEC_SZ{(reg_addr==(P1DIR >>1))  &  P1_EN[0]}})  |
                                  (P1IFG_D  &  {DEC_SZ{(reg_addr==(P1IFG >>1))  &  P1_EN[0]}})  |
                                  (P1IES_D  &  {DEC_SZ{(reg_addr==(P1IES >>1))  &  P1_EN[0]}})  |
                                  (P1IE_D   &  {DEC_SZ{(reg_addr==(P1IE  >>1))  &  P1_EN[0]}})  |
                                  (P1SEL_D  &  {DEC_SZ{(reg_addr==(P1SEL >>1))  &  P1_EN[0]}})  |
                                  (P2IN_D   &  {DEC_SZ{(reg_addr==(P2IN  >>1))  &  P2_EN[0]}})  |
                                  (P2OUT_D  &  {DEC_SZ{(reg_addr==(P2OUT >>1))  &  P2_EN[0]}})  |
                                  (P2DIR_D  &  {DEC_SZ{(reg_addr==(P2DIR >>1))  &  P2_EN[0]}})  |
                                  (P2IFG_D  &  {DEC_SZ{(reg_addr==(P2IFG >>1))  &  P2_EN[0]}})  |
                                  (P2IES_D  &  {DEC_SZ{(reg_addr==(P2IES >>1))  &  P2_EN[0]}})  |
                                  (P2IE_D   &  {DEC_SZ{(reg_addr==(P2IE  >>1))  &  P2_EN[0]}})  |
                                  (P2SEL_D  &  {DEC_SZ{(reg_addr==(P2SEL >>1))  &  P2_EN[0]}})  |
                                  (P3IN_D   &  {DEC_SZ{(reg_addr==(P3IN  >>1))  &  P3_EN[0]}})  |
                                  (P3OUT_D  &  {DEC_SZ{(reg_addr==(P3OUT >>1))  &  P3_EN[0]}})  |
                                  (P3DIR_D  &  {DEC_SZ{(reg_addr==(P3DIR >>1))  &  P3_EN[0]}})  |
                                  (P3SEL_D  &  {DEC_SZ{(reg_addr==(P3SEL >>1))  &  P3_EN[0]}})  |
                                  (P4IN_D   &  {DEC_SZ{(reg_addr==(P4IN  >>1))  &  P4_EN[0]}})  |
                                  (P4OUT_D  &  {DEC_SZ{(reg_addr==(P4OUT >>1))  &  P4_EN[0]}})  |
                                  (P4DIR_D  &  {DEC_SZ{(reg_addr==(P4DIR >>1))  &  P4_EN[0]}})  |
                                  (P4SEL_D  &  {DEC_SZ{(reg_addr==(P4SEL >>1))  &  P4_EN[0]}})  |
                                  (P5IN_D   &  {DEC_SZ{(reg_addr==(P5IN  >>1))  &  P5_EN[0]}})  |
                                  (P5OUT_D  &  {DEC_SZ{(reg_addr==(P5OUT >>1))  &  P5_EN[0]}})  |
                                  (P5DIR_D  &  {DEC_SZ{(reg_addr==(P5DIR >>1))  &  P5_EN[0]}})  |
                                  (P5SEL_D  &  {DEC_SZ{(reg_addr==(P5SEL >>1))  &  P5_EN[0]}})  |
                                  (P6IN_D   &  {DEC_SZ{(reg_addr==(P6IN  >>1))  &  P6_EN[0]}})  |
                                  (P6OUT_D  &  {DEC_SZ{(reg_addr==(P6OUT >>1))  &  P6_EN[0]}})  |
                                  (P6DIR_D  &  {DEC_SZ{(reg_addr==(P6DIR >>1))  &  P6_EN[0]}})  |
                                  (P6SEL_D  &  {DEC_SZ{(reg_addr==(P6SEL >>1))  &  P6_EN[0]}});

// Read/Write probes
wire              reg_lo_write =  per_we[0] & reg_sel;
wire              reg_hi_write =  per_we[1] & reg_sel;
wire              reg_read     = ~|per_we   & reg_sel;

// Read/Write vectors
wire [DEC_SZ-1:0] reg_hi_wr    = reg_dec & {DEC_SZ{reg_hi_write}};
wire [DEC_SZ-1:0] reg_lo_wr    = reg_dec & {DEC_SZ{reg_lo_write}};
wire [DEC_SZ-1:0] reg_rd       = reg_dec & {DEC_SZ{reg_read}};

//============================================================================
// 3) REGISTERS
//============================================================================

// P1IN Register
//---------------
wire [7:0] p1in;

omsp_sync_cell sync_cell_p1in_0 (.data_out(p1in[0]), .data_in(p1_din[0] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_1 (.data_out(p1in[1]), .data_in(p1_din[1] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_2 (.data_out(p1in[2]), .data_in(p1_din[2] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_3 (.data_out(p1in[3]), .data_in(p1_din[3] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_4 (.data_out(p1in[4]), .data_in(p1_din[4] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_5 (.data_out(p1in[5]), .data_in(p1_din[5] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_6 (.data_out(p1in[6]), .data_in(p1_din[6] & P1_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p1in_7 (.data_out(p1in[7]), .data_in(p1_din[7] & P1_EN[0]), .clk(mclk), .rst(puc_rst));


// P1OUT Register
//----------------
reg  [7:0] p1out;

wire       p1out_wr  = P1OUT[0] ? reg_hi_wr[P1OUT] : reg_lo_wr[P1OUT];
wire [7:0] p1out_nxt = P1OUT[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p1out <=  8'h00;
  else if (p1out_wr)  p1out <=  p1out_nxt & P1_EN_MSK;

assign p1_dout = p1out;


// P1DIR Register
//----------------
reg  [7:0] p1dir;

wire       p1dir_wr  = P1DIR[0] ? reg_hi_wr[P1DIR] : reg_lo_wr[P1DIR];
wire [7:0] p1dir_nxt = P1DIR[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p1dir <=  8'h00;
  else if (p1dir_wr)  p1dir <=  p1dir_nxt & P1_EN_MSK;

assign p1_dout_en = p1dir;


// P1IFG Register
//----------------
reg  [7:0] p1ifg;

wire       p1ifg_wr  = P1IFG[0] ? reg_hi_wr[P1IFG] : reg_lo_wr[P1IFG];
wire [7:0] p1ifg_nxt = P1IFG[0] ? per_din[15:8]    : per_din[7:0];
wire [7:0] p1ifg_set;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p1ifg <=  8'h00;
  else if (p1ifg_wr)  p1ifg <=  (p1ifg_nxt | p1ifg_set) & P1_EN_MSK;
  else                p1ifg <=  (p1ifg     | p1ifg_set) & P1_EN_MSK;

// P1IES Register
//----------------
reg  [7:0] p1ies;

wire       p1ies_wr  = P1IES[0] ? reg_hi_wr[P1IES] : reg_lo_wr[P1IES];
wire [7:0] p1ies_nxt = P1IES[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p1ies <=  8'h00;
  else if (p1ies_wr)  p1ies <=  p1ies_nxt & P1_EN_MSK;


// P1IE Register
//----------------
reg  [7:0] p1ie;

wire       p1ie_wr  = P1IE[0] ? reg_hi_wr[P1IE] : reg_lo_wr[P1IE];
wire [7:0] p1ie_nxt = P1IE[0] ? per_din[15:8]   : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p1ie <=  8'h00;
  else if (p1ie_wr)  p1ie <=  p1ie_nxt & P1_EN_MSK;


// P1SEL Register
//----------------
reg  [7:0] p1sel;

wire       p1sel_wr  = P1SEL[0] ? reg_hi_wr[P1SEL] : reg_lo_wr[P1SEL];
wire [7:0] p1sel_nxt = P1SEL[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p1sel <=  8'h00;
  else if (p1sel_wr) p1sel <=  p1sel_nxt & P1_EN_MSK;

assign p1_sel = p1sel;


// P2IN Register
//---------------
wire [7:0] p2in;

omsp_sync_cell sync_cell_p2in_0 (.data_out(p2in[0]), .data_in(p2_din[0] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_1 (.data_out(p2in[1]), .data_in(p2_din[1] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_2 (.data_out(p2in[2]), .data_in(p2_din[2] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_3 (.data_out(p2in[3]), .data_in(p2_din[3] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_4 (.data_out(p2in[4]), .data_in(p2_din[4] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_5 (.data_out(p2in[5]), .data_in(p2_din[5] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_6 (.data_out(p2in[6]), .data_in(p2_din[6] & P2_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p2in_7 (.data_out(p2in[7]), .data_in(p2_din[7] & P2_EN[0]), .clk(mclk), .rst(puc_rst));


// P2OUT Register
//----------------
reg  [7:0] p2out;

wire       p2out_wr  = P2OUT[0] ? reg_hi_wr[P2OUT] : reg_lo_wr[P2OUT];
wire [7:0] p2out_nxt = P2OUT[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p2out <=  8'h00;
  else if (p2out_wr)  p2out <=  p2out_nxt & P2_EN_MSK;

assign p2_dout = p2out;


// P2DIR Register
//----------------
reg  [7:0] p2dir;

wire       p2dir_wr  = P2DIR[0] ? reg_hi_wr[P2DIR] : reg_lo_wr[P2DIR];
wire [7:0] p2dir_nxt = P2DIR[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p2dir <=  8'h00;
  else if (p2dir_wr)  p2dir <=  p2dir_nxt & P2_EN_MSK;

assign p2_dout_en = p2dir;


// P2IFG Register
//----------------
reg  [7:0] p2ifg;

wire       p2ifg_wr  = P2IFG[0] ? reg_hi_wr[P2IFG] : reg_lo_wr[P2IFG];
wire [7:0] p2ifg_nxt = P2IFG[0] ? per_din[15:8]    : per_din[7:0];
wire [7:0] p2ifg_set;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p2ifg <=  8'h00;
  else if (p2ifg_wr)  p2ifg <=  (p2ifg_nxt | p2ifg_set) & P2_EN_MSK;
  else                p2ifg <=  (p2ifg     | p2ifg_set) & P2_EN_MSK;


// P2IES Register
//----------------
reg  [7:0] p2ies;

wire       p2ies_wr  = P2IES[0] ? reg_hi_wr[P2IES] : reg_lo_wr[P2IES];
wire [7:0] p2ies_nxt = P2IES[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p2ies <=  8'h00;
  else if (p2ies_wr)  p2ies <=  p2ies_nxt & P2_EN_MSK;


// P2IE Register
//----------------
reg  [7:0] p2ie;

wire       p2ie_wr  = P2IE[0] ? reg_hi_wr[P2IE] : reg_lo_wr[P2IE];
wire [7:0] p2ie_nxt = P2IE[0] ? per_din[15:8]   : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p2ie <=  8'h00;
  else if (p2ie_wr)  p2ie <=  p2ie_nxt & P2_EN_MSK;


// P2SEL Register
//----------------
reg  [7:0] p2sel;

wire       p2sel_wr  = P2SEL[0] ? reg_hi_wr[P2SEL] : reg_lo_wr[P2SEL];
wire [7:0] p2sel_nxt = P2SEL[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p2sel <=  8'h00;
  else if (p2sel_wr) p2sel <=  p2sel_nxt & P2_EN_MSK;

assign p2_sel = p2sel;


// P3IN Register
//---------------
wire  [7:0] p3in;

omsp_sync_cell sync_cell_p3in_0 (.data_out(p3in[0]), .data_in(p3_din[0] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_1 (.data_out(p3in[1]), .data_in(p3_din[1] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_2 (.data_out(p3in[2]), .data_in(p3_din[2] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_3 (.data_out(p3in[3]), .data_in(p3_din[3] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_4 (.data_out(p3in[4]), .data_in(p3_din[4] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_5 (.data_out(p3in[5]), .data_in(p3_din[5] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_6 (.data_out(p3in[6]), .data_in(p3_din[6] & P3_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p3in_7 (.data_out(p3in[7]), .data_in(p3_din[7] & P3_EN[0]), .clk(mclk), .rst(puc_rst));


// P3OUT Register
//----------------
reg  [7:0] p3out;

wire       p3out_wr  = P3OUT[0] ? reg_hi_wr[P3OUT] : reg_lo_wr[P3OUT];
wire [7:0] p3out_nxt = P3OUT[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p3out <=  8'h00;
  else if (p3out_wr)  p3out <=  p3out_nxt & P3_EN_MSK;

assign p3_dout = p3out;


// P3DIR Register
//----------------
reg  [7:0] p3dir;

wire       p3dir_wr  = P3DIR[0] ? reg_hi_wr[P3DIR] : reg_lo_wr[P3DIR];
wire [7:0] p3dir_nxt = P3DIR[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p3dir <=  8'h00;
  else if (p3dir_wr)  p3dir <=  p3dir_nxt & P3_EN_MSK;

assign p3_dout_en = p3dir;


// P3SEL Register
//----------------
reg  [7:0] p3sel;

wire       p3sel_wr  = P3SEL[0] ? reg_hi_wr[P3SEL] : reg_lo_wr[P3SEL];
wire [7:0] p3sel_nxt = P3SEL[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p3sel <=  8'h00;
  else if (p3sel_wr) p3sel <=  p3sel_nxt & P3_EN_MSK;

assign p3_sel = p3sel;


// P4IN Register
//---------------
wire  [7:0] p4in;

omsp_sync_cell sync_cell_p4in_0 (.data_out(p4in[0]), .data_in(p4_din[0] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_1 (.data_out(p4in[1]), .data_in(p4_din[1] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_2 (.data_out(p4in[2]), .data_in(p4_din[2] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_3 (.data_out(p4in[3]), .data_in(p4_din[3] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_4 (.data_out(p4in[4]), .data_in(p4_din[4] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_5 (.data_out(p4in[5]), .data_in(p4_din[5] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_6 (.data_out(p4in[6]), .data_in(p4_din[6] & P4_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p4in_7 (.data_out(p4in[7]), .data_in(p4_din[7] & P4_EN[0]), .clk(mclk), .rst(puc_rst));


// P4OUT Register
//----------------
reg  [7:0] p4out;

wire       p4out_wr  = P4OUT[0] ? reg_hi_wr[P4OUT] : reg_lo_wr[P4OUT];
wire [7:0] p4out_nxt = P4OUT[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p4out <=  8'h00;
  else if (p4out_wr)  p4out <=  p4out_nxt & P4_EN_MSK;

assign p4_dout = p4out;


// P4DIR Register
//----------------
reg  [7:0] p4dir;

wire       p4dir_wr  = P4DIR[0] ? reg_hi_wr[P4DIR] : reg_lo_wr[P4DIR];
wire [7:0] p4dir_nxt = P4DIR[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p4dir <=  8'h00;
  else if (p4dir_wr)  p4dir <=  p4dir_nxt & P4_EN_MSK;

assign p4_dout_en = p4dir;


// P4SEL Register
//----------------
reg  [7:0] p4sel;

wire       p4sel_wr  = P4SEL[0] ? reg_hi_wr[P4SEL] : reg_lo_wr[P4SEL];
wire [7:0] p4sel_nxt = P4SEL[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p4sel <=  8'h00;
  else if (p4sel_wr) p4sel <=  p4sel_nxt & P4_EN_MSK;

assign p4_sel = p4sel;


// P5IN Register
//---------------
wire  [7:0] p5in;

omsp_sync_cell sync_cell_p5in_0 (.data_out(p5in[0]), .data_in(p5_din[0] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_1 (.data_out(p5in[1]), .data_in(p5_din[1] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_2 (.data_out(p5in[2]), .data_in(p5_din[2] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_3 (.data_out(p5in[3]), .data_in(p5_din[3] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_4 (.data_out(p5in[4]), .data_in(p5_din[4] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_5 (.data_out(p5in[5]), .data_in(p5_din[5] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_6 (.data_out(p5in[6]), .data_in(p5_din[6] & P5_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p5in_7 (.data_out(p5in[7]), .data_in(p5_din[7] & P5_EN[0]), .clk(mclk), .rst(puc_rst));


// P5OUT Register
//----------------
reg  [7:0] p5out;

wire       p5out_wr  = P5OUT[0] ? reg_hi_wr[P5OUT] : reg_lo_wr[P5OUT];
wire [7:0] p5out_nxt = P5OUT[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p5out <=  8'h00;
  else if (p5out_wr)  p5out <=  p5out_nxt & P5_EN_MSK;

assign p5_dout = p5out;


// P5DIR Register
//----------------
reg  [7:0] p5dir;

wire       p5dir_wr  = P5DIR[0] ? reg_hi_wr[P5DIR] : reg_lo_wr[P5DIR];
wire [7:0] p5dir_nxt = P5DIR[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p5dir <=  8'h00;
  else if (p5dir_wr)  p5dir <=  p5dir_nxt & P5_EN_MSK;

assign p5_dout_en = p5dir;


// P5SEL Register
//----------------
reg  [7:0] p5sel;

wire       p5sel_wr  = P5SEL[0] ? reg_hi_wr[P5SEL] : reg_lo_wr[P5SEL];
wire [7:0] p5sel_nxt = P5SEL[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p5sel <=  8'h00;
  else if (p5sel_wr) p5sel <=  p5sel_nxt & P5_EN_MSK;

assign p5_sel = p5sel;


// P6IN Register
//---------------
wire  [7:0] p6in;

omsp_sync_cell sync_cell_p6in_0 (.data_out(p6in[0]), .data_in(p6_din[0] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_1 (.data_out(p6in[1]), .data_in(p6_din[1] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_2 (.data_out(p6in[2]), .data_in(p6_din[2] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_3 (.data_out(p6in[3]), .data_in(p6_din[3] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_4 (.data_out(p6in[4]), .data_in(p6_din[4] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_5 (.data_out(p6in[5]), .data_in(p6_din[5] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_6 (.data_out(p6in[6]), .data_in(p6_din[6] & P6_EN[0]), .clk(mclk), .rst(puc_rst));
omsp_sync_cell sync_cell_p6in_7 (.data_out(p6in[7]), .data_in(p6_din[7] & P6_EN[0]), .clk(mclk), .rst(puc_rst));


// P6OUT Register
//----------------
reg  [7:0] p6out;

wire       p6out_wr  = P6OUT[0] ? reg_hi_wr[P6OUT] : reg_lo_wr[P6OUT];
wire [7:0] p6out_nxt = P6OUT[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p6out <=  8'h00;
  else if (p6out_wr)  p6out <=  p6out_nxt & P6_EN_MSK;

assign p6_dout = p6out;


// P6DIR Register
//----------------
reg  [7:0] p6dir;

wire       p6dir_wr  = P6DIR[0] ? reg_hi_wr[P6DIR] : reg_lo_wr[P6DIR];
wire [7:0] p6dir_nxt = P6DIR[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        p6dir <=  8'h00;
  else if (p6dir_wr)  p6dir <=  p6dir_nxt & P6_EN_MSK;

assign p6_dout_en = p6dir;


// P6SEL Register
//----------------
reg  [7:0] p6sel;

wire       p6sel_wr  = P6SEL[0] ? reg_hi_wr[P6SEL] : reg_lo_wr[P6SEL];
wire [7:0] p6sel_nxt = P6SEL[0] ? per_din[15:8]    : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       p6sel <=  8'h00;
  else if (p6sel_wr) p6sel <=  p6sel_nxt & P6_EN_MSK;

assign p6_sel = p6sel;



//============================================================================
// 4) INTERRUPT GENERATION
//============================================================================

// Port 1 interrupt
//------------------

// Delay input
reg    [7:0] p1in_dly;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)  p1in_dly <=  8'h00;
  else          p1in_dly <=  p1in & P1_EN_MSK;

// Edge detection
wire   [7:0] p1in_re   =   p1in & ~p1in_dly;
wire   [7:0] p1in_fe   =  ~p1in &  p1in_dly;

// Set interrupt flag
assign       p1ifg_set = {p1ies[7] ? p1in_fe[7] : p1in_re[7],
                          p1ies[6] ? p1in_fe[6] : p1in_re[6],
                          p1ies[5] ? p1in_fe[5] : p1in_re[5],
                          p1ies[4] ? p1in_fe[4] : p1in_re[4],
                          p1ies[3] ? p1in_fe[3] : p1in_re[3],
                          p1ies[2] ? p1in_fe[2] : p1in_re[2],
                          p1ies[1] ? p1in_fe[1] : p1in_re[1],
                          p1ies[0] ? p1in_fe[0] : p1in_re[0]} & P1_EN_MSK;

// Generate CPU interrupt
assign       irq_port1 = |(p1ie & p1ifg) & P1_EN[0];


// Port 1 interrupt
//------------------

// Delay input
reg    [7:0] p2in_dly;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)  p2in_dly <=  8'h00;
  else          p2in_dly <=  p2in & P2_EN_MSK;

// Edge detection
wire   [7:0] p2in_re   =   p2in & ~p2in_dly;
wire   [7:0] p2in_fe   =  ~p2in &  p2in_dly;

// Set interrupt flag
assign       p2ifg_set = {p2ies[7] ? p2in_fe[7] : p2in_re[7],
                          p2ies[6] ? p2in_fe[6] : p2in_re[6],
                          p2ies[5] ? p2in_fe[5] : p2in_re[5],
                          p2ies[4] ? p2in_fe[4] : p2in_re[4],
                          p2ies[3] ? p2in_fe[3] : p2in_re[3],
                          p2ies[2] ? p2in_fe[2] : p2in_re[2],
                          p2ies[1] ? p2in_fe[1] : p2in_re[1],
                          p2ies[0] ? p2in_fe[0] : p2in_re[0]} & P2_EN_MSK;

// Generate CPU interrupt
assign      irq_port2 = |(p2ie & p2ifg) & P2_EN[0];


//============================================================================
// 5) DATA OUTPUT GENERATION
//============================================================================

// Data output mux
wire [15:0] p1in_rd   = {8'h00, (p1in  & {8{reg_rd[P1IN]}})}  << (8 & {4{P1IN[0]}});
wire [15:0] p1out_rd  = {8'h00, (p1out & {8{reg_rd[P1OUT]}})} << (8 & {4{P1OUT[0]}});
wire [15:0] p1dir_rd  = {8'h00, (p1dir & {8{reg_rd[P1DIR]}})} << (8 & {4{P1DIR[0]}});
wire [15:0] p1ifg_rd  = {8'h00, (p1ifg & {8{reg_rd[P1IFG]}})} << (8 & {4{P1IFG[0]}});
wire [15:0] p1ies_rd  = {8'h00, (p1ies & {8{reg_rd[P1IES]}})} << (8 & {4{P1IES[0]}});
wire [15:0] p1ie_rd   = {8'h00, (p1ie  & {8{reg_rd[P1IE]}})}  << (8 & {4{P1IE[0]}});
wire [15:0] p1sel_rd  = {8'h00, (p1sel & {8{reg_rd[P1SEL]}})} << (8 & {4{P1SEL[0]}});
wire [15:0] p2in_rd   = {8'h00, (p2in  & {8{reg_rd[P2IN]}})}  << (8 & {4{P2IN[0]}});
wire [15:0] p2out_rd  = {8'h00, (p2out & {8{reg_rd[P2OUT]}})} << (8 & {4{P2OUT[0]}});
wire [15:0] p2dir_rd  = {8'h00, (p2dir & {8{reg_rd[P2DIR]}})} << (8 & {4{P2DIR[0]}});
wire [15:0] p2ifg_rd  = {8'h00, (p2ifg & {8{reg_rd[P2IFG]}})} << (8 & {4{P2IFG[0]}});
wire [15:0] p2ies_rd  = {8'h00, (p2ies & {8{reg_rd[P2IES]}})} << (8 & {4{P2IES[0]}});
wire [15:0] p2ie_rd   = {8'h00, (p2ie  & {8{reg_rd[P2IE]}})}  << (8 & {4{P2IE[0]}});
wire [15:0] p2sel_rd  = {8'h00, (p2sel & {8{reg_rd[P2SEL]}})} << (8 & {4{P2SEL[0]}});
wire [15:0] p3in_rd   = {8'h00, (p3in  & {8{reg_rd[P3IN]}})}  << (8 & {4{P3IN[0]}});
wire [15:0] p3out_rd  = {8'h00, (p3out & {8{reg_rd[P3OUT]}})} << (8 & {4{P3OUT[0]}});
wire [15:0] p3dir_rd  = {8'h00, (p3dir & {8{reg_rd[P3DIR]}})} << (8 & {4{P3DIR[0]}});
wire [15:0] p3sel_rd  = {8'h00, (p3sel & {8{reg_rd[P3SEL]}})} << (8 & {4{P3SEL[0]}});
wire [15:0] p4in_rd   = {8'h00, (p4in  & {8{reg_rd[P4IN]}})}  << (8 & {4{P4IN[0]}});
wire [15:0] p4out_rd  = {8'h00, (p4out & {8{reg_rd[P4OUT]}})} << (8 & {4{P4OUT[0]}});
wire [15:0] p4dir_rd  = {8'h00, (p4dir & {8{reg_rd[P4DIR]}})} << (8 & {4{P4DIR[0]}});
wire [15:0] p4sel_rd  = {8'h00, (p4sel & {8{reg_rd[P4SEL]}})} << (8 & {4{P4SEL[0]}});
wire [15:0] p5in_rd   = {8'h00, (p5in  & {8{reg_rd[P5IN]}})}  << (8 & {4{P5IN[0]}});
wire [15:0] p5out_rd  = {8'h00, (p5out & {8{reg_rd[P5OUT]}})} << (8 & {4{P5OUT[0]}});
wire [15:0] p5dir_rd  = {8'h00, (p5dir & {8{reg_rd[P5DIR]}})} << (8 & {4{P5DIR[0]}});
wire [15:0] p5sel_rd  = {8'h00, (p5sel & {8{reg_rd[P5SEL]}})} << (8 & {4{P5SEL[0]}});
wire [15:0] p6in_rd   = {8'h00, (p6in  & {8{reg_rd[P6IN]}})}  << (8 & {4{P6IN[0]}});
wire [15:0] p6out_rd  = {8'h00, (p6out & {8{reg_rd[P6OUT]}})} << (8 & {4{P6OUT[0]}});
wire [15:0] p6dir_rd  = {8'h00, (p6dir & {8{reg_rd[P6DIR]}})} << (8 & {4{P6DIR[0]}});
wire [15:0] p6sel_rd  = {8'h00, (p6sel & {8{reg_rd[P6SEL]}})} << (8 & {4{P6SEL[0]}});

wire [15:0] per_dout  =  p1in_rd   |
                         p1out_rd  |
                         p1dir_rd  |
                         p1ifg_rd  |
                         p1ies_rd  |
                         p1ie_rd   |
                         p1sel_rd  |
                         p2in_rd   |
                         p2out_rd  |
                         p2dir_rd  |
                         p2ifg_rd  |
                         p2ies_rd  |
                         p2ie_rd   |
                         p2sel_rd  |
                         p3in_rd   |
                         p3out_rd  |
                         p3dir_rd  |
                         p3sel_rd  |
                         p4in_rd   |
                         p4out_rd  |
                         p4dir_rd  |
                         p4sel_rd  |
                         p5in_rd   |
                         p5out_rd  |
                         p5dir_rd  |
                         p5sel_rd  |
                         p6in_rd   |
                         p6out_rd  |
                         p6dir_rd  |
                         p6sel_rd;

endmodule // omsp_gpio
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_timerA.v
//
// *Module Description:
//                       Timer A top-level
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 204 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2015-07-08 22:34:10 +0200 (Wed, 08 Jul 2015) $
//----------------------------------------------------------------------------
`ifdef OMSP_TA_NO_INCLUDE
`else
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_timerA_defines.v
//
// *Module Description:
//                      omsp_timerA Configuration file
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 103 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2011-03-05 15:44:48 +0100 (Sat, 05 Mar 2011) $
//----------------------------------------------------------------------------
//`define OMSP_TA_NO_INCLUDE
`ifdef OMSP_TA_NO_INCLUDE
`else
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_timerA_defines.v
//
// *Module Description:
//                      omsp_timerA Configuration file
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 103 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2011-03-05 15:44:48 +0100 (Sat, 05 Mar 2011) $
//----------------------------------------------------------------------------
//`define OMSP_TA_NO_INCLUDE
`ifdef OMSP_TA_NO_INCLUDE
`else
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_timerA_undefines.v
//
// *Module Description:
//                      omsp_timerA Verilog `undef file
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 23 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2009-08-30 18:39:26 +0200 (Sun, 30 Aug 2009) $
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// SYSTEM CONFIGURATION
//----------------------------------------------------------------------------



//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//=====        SYSTEM CONSTANTS --- !!!!!!!! DO NOT EDIT !!!!!!!!      =====//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

// Timer A: TACTL Control Register
`ifdef TASSELx
`undef TASSELx
`endif
`ifdef TAIDx
`undef TAIDx
`endif
`ifdef TAMCx
`undef TAMCx
`endif
`ifdef TACLR
`undef TACLR
`endif
`ifdef TAIE
`undef TAIE
`endif
`ifdef TAIFG
`undef TAIFG
`endif

// Timer A: TACCTLx Capture/Compare Control Register
`ifdef TACMx
`undef TACMx
`endif
`ifdef TACCISx
`undef TACCISx
`endif
`ifdef TASCS
`undef TASCS
`endif
`ifdef TASCCI
`undef TASCCI
`endif
`ifdef TACAP
`undef TACAP
`endif
`ifdef TAOUTMODx
`undef TAOUTMODx
`endif
`ifdef TACCIE
`undef TACCIE
`endif
`ifdef TACCI
`undef TACCI
`endif
`ifdef TAOUT
`undef TAOUT
`endif
`ifdef TACOV
`undef TACOV
`endif
`ifdef TACCIFG
`undef TACCIFG
`endif
`endif

//----------------------------------------------------------------------------
// TIMER A CONFIGURATION
//----------------------------------------------------------------------------



//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//=====        SYSTEM CONSTANTS --- !!!!!!!! DO NOT EDIT !!!!!!!!      =====//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

// Timer A: TACTL Control Register
`define TASSELx     9:8
`define TAIDx       7:6
`define TAMCx       5:4
`define TACLR       2
`define TAIE        1
`define TAIFG       0

// Timer A: TACCTLx Capture/Compare Control Register
`define TACMx      15:14
`define TACCISx    13:12
`define TASCS      11
`define TASCCI     10
`define TACAP       8
`define TAOUTMODx   7:5
`define TACCIE      4
`define TACCI       3
`define TAOUT       2
`define TACOV       1
`define TACCIFG     0
`endif

//----------------------------------------------------------------------------
// TIMER A CONFIGURATION
//----------------------------------------------------------------------------



//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//=====        SYSTEM CONSTANTS --- !!!!!!!! DO NOT EDIT !!!!!!!!      =====//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

// Timer A: TACTL Control Register
`define TASSELx     9:8
`define TAIDx       7:6
`define TAMCx       5:4
`define TACLR       2
`define TAIE        1
`define TAIFG       0

// Timer A: TACCTLx Capture/Compare Control Register
`define TACMx      15:14
`define TACCISx    13:12
`define TASCS      11
`define TASCCI     10
`define TACAP       8
`define TAOUTMODx   7:5
`define TACCIE      4
`define TACCI       3
`define TAOUT       2
`define TACOV       1
`define TACCIFG     0

`endif

//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: template_periph_8b.v
//
// *Module Description:
//                       8 bit peripheral template.
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 134 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2012-03-22 21:31:06 +0100 (Thu, 22 Mar 2012) $
//----------------------------------------------------------------------------

module  template_periph_8b (

// OUTPUTs
    per_dout,                       // Peripheral data output

// INPUTs
    mclk,                           // Main system clock
    per_addr,                       // Peripheral address
    per_din,                        // Peripheral data input
    per_en,                         // Peripheral enable (high active)
    per_we,                         // Peripheral write enable (high active)
    puc_rst                         // Main system reset
);

// OUTPUTs
//=========
output      [15:0] per_dout;        // Peripheral data output

// INPUTs
//=========
input              mclk;            // Main system clock
input       [13:0] per_addr;        // Peripheral address
input       [15:0] per_din;         // Peripheral data input
input              per_en;          // Peripheral enable (high active)
input        [1:0] per_we;          // Peripheral write enable (high active)
input              puc_rst;         // Main system reset


//=============================================================================
// 1)  PARAMETER DECLARATION
//=============================================================================

// Register base address (must be aligned to decoder bit width)
parameter       [14:0] BASE_ADDR   = 15'h0090;

// Decoder bit width (defines how many bits are considered for address decoding)
parameter              DEC_WD      =  2;

// Register addresses offset
parameter [DEC_WD-1:0] CNTRL1      =  'h0,
                       CNTRL2      =  'h1,
                       CNTRL3      =  'h2,
                       CNTRL4      =  'h3;


// Register one-hot decoder utilities
parameter              DEC_SZ      =  (1 << DEC_WD);
parameter [DEC_SZ-1:0] BASE_REG    =  {{DEC_SZ-1{1'b0}}, 1'b1};

// Register one-hot decoder
parameter [DEC_SZ-1:0] CNTRL1_D  = (BASE_REG << CNTRL1),
                       CNTRL2_D  = (BASE_REG << CNTRL2),
                       CNTRL3_D  = (BASE_REG << CNTRL3),
                       CNTRL4_D  = (BASE_REG << CNTRL4);


//============================================================================
// 2)  REGISTER DECODER
//============================================================================

// Local register selection
wire              reg_sel      =  per_en & (per_addr[13:DEC_WD-1]==BASE_ADDR[14:DEC_WD]);

// Register local address
wire [DEC_WD-1:0] reg_addr     =  {1'b0, per_addr[DEC_WD-2:0]};

// Register address decode
wire [DEC_SZ-1:0] reg_dec      = (CNTRL1_D  &  {DEC_SZ{(reg_addr==(CNTRL1 >>1))}}) |
                                 (CNTRL2_D  &  {DEC_SZ{(reg_addr==(CNTRL2 >>1))}}) |
                                 (CNTRL3_D  &  {DEC_SZ{(reg_addr==(CNTRL3 >>1))}}) |
                                 (CNTRL4_D  &  {DEC_SZ{(reg_addr==(CNTRL4 >>1))}});

// Read/Write probes
wire              reg_lo_write =  per_we[0] & reg_sel;
wire              reg_hi_write =  per_we[1] & reg_sel;
wire              reg_read     = ~|per_we   & reg_sel;

// Read/Write vectors
wire [DEC_SZ-1:0] reg_hi_wr    = reg_dec & {DEC_SZ{reg_hi_write}};
wire [DEC_SZ-1:0] reg_lo_wr    = reg_dec & {DEC_SZ{reg_lo_write}};
wire [DEC_SZ-1:0] reg_rd       = reg_dec & {DEC_SZ{reg_read}};


//============================================================================
// 3) REGISTERS
//============================================================================

// CNTRL1 Register
//-----------------
reg  [7:0] cntrl1;

wire       cntrl1_wr  = CNTRL1[0] ? reg_hi_wr[CNTRL1] : reg_lo_wr[CNTRL1];
wire [7:0] cntrl1_nxt = CNTRL1[0] ? per_din[15:8]     : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl1 <=  8'h00;
  else if (cntrl1_wr) cntrl1 <=  cntrl1_nxt;


// CNTRL2 Register
//-----------------
reg  [7:0] cntrl2;

wire       cntrl2_wr  = CNTRL2[0] ? reg_hi_wr[CNTRL2] : reg_lo_wr[CNTRL2];
wire [7:0] cntrl2_nxt = CNTRL2[0] ? per_din[15:8]     : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl2 <=  8'h00;
  else if (cntrl2_wr) cntrl2 <=  cntrl2_nxt;


// CNTRL3 Register
//-----------------
reg  [7:0] cntrl3;

wire       cntrl3_wr  = CNTRL3[0] ? reg_hi_wr[CNTRL3] : reg_lo_wr[CNTRL3];
wire [7:0] cntrl3_nxt = CNTRL3[0] ? per_din[15:8]     : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl3 <=  8'h00;
  else if (cntrl3_wr) cntrl3 <=  cntrl3_nxt;


// CNTRL4 Register
//-----------------
reg  [7:0] cntrl4;

wire       cntrl4_wr  = CNTRL4[0] ? reg_hi_wr[CNTRL4] : reg_lo_wr[CNTRL4];
wire [7:0] cntrl4_nxt = CNTRL4[0] ? per_din[15:8]     : per_din[7:0];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl4 <=  8'h00;
  else if (cntrl4_wr) cntrl4 <=  cntrl4_nxt;



//============================================================================
// 4) DATA OUTPUT GENERATION
//============================================================================

// Data output mux
wire [15:0] cntrl1_rd   = {8'h00, (cntrl1  & {8{reg_rd[CNTRL1]}})}  << (8 & {4{CNTRL1[0]}});
wire [15:0] cntrl2_rd   = {8'h00, (cntrl2  & {8{reg_rd[CNTRL2]}})}  << (8 & {4{CNTRL2[0]}});
wire [15:0] cntrl3_rd   = {8'h00, (cntrl3  & {8{reg_rd[CNTRL3]}})}  << (8 & {4{CNTRL3[0]}});
wire [15:0] cntrl4_rd   = {8'h00, (cntrl4  & {8{reg_rd[CNTRL4]}})}  << (8 & {4{CNTRL4[0]}});

wire [15:0] per_dout  =  cntrl1_rd  |
                         cntrl2_rd  |
                         cntrl3_rd  |
                         cntrl4_rd;


endmodule // template_periph_8b
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: template_periph_16b.v
//
// *Module Description:
//                       16 bit peripheral template.
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 134 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2012-03-22 21:31:06 +0100 (Thu, 22 Mar 2012) $
//----------------------------------------------------------------------------

module  template_periph_16b (

// OUTPUTs
    per_dout,                       // Peripheral data output

// INPUTs
    mclk,                           // Main system clock
    per_addr,                       // Peripheral address
    per_din,                        // Peripheral data input
    per_en,                         // Peripheral enable (high active)
    per_we,                         // Peripheral write enable (high active)
    puc_rst                         // Main system reset
);

// OUTPUTs
//=========
output       [15:0] per_dout;       // Peripheral data output

// INPUTs
//=========
input               mclk;           // Main system clock
input        [13:0] per_addr;       // Peripheral address
input        [15:0] per_din;        // Peripheral data input
input               per_en;         // Peripheral enable (high active)
input         [1:0] per_we;         // Peripheral write enable (high active)
input               puc_rst;        // Main system reset


//=============================================================================
// 1)  PARAMETER DECLARATION
//=============================================================================

// Register base address (must be aligned to decoder bit width)
parameter       [14:0] BASE_ADDR   = 15'h0190;

// Decoder bit width (defines how many bits are considered for address decoding)
parameter              DEC_WD      =  3;

// Register addresses offset
parameter [DEC_WD-1:0] CNTRL1      = 'h0,
                       CNTRL2      = 'h2,
                       CNTRL3      = 'h4,
                       CNTRL4      = 'h6;

// Register one-hot decoder utilities
parameter              DEC_SZ      =  (1 << DEC_WD);
parameter [DEC_SZ-1:0] BASE_REG    =  {{DEC_SZ-1{1'b0}}, 1'b1};

// Register one-hot decoder
parameter [DEC_SZ-1:0] CNTRL1_D    = (BASE_REG << CNTRL1),
                       CNTRL2_D    = (BASE_REG << CNTRL2),
                       CNTRL3_D    = (BASE_REG << CNTRL3),
                       CNTRL4_D    = (BASE_REG << CNTRL4);


//============================================================================
// 2)  REGISTER DECODER
//============================================================================

// Local register selection
wire              reg_sel   =  per_en & (per_addr[13:DEC_WD-1]==BASE_ADDR[14:DEC_WD]);

// Register local address
wire [DEC_WD-1:0] reg_addr  =  {per_addr[DEC_WD-2:0], 1'b0};

// Register address decode
wire [DEC_SZ-1:0] reg_dec   =  (CNTRL1_D  &  {DEC_SZ{(reg_addr == CNTRL1 )}})  |
                               (CNTRL2_D  &  {DEC_SZ{(reg_addr == CNTRL2 )}})  |
                               (CNTRL3_D  &  {DEC_SZ{(reg_addr == CNTRL3 )}})  |
                               (CNTRL4_D  &  {DEC_SZ{(reg_addr == CNTRL4 )}});

// Read/Write probes
wire              reg_write =  |per_we & reg_sel;
wire              reg_read  = ~|per_we & reg_sel;

// Read/Write vectors
wire [DEC_SZ-1:0] reg_wr    = reg_dec & {DEC_SZ{reg_write}};
wire [DEC_SZ-1:0] reg_rd    = reg_dec & {DEC_SZ{reg_read}};


//============================================================================
// 3) REGISTERS
//============================================================================

// CNTRL1 Register
//-----------------
reg  [15:0] cntrl1;

wire        cntrl1_wr = reg_wr[CNTRL1];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl1 <=  16'h0000;
  else if (cntrl1_wr) cntrl1 <=  per_din;


// CNTRL2 Register
//-----------------
reg  [15:0] cntrl2;

wire        cntrl2_wr = reg_wr[CNTRL2];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl2 <=  16'h0000;
  else if (cntrl2_wr) cntrl2 <=  per_din;


// CNTRL3 Register
//-----------------
reg  [15:0] cntrl3;

wire        cntrl3_wr = reg_wr[CNTRL3];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl3 <=  16'h0000;
  else if (cntrl3_wr) cntrl3 <=  per_din;


// CNTRL4 Register
//-----------------
reg  [15:0] cntrl4;

wire        cntrl4_wr = reg_wr[CNTRL4];

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        cntrl4 <=  16'h0000;
  else if (cntrl4_wr) cntrl4 <=  per_din;


//============================================================================
// 4) DATA OUTPUT GENERATION
//============================================================================

// Data output mux
wire [15:0] cntrl1_rd  = cntrl1  & {16{reg_rd[CNTRL1]}};
wire [15:0] cntrl2_rd  = cntrl2  & {16{reg_rd[CNTRL2]}};
wire [15:0] cntrl3_rd  = cntrl3  & {16{reg_rd[CNTRL3]}};
wire [15:0] cntrl4_rd  = cntrl4  & {16{reg_rd[CNTRL4]}};

wire [15:0] per_dout   =  cntrl1_rd  |
                          cntrl2_rd  |
                          cntrl3_rd  |
                          cntrl4_rd;


endmodule // template_periph_16b
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: io_cell.v
//
// *Module Description:
//                       I/O cell model
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 103 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2011-03-05 15:44:48 +0100 (Sat, 05 Mar 2011) $
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Copyright (C) 2001 Authors
//
// This source file may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// This source file is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This source is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this source; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//----------------------------------------------------------------------------
//
// *File Name: msp_debug.v
//
// *Module Description:
//                      MSP430 core debug utility signals
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 134 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2012-03-22 21:31:06 +0100 (Thu, 22 Mar 2012) $
//----------------------------------------------------------------------------
`ifdef OMSP_NO_INCLUDE
`else
`include "openMSP430_defines.v"
`endif

module msp_debug (

// OUTPUTs
    e_state,                       // Execution state
    i_state,                       // Instruction fetch state
    inst_cycle,                    // Cycle number within current instruction
    inst_full,                     // Currently executed instruction (full version)
    inst_number,                   // Instruction number since last system reset
    inst_pc,                       // Instruction Program counter
    inst_short,                    // Currently executed instruction (short version)

// INPUTs
    mclk,                          // Main system clock
    puc_rst                        // Main system reset
);

// OUTPUTs
//============
output  [8*32-1:0] e_state;        // Execution state
output  [8*32-1:0] i_state;        // Instruction fetch state
output      [31:0] inst_cycle;     // Cycle number within current instruction
output  [8*32-1:0] inst_full;      // Currently executed instruction (full version)
output      [31:0] inst_number;    // Instruction number since last system reset
output      [15:0] inst_pc;        // Instruction Program counter
output  [8*32-1:0] inst_short;     // Currently executed instruction (short version)

// INPUTs
//============
input              mclk;           // Main system clock
input              puc_rst;        // Main system reset


//=============================================================================
// 1) ASCII FORMATING FUNCTIONS
//=============================================================================

// This function simply concatenates two strings together, ignorning the NULL
// at the end of string2.
// The specified number of space will be inserted between string1 and string2
function [64*8-1:0] myFormat;

  input [32*8-1:0] string1;
  input [32*8-1:0] string2;
  input      [3:0] space;

  integer i,j;
  begin
     myFormat = 0;
`ifdef VXL			// no +:
`else
     j        = 0;
     for ( i=0; i < 32; i=i+1)                      // Copy string2
       begin
	  myFormat[8*i +: 8] = string2[8*i +: 8];
	  if ((string2[8*i +: 8] == 0) && (j == 0)) j=i;
       end

     for ( i=0; i < space; i=i+1)                   // Add spaces
       myFormat[8*(j+i) +: 8] = " ";
     j=j+space;

     for ( i=0; i < 32; i=i+1)                      // Copy string1
       myFormat[8*(j+i) +: 8] = string1[8*i +: 8];
`endif
  end
endfunction


//=============================================================================
// 2) CONNECTIONS TO MSP430 CORE INTERNALS
//=============================================================================

wire  [2:0] i_state_bin = tb_openMSP430.dut.frontend_0.i_state;
wire  [3:0] e_state_bin = tb_openMSP430.dut.frontend_0.e_state;

wire        decode      = tb_openMSP430.dut.frontend_0.decode;
wire [15:0] ir          = tb_openMSP430.dut.frontend_0.ir;
wire        irq_detect  = tb_openMSP430.dut.frontend_0.irq_detect;
wire  [3:0] irq_num     = tb_openMSP430.dut.frontend_0.irq_num;
wire [15:0] pc          = tb_openMSP430.dut.frontend_0.pc;


//=============================================================================
// 3) GENERATE DEBUG SIGNALS
//=============================================================================

// Instruction fetch state
//=========================
reg [8*32-1:0] i_state;

always @(i_state_bin)
    case(i_state_bin)
      3'h0    : i_state =  "IRQ_FETCH";
      3'h1    : i_state =  "IRQ_DONE";
      3'h2    : i_state =  "DEC";
      3'h3    : i_state =  "EXT1";
      3'h4    : i_state =  "EXT2";
      3'h5    : i_state =  "IDLE";
      default : i_state =  "XXXXX";
    endcase


// Execution state
//=========================

reg [8*32-1:0] e_state;

always @(e_state_bin)
    case(e_state_bin)
      4'h2    : e_state =  "IRQ_0";
      4'h1    : e_state =  "IRQ_1";
      4'h0    : e_state =  "IRQ_2";
      4'h3    : e_state =  "IRQ_3";
      4'h4    : e_state =  "IRQ_4";
      4'h5    : e_state =  "SRC_AD";
      4'h6    : e_state =  "SRC_RD";
      4'h7    : e_state =  "SRC_WR";
      4'h8    : e_state =  "DST_AD";
      4'h9    : e_state =  "DST_RD";
      4'hA    : e_state =  "DST_WR";
      4'hB    : e_state =  "EXEC";
      4'hC    : e_state =  "JUMP";
      4'hD    : e_state =  "IDLE";
      default : e_state =  "xxxx";
    endcase


// Count instruction number & cycles
//====================================

reg [31:0]  inst_number;
always @(posedge mclk or posedge puc_rst)
  if (puc_rst)     inst_number  <= 0;
  else if (decode) inst_number  <= inst_number+1;

reg [31:0]  inst_cycle;
always @(posedge mclk or posedge puc_rst)
  if (puc_rst)     inst_cycle <= 0;
  else if (decode) inst_cycle <= 0;
  else             inst_cycle <= inst_cycle+1;


// Decode instruction
//====================================

// Buffer opcode
reg [15:0]  opcode;
always @(posedge mclk or posedge puc_rst)
  if (puc_rst)     opcode  <= 0;
  else if (decode) opcode  <= ir;

// Interrupts
reg irq;
always @(posedge mclk or posedge puc_rst)
  if (puc_rst)     irq     <= 1'b1;
  else if (decode) irq     <= irq_detect;

// Instruction type
reg [8*32-1:0] inst_type;
always @(opcode or irq)
  if (irq)
    inst_type =  "IRQ";
  else
    case(opcode[15:13])
      3'b000  : inst_type =  "SIG-OP";
      3'b001  : inst_type =  "JUMP";
      default : inst_type =  "TWO-OP";
    endcase


// Instructions name
reg [8*32-1:0] inst_name;
always @(opcode or inst_type or irq_num)
  if (inst_type=="IRQ")
    case(irq_num[3:0])
      4'b0000        : inst_name =  "IRQ 0";
      4'b0001        : inst_name =  "IRQ 1";
      4'b0010        : inst_name =  "IRQ 2";
      4'b0011        : inst_name =  "IRQ 3";
      4'b0100        : inst_name =  "IRQ 4";
      4'b0101        : inst_name =  "IRQ 5";
      4'b0110        : inst_name =  "IRQ 6";
      4'b0111        : inst_name =  "IRQ 7";
      4'b1000        : inst_name =  "IRQ 8";
      4'b1001        : inst_name =  "IRQ 9";
      4'b1010        : inst_name =  "IRQ 10";
      4'b1011        : inst_name =  "IRQ 11";
      4'b1100        : inst_name =  "IRQ 12";
      4'b1101        : inst_name =  "IRQ 13";
      4'b1110        : inst_name =  "NMI";
      default        : inst_name =  "RESET";
    endcase
  else if (inst_type=="SIG-OP")
    case(opcode[15:7])
      9'b000100_000  : inst_name =  "RRC";
      9'b000100_001  : inst_name =  "SWPB";
      9'b000100_010  : inst_name =  "RRA";
      9'b000100_011  : inst_name =  "SXT";
      9'b000100_100  : inst_name =  "PUSH";
      9'b000100_101  : inst_name =  "CALL";
      9'b000100_110  : inst_name =  "RETI";
      default        : inst_name =  "xxxx";
    endcase
  else if (inst_type=="JUMP")
    case(opcode[15:10])
      6'b001_000     : inst_name =  "JNE";
      6'b001_001     : inst_name =  "JEQ";
      6'b001_010     : inst_name =  "JNC";
      6'b001_011     : inst_name =  "JC";
      6'b001_100     : inst_name =  "JN";
      6'b001_101     : inst_name =  "JGE";
      6'b001_110     : inst_name =  "JL";
      6'b001_111     : inst_name =  "JMP";
      default        : inst_name =  "xxxx";
    endcase
  else if (inst_type=="TWO-OP")
    case(opcode[15:12])
      4'b0100        : inst_name =  "MOV";
      4'b0101        : inst_name =  "ADD";
      4'b0110        : inst_name =  "ADDC";
      4'b0111        : inst_name =  "SUBC";
      4'b1000        : inst_name =  "SUB";
      4'b1001        : inst_name =  "CMP";
      4'b1010        : inst_name =  "DADD";
      4'b1011        : inst_name =  "BIT";
      4'b1100        : inst_name =  "BIC";
      4'b1101        : inst_name =  "BIS";
      4'b1110        : inst_name =  "XOR";
      4'b1111        : inst_name =  "AND";
      default        : inst_name =  "xxxx";
    endcase

// Instructions byte/word mode
reg [8*32-1:0] inst_bw;
always @(opcode or inst_type)
  if (inst_type=="IRQ")
    inst_bw =  "";
  else if (inst_type=="SIG-OP")
    inst_bw =  opcode[6] ? ".B" : "";
  else if (inst_type=="JUMP")
    inst_bw =  "";
  else if (inst_type=="TWO-OP")
    inst_bw =  opcode[6] ? ".B" : "";

// Source register
reg [8*32-1:0] inst_src;
wire     [3:0] src_reg = (inst_type=="SIG-OP") ? opcode[3:0] : opcode[11:8];

always @(src_reg or inst_type)
  if (inst_type=="IRQ")
    inst_src =  "";
  else if (inst_type=="JUMP")
    inst_src =  "";
  else if ((inst_type=="SIG-OP") || (inst_type=="TWO-OP"))
    case(src_reg)
      4'b0000 : inst_src =  "r0";
      4'b0001 : inst_src =  "r1";
      4'b0010 : inst_src =  "r2";
      4'b0011 : inst_src =  "r3";
      4'b0100 : inst_src =  "r4";
      4'b0101 : inst_src =  "r5";
      4'b0110 : inst_src =  "r6";
      4'b0111 : inst_src =  "r7";
      4'b1000 : inst_src =  "r8";
      4'b1001 : inst_src =  "r9";
      4'b1010 : inst_src =  "r10";
      4'b1011 : inst_src =  "r11";
      4'b1100 : inst_src =  "r12";
      4'b1101 : inst_src =  "r13";
      4'b1110 : inst_src =  "r14";
      default : inst_src =  "r15";
    endcase

// Destination register
reg [8*32-1:0] inst_dst;
always @(opcode or inst_type)
  if (inst_type=="IRQ")
    inst_dst =  "";
  else if (inst_type=="SIG-OP")
    inst_dst =  "";
  else if (inst_type=="JUMP")
    inst_dst =  "";
  else if (inst_type=="TWO-OP")
    case(opcode[3:0])
      4'b0000 : inst_dst =  "r0";
      4'b0001 : inst_dst =  "r1";
      4'b0010 : inst_dst =  "r2";
      4'b0011 : inst_dst =  "r3";
      4'b0100 : inst_dst =  "r4";
      4'b0101 : inst_dst =  "r5";
      4'b0110 : inst_dst =  "r6";
      4'b0111 : inst_dst =  "r7";
      4'b1000 : inst_dst =  "r8";
      4'b1001 : inst_dst =  "r9";
      4'b1010 : inst_dst =  "r10";
      4'b1011 : inst_dst =  "r11";
      4'b1100 : inst_dst =  "r12";
      4'b1101 : inst_dst =  "r13";
      4'b1110 : inst_dst =  "r14";
      default : inst_dst =  "r15";
    endcase

// Source Addressing mode
reg [8*32-1:0] inst_as;
always @(inst_type or src_reg or opcode or inst_src)
  begin
  if (inst_type=="IRQ")
    inst_as =  "";
  else if (inst_type=="JUMP")
    inst_as =  "";
  else if (src_reg==4'h3) // Addressing mode using R3
    case (opcode[5:4])
      2'b11  : inst_as =  "#-1";
      2'b10  : inst_as =  "#2";
      2'b01  : inst_as =  "#1";
      default: inst_as =  "#0";
    endcase
  else if (src_reg==4'h2) // Addressing mode using R2
    case (opcode[5:4])
      2'b11  : inst_as =  "#8";
      2'b10  : inst_as =  "#4";
      2'b01  : inst_as =  "&EDE";
      default: inst_as =  inst_src;
    endcase
  else if (src_reg==4'h0) // Addressing mode using R0
    case (opcode[5:4])
      2'b11  : inst_as =  "#N";
      2'b10  : inst_as =  myFormat("@", inst_src, 0);
      2'b01  : inst_as =  "EDE";
      default: inst_as =  inst_src;
    endcase
  else                    // General Addressing mode
    case (opcode[5:4])
      2'b11  : begin
	       inst_as =  myFormat("@", inst_src, 0);
	       inst_as =  myFormat(inst_as, "+", 0);
               end
      2'b10  : inst_as =  myFormat("@", inst_src, 0);
      2'b01  : begin
	       inst_as =  myFormat("x(", inst_src, 0);
  	       inst_as =  myFormat(inst_as, ")", 0);
               end
      default: inst_as =  inst_src;
    endcase
  end

// Destination Addressing mode
reg [8*32-1:0] inst_ad;
always @(opcode or inst_type or inst_dst)
  begin
     if (inst_type!="TWO-OP")
       inst_ad =  "";
     else if (opcode[3:0]==4'h2)   // Addressing mode using R2
       case (opcode[7])
	 1'b1   : inst_ad =  "&EDE";
	 default: inst_ad =  inst_dst;
       endcase
     else if (opcode[3:0]==4'h0)   // Addressing mode using R0
       case (opcode[7])
	 2'b1   : inst_ad =  "EDE";
	 default: inst_ad =  inst_dst;
       endcase
     else                          // General Addressing mode
       case (opcode[7])
	 2'b1   : begin
	          inst_ad =  myFormat("x(", inst_dst, 0);
  	          inst_ad =  myFormat(inst_ad, ")", 0);
                  end
	 default: inst_ad =  inst_dst;
       endcase
  end


// Currently executed instruction
//================================

wire [8*32-1:0] inst_short = inst_name;

reg  [8*32-1:0] inst_full;
always @(inst_type or inst_name or inst_bw or inst_as or inst_ad)
  begin
     inst_full   = myFormat(inst_name, inst_bw, 0);
     inst_full   = myFormat(inst_full, inst_as, 1);
     if (inst_type=="TWO-OP")
       inst_full = myFormat(inst_full, ",",     0);
     inst_full   = myFormat(inst_full, inst_ad, 1);
     if (opcode==16'h4303)
       inst_full = "NOP";
     if (opcode==`DBG_SWBRK_OP)
       inst_full = "SBREAK";

  end


// Instruction program counter
//================================

reg  [15:0] inst_pc;
always @(posedge mclk or posedge puc_rst)
  if (puc_rst)     inst_pc  <=  16'h0000;
  else if (decode) inst_pc  <=  pc;


endmodule // msp_debug

//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_timerA.v
//
// *Module Description:
//                       Timer A top-level
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 204 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2015-07-08 22:34:10 +0200 (Wed, 08 Jul 2015) $
//----------------------------------------------------------------------------
`ifdef OMSP_TA_NO_INCLUDE
`else
//----------------------------------------------------------------------------
// Copyright (C) 2009 , Olivier Girard
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the authors nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE
//
//----------------------------------------------------------------------------
//
// *File Name: omsp_timerA_undefines.v
//
// *Module Description:
//                      omsp_timerA Verilog `undef file
//
// *Author(s):
//              - Olivier Girard,    olgirard@gmail.com
//
//----------------------------------------------------------------------------
// $Rev: 23 $
// $LastChangedBy: olivier.girard $
// $LastChangedDate: 2009-08-30 18:39:26 +0200 (Sun, 30 Aug 2009) $
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// SYSTEM CONFIGURATION
//----------------------------------------------------------------------------



//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//=====        SYSTEM CONSTANTS --- !!!!!!!! DO NOT EDIT !!!!!!!!      =====//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

// Timer A: TACTL Control Register
`ifdef TASSELx
`undef TASSELx
`endif
`ifdef TAIDx
`undef TAIDx
`endif
`ifdef TAMCx
`undef TAMCx
`endif
`ifdef TACLR
`undef TACLR
`endif
`ifdef TAIE
`undef TAIE
`endif
`ifdef TAIFG
`undef TAIFG
`endif

// Timer A: TACCTLx Capture/Compare Control Register
`ifdef TACMx
`undef TACMx
`endif
`ifdef TACCISx
`undef TACCISx
`endif
`ifdef TASCS
`undef TASCS
`endif
`ifdef TASCCI
`undef TASCCI
`endif
`ifdef TACAP
`undef TACAP
`endif
`ifdef TAOUTMODx
`undef TAOUTMODx
`endif
`ifdef TACCIE
`undef TACCIE
`endif
`ifdef TACCI
`undef TACCI
`endif
`ifdef TAOUT
`undef TAOUT
`endif
`ifdef TACOV
`undef TACOV
`endif
`ifdef TACCIFG
`undef TACCIFG
`endif
`endif

module  omsp_timerA (

// OUTPUTs
    irq_ta0,                        // Timer A interrupt: TACCR0
    irq_ta1,                        // Timer A interrupt: TAIV, TACCR1, TACCR2
    per_dout,                       // Peripheral data output
    ta_out0,                        // Timer A output 0
    ta_out0_en,                     // Timer A output 0 enable
    ta_out1,                        // Timer A output 1
    ta_out1_en,                     // Timer A output 1 enable
    ta_out2,                        // Timer A output 2
    ta_out2_en,                     // Timer A output 2 enable

// INPUTs
    aclk_en,                        // ACLK enable (from CPU)
    dbg_freeze,                     // Freeze Timer A counter
    inclk,                          // INCLK external timer clock (SLOW)
    irq_ta0_acc,                    // Interrupt request TACCR0 accepted
    mclk,                           // Main system clock
    per_addr,                       // Peripheral address
    per_din,                        // Peripheral data input
    per_en,                         // Peripheral enable (high active)
    per_we,                         // Peripheral write enable (high active)
    puc_rst,                        // Main system reset
    smclk_en,                       // SMCLK enable (from CPU)
    ta_cci0a,                       // Timer A capture 0 input A
    ta_cci0b,                       // Timer A capture 0 input B
    ta_cci1a,                       // Timer A capture 1 input A
    ta_cci1b,                       // Timer A capture 1 input B
    ta_cci2a,                       // Timer A capture 2 input A
    ta_cci2b,                       // Timer A capture 2 input B
    taclk                           // TACLK external timer clock (SLOW)
);

// OUTPUTs
//=========
output              irq_ta0;        // Timer A interrupt: TACCR0
output              irq_ta1;        // Timer A interrupt: TAIV, TACCR1, TACCR2
output       [15:0] per_dout;       // Peripheral data output
output              ta_out0;        // Timer A output 0
output              ta_out0_en;     // Timer A output 0 enable
output              ta_out1;        // Timer A output 1
output              ta_out1_en;     // Timer A output 1 enable
output              ta_out2;        // Timer A output 2
output              ta_out2_en;     // Timer A output 2 enable

// INPUTs
//=========
input               aclk_en;        // ACLK enable (from CPU)
input               dbg_freeze;     // Freeze Timer A counter
input               inclk;          // INCLK external timer clock (SLOW)
input               irq_ta0_acc;    // Interrupt request TACCR0 accepted
input               mclk;           // Main system clock
input        [13:0] per_addr;       // Peripheral address
input        [15:0] per_din;        // Peripheral data input
input               per_en;         // Peripheral enable (high active)
input         [1:0] per_we;         // Peripheral write enable (high active)
input               puc_rst;        // Main system reset
input               smclk_en;       // SMCLK enable (from CPU)
input               ta_cci0a;       // Timer A capture 0 input A
input               ta_cci0b;       // Timer A capture 0 input B
input               ta_cci1a;       // Timer A capture 1 input A
input               ta_cci1b;       // Timer A capture 1 input B
input               ta_cci2a;       // Timer A capture 2 input A
input               ta_cci2b;       // Timer A capture 2 input B
input               taclk;          // TACLK external timer clock (SLOW)


//=============================================================================
// 1)  PARAMETER DECLARATION
//=============================================================================

// Register base address (must be aligned to decoder bit width)
parameter       [14:0] BASE_ADDR  = 15'h0100;

// Decoder bit width (defines how many bits are considered for address decoding)
parameter              DEC_WD     =  7;

// Register addresses offset
parameter [DEC_WD-1:0] TACTL      = 'h60,
                       TAR        = 'h70,
                       TACCTL0    = 'h62,
                       TACCR0     = 'h72,
                       TACCTL1    = 'h64,
                       TACCR1     = 'h74,
                       TACCTL2    = 'h66,
                       TACCR2     = 'h76,
                       TAIV       = 'h2E;

// Register one-hot decoder utilities
parameter              DEC_SZ     =  (1 << DEC_WD);
parameter [DEC_SZ-1:0] BASE_REG   =  {{DEC_SZ-1{1'b0}}, 1'b1};

// Register one-hot decoder
parameter [DEC_SZ-1:0] TACTL_D    = (BASE_REG << TACTL),
                       TAR_D      = (BASE_REG << TAR),
                       TACCTL0_D  = (BASE_REG << TACCTL0),
                       TACCR0_D   = (BASE_REG << TACCR0),
                       TACCTL1_D  = (BASE_REG << TACCTL1),
                       TACCR1_D   = (BASE_REG << TACCR1),
                       TACCTL2_D  = (BASE_REG << TACCTL2),
                       TACCR2_D   = (BASE_REG << TACCR2),
                       TAIV_D     = (BASE_REG << TAIV);


//============================================================================
// 2)  REGISTER DECODER
//============================================================================

// Local register selection
wire              reg_sel   =  per_en & (per_addr[13:DEC_WD-1]==BASE_ADDR[14:DEC_WD]);

// Register local address
wire [DEC_WD-1:0] reg_addr  =  {per_addr[DEC_WD-2:0], 1'b0};

// Register address decode
wire [DEC_SZ-1:0] reg_dec   =  (TACTL_D    &  {DEC_SZ{(reg_addr == TACTL   )}})  |
                               (TAR_D      &  {DEC_SZ{(reg_addr == TAR     )}})  |
                               (TACCTL0_D  &  {DEC_SZ{(reg_addr == TACCTL0 )}})  |
                               (TACCR0_D   &  {DEC_SZ{(reg_addr == TACCR0  )}})  |
                               (TACCTL1_D  &  {DEC_SZ{(reg_addr == TACCTL1 )}})  |
                               (TACCR1_D   &  {DEC_SZ{(reg_addr == TACCR1  )}})  |
                               (TACCTL2_D  &  {DEC_SZ{(reg_addr == TACCTL2 )}})  |
                               (TACCR2_D   &  {DEC_SZ{(reg_addr == TACCR2  )}})  |
                               (TAIV_D     &  {DEC_SZ{(reg_addr == TAIV    )}});

// Read/Write probes
wire              reg_write =  |per_we & reg_sel;
wire              reg_read  = ~|per_we & reg_sel;

// Read/Write vectors
wire [DEC_SZ-1:0] reg_wr    = reg_dec & {512{reg_write}};
wire [DEC_SZ-1:0] reg_rd    = reg_dec & {512{reg_read}};


//============================================================================
// 3) REGISTERS
//============================================================================

// TACTL Register
//-----------------
reg   [9:0] tactl;

wire        tactl_wr = reg_wr[TACTL];
wire        taclr    = tactl_wr & per_din[TACLR];
wire        taifg_set;
wire        taifg_clr;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       tactl <=  10'h000;
  else if (tactl_wr) tactl <=  ((per_din[9:0] & 10'h3f3) | {9'h000, taifg_set}) & {9'h1ff, ~taifg_clr};
  else               tactl <=  (tactl                    | {9'h000, taifg_set}) & {9'h1ff, ~taifg_clr};


// TAR Register
//-----------------
reg  [15:0] tar;

wire        tar_wr = reg_wr[TAR];

wire        tar_clk;
wire        tar_clr;
wire        tar_inc;
wire        tar_dec;
wire [15:0] tar_add  = tar_inc ? 16'h0001 :
                       tar_dec ? 16'hffff : 16'h0000;
wire [15:0] tar_nxt  = tar_clr ? 16'h0000 : (tar+tar_add);

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)                     tar <=  16'h0000;
  else if  (tar_wr)                tar <=  per_din;
  else if  (taclr)                 tar <=  16'h0000;
  else if  (tar_clk & ~dbg_freeze) tar <=  tar_nxt;


// TACCTL0 Register
//------------------
reg  [15:0] tacctl0;

wire        tacctl0_wr = reg_wr[TACCTL0];
wire        ccifg0_set;
wire        cov0_set;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)         tacctl0  <=  16'h0000;
  else if (tacctl0_wr) tacctl0  <=  ((per_din & 16'hf9f7) | {14'h0000, cov0_set, ccifg0_set}) & {15'h7fff, ~irq_ta0_acc};
  else                 tacctl0  <=  (tacctl0              | {14'h0000, cov0_set, ccifg0_set}) & {15'h7fff, ~irq_ta0_acc};

wire        cci0;
wire        cci0_s;
reg         scci0;
wire [15:0] tacctl0_full = tacctl0 | {5'h00, scci0, 6'h00, cci0_s, 3'h0};


// TACCR0 Register
//------------------
reg  [15:0] taccr0;

wire        taccr0_wr = reg_wr[TACCR0];
wire        cci0_cap;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        taccr0 <=  16'h0000;
  else if (taccr0_wr) taccr0 <=  per_din;
  else if (cci0_cap)  taccr0 <=  tar;


// TACCTL1 Register
//------------------
reg  [15:0] tacctl1;

wire        tacctl1_wr = reg_wr[TACCTL1];
wire        ccifg1_set;
wire        ccifg1_clr;
wire        cov1_set;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)         tacctl1 <=  16'h0000;
  else if (tacctl1_wr) tacctl1 <=  ((per_din & 16'hf9f7) | {14'h0000, cov1_set, ccifg1_set}) & {15'h7fff, ~ccifg1_clr};
  else                 tacctl1 <=  (tacctl1              | {14'h0000, cov1_set, ccifg1_set}) & {15'h7fff, ~ccifg1_clr};

wire        cci1;
wire        cci1_s;
reg         scci1;
wire [15:0] tacctl1_full = tacctl1 | {5'h00, scci1, 6'h00, cci1_s, 3'h0};


// TACCR1 Register
//------------------
reg  [15:0] taccr1;

wire        taccr1_wr = reg_wr[TACCR1];
wire        cci1_cap;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        taccr1 <=  16'h0000;
  else if (taccr1_wr) taccr1 <=  per_din;
  else if (cci1_cap)  taccr1 <=  tar;


// TACCTL2 Register
//------------------
reg  [15:0] tacctl2;

wire        tacctl2_wr = reg_wr[TACCTL2];
wire        ccifg2_set;
wire        ccifg2_clr;
wire        cov2_set;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)         tacctl2 <=  16'h0000;
  else if (tacctl2_wr) tacctl2 <=  ((per_din & 16'hf9f7) | {14'h0000, cov2_set, ccifg2_set}) & {15'h7fff, ~ccifg2_clr};
  else                 tacctl2 <=  (tacctl2              | {14'h0000, cov2_set, ccifg2_set}) & {15'h7fff, ~ccifg2_clr};

wire        cci2;
wire        cci2_s;
reg         scci2;
wire [15:0] tacctl2_full = tacctl2 | {5'h00, scci2, 6'h00, cci2_s, 3'h0};


// TACCR2 Register
//------------------
reg  [15:0] taccr2;

wire        taccr2_wr = reg_wr[TACCR2];
wire        cci2_cap;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)        taccr2 <=  16'h0000;
  else if (taccr2_wr) taccr2 <=  per_din;
  else if (cci2_cap)  taccr2 <=  tar;


// TAIV Register
//------------------

wire [3:0] taiv = (tacctl1[`TACCIFG] & tacctl1[`TACCIE]) ? 4'h2 :
                  (tacctl2[`TACCIFG] & tacctl2[`TACCIE]) ? 4'h4 :
                  (tactl[`TAIFG]     & tactl[`TAIE])     ? 4'hA :
                                                           4'h0;

assign     ccifg1_clr = (reg_rd[TAIV] | reg_wr[TAIV]) & (taiv==4'h2);
assign     ccifg2_clr = (reg_rd[TAIV] | reg_wr[TAIV]) & (taiv==4'h4);
assign     taifg_clr  = (reg_rd[TAIV] | reg_wr[TAIV]) & (taiv==4'hA);


//============================================================================
// 4) DATA OUTPUT GENERATION
//============================================================================

// Data output mux
wire [15:0] tactl_rd   = {6'h00, tactl}  & {16{reg_rd[TACTL]}};
wire [15:0] tar_rd     = tar             & {16{reg_rd[TAR]}};
wire [15:0] tacctl0_rd = tacctl0_full    & {16{reg_rd[TACCTL0]}};
wire [15:0] taccr0_rd  = taccr0          & {16{reg_rd[TACCR0]}};
wire [15:0] tacctl1_rd = tacctl1_full    & {16{reg_rd[TACCTL1]}};
wire [15:0] taccr1_rd  = taccr1          & {16{reg_rd[TACCR1]}};
wire [15:0] tacctl2_rd = tacctl2_full    & {16{reg_rd[TACCTL2]}};
wire [15:0] taccr2_rd  = taccr2          & {16{reg_rd[TACCR2]}};
wire [15:0] taiv_rd    = {12'h000, taiv} & {16{reg_rd[TAIV]}};

wire [15:0] per_dout   =  tactl_rd   |
                          tar_rd     |
                          tacctl0_rd |
                          taccr0_rd  |
                          tacctl1_rd |
                          taccr1_rd  |
                          tacctl2_rd |
                          taccr2_rd  |
                          taiv_rd;


//============================================================================
// 5) Timer A counter control
//============================================================================

// Clock input synchronization (TACLK & INCLK)
//-----------------------------------------------------------
wire taclk_s;
wire inclk_s;

omsp_sync_cell sync_cell_taclk (
    .data_out  (taclk_s),
    .data_in   (taclk),
    .clk       (mclk),
    .rst       (puc_rst)
);

omsp_sync_cell sync_cell_inclk (
    .data_out  (inclk_s),
    .data_in   (inclk),
    .clk       (mclk),
    .rst       (puc_rst)
);


// Clock edge detection (TACLK & INCLK)
//-----------------------------------------------------------

reg  taclk_dly;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst) taclk_dly <=  1'b0;
  else         taclk_dly <=  taclk_s;

wire taclk_en = taclk_s & ~taclk_dly;


reg  inclk_dly;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst) inclk_dly <=  1'b0;
  else         inclk_dly <=  inclk_s;

wire inclk_en = inclk_s & ~inclk_dly;


// Timer clock input mux
//-----------------------------------------------------------

wire sel_clk = (tactl[`TASSELx]==2'b00) ? taclk_en :
               (tactl[`TASSELx]==2'b01) ?  aclk_en :
               (tactl[`TASSELx]==2'b10) ? smclk_en : inclk_en;


// Generate update pluse for the counter (<=> divided clock)
//-----------------------------------------------------------
reg [2:0] clk_div;

assign    tar_clk = sel_clk & ((tactl[`TAIDx]==2'b00) ?  1'b1         :
                               (tactl[`TAIDx]==2'b01) ?  clk_div[0]   :
                               (tactl[`TAIDx]==2'b10) ? &clk_div[1:0] :
                                                        &clk_div[2:0]);

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)                               clk_div <=  3'h0;
  else if  (tar_clk | taclr)                 clk_div <=  3'h0;
  else if ((tactl[`TAMCx]!=2'b00) & sel_clk) clk_div <=  clk_div+3'h1;


// Time counter control signals
//-----------------------------------------------------------

assign  tar_clr   = ((tactl[`TAMCx]==2'b01) & (tar>=taccr0))         |
                    ((tactl[`TAMCx]==2'b11) & (taccr0==16'h0000));

assign  tar_inc   =  (tactl[`TAMCx]==2'b01) | (tactl[`TAMCx]==2'b10) |
                    ((tactl[`TAMCx]==2'b11) & ~tar_dec);

reg tar_dir;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)                        tar_dir <=  1'b0;
  else if (taclr)                     tar_dir <=  1'b0;
  else if (tactl[`TAMCx]==2'b11)
    begin
       if (tar_clk & (tar==16'h0001)) tar_dir <=  1'b0;
       else if       (tar>=taccr0)    tar_dir <=  1'b1;
    end
  else                                tar_dir <=  1'b0;

assign tar_dec = tar_dir | ((tactl[`TAMCx]==2'b11) & (tar>=taccr0));


//============================================================================
// 6) Timer A comparator
//============================================================================

wire equ0 = (tar_nxt==taccr0) & (tar!=taccr0);
wire equ1 = (tar_nxt==taccr1) & (tar!=taccr1);
wire equ2 = (tar_nxt==taccr2) & (tar!=taccr2);


//============================================================================
// 7) Timer A capture logic
//============================================================================

// Input selection
//------------------
assign cci0 = (tacctl0[`TACCISx]==2'b00) ? ta_cci0a :
              (tacctl0[`TACCISx]==2'b01) ? ta_cci0b :
              (tacctl0[`TACCISx]==2'b10) ?     1'b0 : 1'b1;

assign cci1 = (tacctl1[`TACCISx]==2'b00) ? ta_cci1a :
              (tacctl1[`TACCISx]==2'b01) ? ta_cci1b :
              (tacctl1[`TACCISx]==2'b10) ?     1'b0 : 1'b1;

assign cci2 = (tacctl2[`TACCISx]==2'b00) ? ta_cci2a :
              (tacctl2[`TACCISx]==2'b01) ? ta_cci2b :
              (tacctl2[`TACCISx]==2'b10) ?     1'b0 : 1'b1;

// CCIx synchronization
omsp_sync_cell sync_cell_cci0 (
    .data_out (cci0_s),
    .data_in  (cci0),
    .clk      (mclk),
    .rst      (puc_rst)
);
omsp_sync_cell sync_cell_cci1 (
    .data_out (cci1_s),
    .data_in  (cci1),
    .clk      (mclk),
    .rst      (puc_rst)
);
omsp_sync_cell sync_cell_cci2 (
    .data_out (cci2_s),
    .data_in  (cci2),
    .clk      (mclk),
    .rst      (puc_rst)
);

// Register CCIx for edge detection
reg cci0_dly;
reg cci1_dly;
reg cci2_dly;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)
    begin
       cci0_dly <=  1'b0;
       cci1_dly <=  1'b0;
       cci2_dly <=  1'b0;
    end
  else
    begin
       cci0_dly <=  cci0_s;
       cci1_dly <=  cci1_s;
       cci2_dly <=  cci2_s;
    end


// Generate SCCIx
//------------------

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)             scci0 <=  1'b0;
  else if (tar_clk & equ0) scci0 <=  cci0_s;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)             scci1 <=  1'b0;
  else if (tar_clk & equ1) scci1 <=  cci1_s;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)             scci2 <=  1'b0;
  else if (tar_clk & equ2) scci2 <=  cci2_s;


// Capture mode
//------------------
wire cci0_evt = (tacctl0[`TACMx]==2'b00) ? 1'b0                  :
                (tacctl0[`TACMx]==2'b01) ? ( cci0_s & ~cci0_dly) :   // Rising edge
                (tacctl0[`TACMx]==2'b10) ? (~cci0_s &  cci0_dly) :   // Falling edge
                                           ( cci0_s ^  cci0_dly);    // Both edges

wire cci1_evt = (tacctl1[`TACMx]==2'b00) ? 1'b0                  :
                (tacctl1[`TACMx]==2'b01) ? ( cci1_s & ~cci1_dly) :   // Rising edge
                (tacctl1[`TACMx]==2'b10) ? (~cci1_s &  cci1_dly) :   // Falling edge
                                           ( cci1_s ^  cci1_dly);    // Both edges

wire cci2_evt = (tacctl2[`TACMx]==2'b00) ? 1'b0                  :
                (tacctl2[`TACMx]==2'b01) ? ( cci2_s & ~cci2_dly) :   // Rising edge
                (tacctl2[`TACMx]==2'b10) ? (~cci2_s &  cci2_dly) :   // Falling edge
                                           ( cci2_s ^  cci2_dly);    // Both edges

// Event Synchronization
//-----------------------

reg cci0_evt_s;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       cci0_evt_s <=  1'b0;
  else if (tar_clk)  cci0_evt_s <=  1'b0;
  else if (cci0_evt) cci0_evt_s <=  1'b1;

reg cci1_evt_s;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       cci1_evt_s <=  1'b0;
  else if (tar_clk)  cci1_evt_s <=  1'b0;
  else if (cci1_evt) cci1_evt_s <=  1'b1;

reg cci2_evt_s;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)       cci2_evt_s <=  1'b0;
  else if (tar_clk)  cci2_evt_s <=  1'b0;
  else if (cci2_evt) cci2_evt_s <=  1'b1;

reg cci0_sync;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst) cci0_sync <=  1'b0;
  else         cci0_sync <=  (tar_clk & cci0_evt_s) | (tar_clk & cci0_evt & ~cci0_evt_s);

reg cci1_sync;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst) cci1_sync <=  1'b0;
  else         cci1_sync <=  (tar_clk & cci1_evt_s) | (tar_clk & cci1_evt & ~cci1_evt_s);

reg cci2_sync;
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst) cci2_sync <=  1'b0;
  else         cci2_sync <=  (tar_clk & cci2_evt_s) | (tar_clk & cci2_evt & ~cci2_evt_s);


// Generate final capture command
//-----------------------------------

assign cci0_cap  = tacctl0[`TASCS] ? cci0_sync : cci0_evt;
assign cci1_cap  = tacctl1[`TASCS] ? cci1_sync : cci1_evt;
assign cci2_cap  = tacctl2[`TASCS] ? cci2_sync : cci2_evt;


// Generate capture overflow flag
//-----------------------------------

reg  cap0_taken;
wire cap0_taken_clr = reg_rd[TACCR0] | (tacctl0_wr & tacctl0[`TACOV] & ~per_din[`TACOV]);
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)             cap0_taken <=  1'b0;
  else if (cci0_cap)       cap0_taken <=  1'b1;
  else if (cap0_taken_clr) cap0_taken <=  1'b0;

reg  cap1_taken;
wire cap1_taken_clr = reg_rd[TACCR1] | (tacctl1_wr & tacctl1[`TACOV] & ~per_din[`TACOV]);
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)             cap1_taken <=  1'b0;
  else if (cci1_cap)       cap1_taken <=  1'b1;
  else if (cap1_taken_clr) cap1_taken <=  1'b0;

reg  cap2_taken;
wire cap2_taken_clr = reg_rd[TACCR2] | (tacctl2_wr & tacctl2[`TACOV] & ~per_din[`TACOV]);
always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)             cap2_taken <=  1'b0;
  else if (cci2_cap)       cap2_taken <=  1'b1;
  else if (cap2_taken_clr) cap2_taken <=  1'b0;


assign cov0_set = cap0_taken & cci0_cap & ~reg_rd[TACCR0];
assign cov1_set = cap1_taken & cci1_cap & ~reg_rd[TACCR1];
assign cov2_set = cap2_taken & cci2_cap & ~reg_rd[TACCR2];


//============================================================================
// 8) Timer A output unit
//============================================================================

// Output unit 0
//-------------------
reg  ta_out0;

wire ta_out0_mode0 = tacctl0[`TAOUT];                // Output
wire ta_out0_mode1 = equ0 ?  1'b1    : ta_out0;      // Set
wire ta_out0_mode2 = equ0 ? ~ta_out0 :               // Toggle/Reset
                     equ0 ?  1'b0    : ta_out0;
wire ta_out0_mode3 = equ0 ?  1'b1    :               // Set/Reset
                     equ0 ?  1'b0    : ta_out0;
wire ta_out0_mode4 = equ0 ? ~ta_out0 : ta_out0;      // Toggle
wire ta_out0_mode5 = equ0 ?  1'b0    : ta_out0;      // Reset
wire ta_out0_mode6 = equ0 ? ~ta_out0 :               // Toggle/Set
                     equ0 ?  1'b1    : ta_out0;
wire ta_out0_mode7 = equ0 ?  1'b0    :               // Reset/Set
                     equ0 ?  1'b1    : ta_out0;

wire ta_out0_nxt   = (tacctl0[`TAOUTMODx]==3'b000) ? ta_out0_mode0 :
                     (tacctl0[`TAOUTMODx]==3'b001) ? ta_out0_mode1 :
                     (tacctl0[`TAOUTMODx]==3'b010) ? ta_out0_mode2 :
                     (tacctl0[`TAOUTMODx]==3'b011) ? ta_out0_mode3 :
                     (tacctl0[`TAOUTMODx]==3'b100) ? ta_out0_mode4 :
                     (tacctl0[`TAOUTMODx]==3'b101) ? ta_out0_mode5 :
                     (tacctl0[`TAOUTMODx]==3'b110) ? ta_out0_mode6 :
                                                     ta_out0_mode7;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)                                     ta_out0 <=  1'b0;
  else if ((tacctl0[`TAOUTMODx]==3'b001) & taclr)  ta_out0 <=  1'b0;
  else if (tar_clk)                                ta_out0 <=  ta_out0_nxt;

assign  ta_out0_en = ~tacctl0[`TACAP];


// Output unit 1
//-------------------
reg  ta_out1;

wire ta_out1_mode0 = tacctl1[`TAOUT];                // Output
wire ta_out1_mode1 = equ1 ?  1'b1    : ta_out1;      // Set
wire ta_out1_mode2 = equ1 ? ~ta_out1 :               // Toggle/Reset
                     equ0 ?  1'b0    : ta_out1;
wire ta_out1_mode3 = equ1 ?  1'b1    :               // Set/Reset
                     equ0 ?  1'b0    : ta_out1;
wire ta_out1_mode4 = equ1 ? ~ta_out1 : ta_out1;      // Toggle
wire ta_out1_mode5 = equ1 ?  1'b0    : ta_out1;      // Reset
wire ta_out1_mode6 = equ1 ? ~ta_out1 :               // Toggle/Set
                     equ0 ?  1'b1    : ta_out1;
wire ta_out1_mode7 = equ1 ?  1'b0    :               // Reset/Set
                     equ0 ?  1'b1    : ta_out1;

wire ta_out1_nxt   = (tacctl1[`TAOUTMODx]==3'b000) ? ta_out1_mode0 :
                     (tacctl1[`TAOUTMODx]==3'b001) ? ta_out1_mode1 :
                     (tacctl1[`TAOUTMODx]==3'b010) ? ta_out1_mode2 :
                     (tacctl1[`TAOUTMODx]==3'b011) ? ta_out1_mode3 :
                     (tacctl1[`TAOUTMODx]==3'b100) ? ta_out1_mode4 :
                     (tacctl1[`TAOUTMODx]==3'b101) ? ta_out1_mode5 :
                     (tacctl1[`TAOUTMODx]==3'b110) ? ta_out1_mode6 :
                                                     ta_out1_mode7;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)                                     ta_out1 <=  1'b0;
  else if ((tacctl1[`TAOUTMODx]==3'b001) & taclr)  ta_out1 <=  1'b0;
  else if (tar_clk)                                ta_out1 <=  ta_out1_nxt;

assign  ta_out1_en = ~tacctl1[`TACAP];


// Output unit 2
//-------------------
reg  ta_out2;

wire ta_out2_mode0 = tacctl2[`TAOUT];                // Output
wire ta_out2_mode1 = equ2 ?  1'b1    : ta_out2;      // Set
wire ta_out2_mode2 = equ2 ? ~ta_out2 :               // Toggle/Reset
                     equ0 ?  1'b0    : ta_out2;
wire ta_out2_mode3 = equ2 ?  1'b1    :               // Set/Reset
                     equ0 ?  1'b0    : ta_out2;
wire ta_out2_mode4 = equ2 ? ~ta_out2 : ta_out2;      // Toggle
wire ta_out2_mode5 = equ2 ?  1'b0    : ta_out2;      // Reset
wire ta_out2_mode6 = equ2 ? ~ta_out2 :               // Toggle/Set
                     equ0 ?  1'b1    : ta_out2;
wire ta_out2_mode7 = equ2 ?  1'b0    :               // Reset/Set
                     equ0 ?  1'b1    : ta_out2;

wire ta_out2_nxt   = (tacctl2[`TAOUTMODx]==3'b000) ? ta_out2_mode0 :
                     (tacctl2[`TAOUTMODx]==3'b001) ? ta_out2_mode1 :
                     (tacctl2[`TAOUTMODx]==3'b010) ? ta_out2_mode2 :
                     (tacctl2[`TAOUTMODx]==3'b011) ? ta_out2_mode3 :
                     (tacctl2[`TAOUTMODx]==3'b100) ? ta_out2_mode4 :
                     (tacctl2[`TAOUTMODx]==3'b101) ? ta_out2_mode5 :
                     (tacctl2[`TAOUTMODx]==3'b110) ? ta_out2_mode6 :
                                                     ta_out2_mode7;

always @ (posedge mclk or posedge puc_rst)
  if (puc_rst)                                     ta_out2 <=  1'b0;
  else if ((tacctl2[`TAOUTMODx]==3'b001) & taclr)  ta_out2 <=  1'b0;
  else if (tar_clk)                                ta_out2 <=  ta_out2_nxt;

assign  ta_out2_en = ~tacctl2[`TACAP];


//============================================================================
// 9) Timer A interrupt generation
//============================================================================


assign   taifg_set   = tar_clk & (((tactl[`TAMCx]==2'b01) & (tar==taccr0))                  |
                                  ((tactl[`TAMCx]==2'b10) & (tar==16'hffff))                |
                                  ((tactl[`TAMCx]==2'b11) & (tar_nxt==16'h0000) & tar_dec));

assign   ccifg0_set  = tacctl0[`TACAP] ? cci0_cap : (tar_clk &  ((tactl[`TAMCx]!=2'b00) & equ0));
assign   ccifg1_set  = tacctl1[`TACAP] ? cci1_cap : (tar_clk &  ((tactl[`TAMCx]!=2'b00) & equ1));
assign   ccifg2_set  = tacctl2[`TACAP] ? cci2_cap : (tar_clk &  ((tactl[`TAMCx]!=2'b00) & equ2));


wire     irq_ta0    = (tacctl0[`TACCIFG] & tacctl0[`TACCIE]);

wire     irq_ta1    = (tactl[`TAIFG]     & tactl[`TAIE])     |
                      (tacctl1[`TACCIFG] & tacctl1[`TACCIE]) |
                      (tacctl2[`TACCIFG] & tacctl2[`TACCIE]);


endmodule // omsp_timerA

`ifdef OMSP_TA_NO_INCLUDE
`else
`include "omsp_timerA_undefines.v"
`endif
