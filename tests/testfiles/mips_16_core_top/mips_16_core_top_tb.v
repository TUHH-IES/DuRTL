////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   19:38:28 02/09/2012
// Design Name:   mips_16_core_top
// Module Name:   F:/Projects/My_MIPS/mips_16/bench/mips_16_core_top/mips_16_core_top_tb_0.v
// Project Name:  mips_16
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: mips_16_core_top
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////
// `timescale 1ns/1ps
 `ifndef _MIPS_16_DEFS
 `define _MIPS_16_DEFS
 	
	//`define CODE_FOR_SYNTHESIS				// uncomment this macro will remove all non-systhesis code
	`define USE_SIMULATION_CODE					// uncomment this to use simulation instruction memory
	
	`define PC_WIDTH				8
	`define	INSTR_MEM_ADDR_WIDTH	8
	`define DATA_MEM_ADDR_WIDTH		8
 
	/************** Operation Code in instructions ****************/
	`define OP_NOP			4'b0000
	`define OP_ADD			4'b0001
	`define OP_SUB			4'b0010
	`define OP_AND			4'b0011
	`define OP_OR			4'b0100
	`define OP_XOR			4'b0101
	`define OP_SL			4'b0110
	`define OP_SR			4'b0111
	`define OP_SRU			4'b1000
	`define OP_ADDI			4'b1001
	`define OP_LD			4'b1010
	`define OP_ST			4'b1011
	`define OP_BZ			4'b1100
	
	/************** ALU operation command ****************/
	`define ALU_NC			3'bxxx		// not care
	`define ALU_ADD			3'b000
	`define ALU_SUB			3'b001
	`define ALU_AND			3'b010
	`define ALU_OR			3'b011
	`define ALU_XOR			3'b100
	`define ALU_SL			3'b101
	`define ALU_SR			3'b110
	`define ALU_SRU			3'b111
	
	/************** Branch condition code ****************/
	`define BRANCH_Z		3'b000
	//`define BRANCH_GT		3'b001
	//`define BRANCH_LE		3'b010
	
 
 
 `endif
 module mips_16_core_top_tb_0_v;

	// Inputs
	reg clk;
	reg rst;

	// Outputs
	wire [`PC_WIDTH-1:0] pc;
	
	parameter CLK_PERIOD = 10;
	always #(CLK_PERIOD /2) 
		clk =~clk;
	integer i;
	integer test;
	
	// Instantiate the Unit Under Test (UUT)
	mips_16_core_top uut (
		.clk(clk), 
		.rst(rst), 
		.pc(pc)
	);
	
	initial begin
		// Initialize Inputs
		clk = 0;
		rst = 0;

		// Wait 100 ns for global reset to finish
		#100;
        
		// Add stimulus here
		#(CLK_PERIOD/2)
		#1
		display_debug_message;
		//test1;
		test2;
		$finish;
	end
      
	task display_debug_message;
		begin
			$display("\n***************************");
			$display("mips_16 core test");
			$display("***************************\n");
		end
	endtask
	
	task sys_reset;
		begin
			rst = 0;
			#(CLK_PERIOD*1) rst = 1;
			#(CLK_PERIOD*1) rst = 0;
		end
	endtask
	
	
	task test1;
		begin
			$readmemb("test1.prog",uut.IF_stage_inst.imem.rom);
			$display("rom load successfully\n");
			$display("running test1\n");
			sys_reset;
			#1
			
			//#(CLK_PERIOD) test = 1;
			
			$monitor("current pc: %d ,instruction: %x", pc, uut.instruction);
			
			#(CLK_PERIOD*100)
			$monitoroff;
			display_all_regs;
			$display("ram[10] = %d", uut.MEM_stage_inst.dmem.ram[10]);
			//#(CLK_PERIOD*100) test = 0;
			sys_reset;
			
		end
	endtask
	
	task test2;
		begin
			$readmemb(`INPUT_FILE_NAME,uut.IF_stage_inst.imem.rom);
			$display("rom load successfully\n");
			$display("running test2\n");
			$display("multiply R3=R1*R2\n");
			sys_reset;
			display_all_regs;
			#1
			test = 2;
			
			//#(CLK_PERIOD) test = 1;
			
			$monitor("current pc: %d ,instruction: %x", pc, uut.instruction);
			
			#(CLK_PERIOD*400)
			$monitoroff;
			display_all_regs;
			
			test = 0;
			sys_reset;
			
		end
	endtask
	
	task display_all_regs;
		begin
	//		$display("display_all_regs:");
	//		$display("------------------------------");
	//		$display("R0\tR1\tR2\tR3\tR4\tR5\tR6\tR7");
	//		for(i=0; i<8; i=i+1)
	//				$write("%d\t",uut.register_file_inst.reg_array[i]);
	//		$display("\n------------------------------");
		end
	endtask
	
	
	always @ (test) begin
	    case(test)
			// 1: begin
			    // $display("running test1\n");
				 // while(test == 1) begin
				    // @(uut.pc)
					// $display("current pc : %d",uut.pc);
					// if(uut.pc == 40) begin
						// #1
					    // branch_taken = 1;
						// branch_offset_imm = -30;
						// #(CLK_PERIOD*1) 
						// branch_taken =0;
						// branch_offset_imm = 0;
					// end
				 // end
			// end
			
			2: begin
			    $display("running test2\n");
				 while(test == 2) begin
				    @(pc)
					if(pc==6) begin
						$display("current pc : %d",uut.pc);
						display_all_regs();
					end
							
				 end
			end
			
		endcase
	end
initial begin
	$dumpfile(`DUMP_FILE_NAME);
	$dumpvars();
	end
endmodule

