
module main;

  reg clk, rst;
  wire [31:0] bus_A_ins;
  reg [31:0] bus_in_ins;
  wire [31:0] bus_A_data;
  reg [31:0] bus_in_data;
  wire [31:0] bus_out;
  wire [7:0] current_opcode;
  wire bus_WE, bus_RE_ins, bus_RE_data;

  reg [7:0] memory [0:99];  //100-element array of 8-bit wide reg
	reg [7:0] counter;

  cisc_cpu_p dut (.clk(clk), .rst(rst), .bus_A_ins(bus_A_ins), .bus_in_ins(bus_in_ins), .bus_A_data(bus_A_data),
                  .bus_in_data(bus_in_data), .bus_out(bus_out), .bus_WE(bus_WE), .bus_RE_data(bus_RE_data),
                  .bus_RE_ins(bus_RE_ins), .current_opcode(current_opcode));

	initial begin
	memory[0] = 0;
	memory[1] = 0;
	memory[2] = 0;
	memory[3] = 0;
	memory[4] = 0;
	memory[30] = 5;
	memory[35] = 8;
	memory[40] = 13;
	memory[90] = 1;
	memory[91] = 0;
	memory[92] = 0;
	memory[93] = 0;
	bus_in_ins = 0;
		$readmemh(`INPUT_FILE_NAME, memory);
		$display ("%h ", memory[8'h00]);	
	end

  initial begin
	clk = 1'b0;
	rst = 1'b1;
	repeat(3) #10 clk = ~clk;
	rst = 1'b0;
	forever #10 clk = ~clk; // generate a clock
  end

  initial begin
	clk = 0;
	rst = 1;
	
    $display("CISC_CPU\n");
     
    #35 rst = 0;
	
	#100000 $display("bus_out=%d; bus_A= %d; bus_WE= %d; bus_RE= %d", bus_out, bus_A_ins, bus_WE, bus_RE_ins);
		 $display("current_opcode= %h", current_opcode);
		 
    $finish;
    
  end // initial begin

	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);
			$dumpvars(0,main);
		end
		
	always @(posedge clk) begin
	memory[90] <= 1;
	memory[94] <= 10;
	memory[95] <= 0;
	memory[96] <= 0;
	memory[97] <= 0;
		if (!rst) begin
		bus_in_ins[31:24] <= memory[bus_A_ins+3];
		bus_in_ins[23:16] <= memory[bus_A_ins+2];
		bus_in_ins[15:8] <= memory[bus_A_ins+1];
		bus_in_ins[7:0] <= memory[bus_A_ins];
		end
	end
	
	always @(negedge clk) begin
		if (bus_RE_data) begin
		bus_in_data[31:24] <= memory[bus_A_data+3];
		bus_in_data[23:16] <= memory[bus_A_data+2];
		bus_in_data[15:8] <= memory[bus_A_data+1];
		bus_in_data[7:0] <= memory[bus_A_data];
		end
	end
		

endmodule // main

/*		Beispiel Programm aus dem Buch
		"\x29\xF6"          // 00000000 sub esi,esi
        "\x29\xC0"          // 00000002 sub eax,eax
        "\x29\xDB"          // 00000004 sub ebx,ebx
        "\x8B\x56\x17"      // 00000006 mov edx,[esi+0x17]
        "\x01\xD0"          // 00000009 add eax,edx
        "\x01\xC3"          // 0000000B add ebx,eax
        "\x89\xC1"          // 0000000D mov ecx,eax
        "\x8B\x56\x1B"      // 0000000F mov edx,[esi+0x1b]
        "\x29\xD1"          // 00000012 sub ecx,edx
        "\x75\xF0"          // 00000014 jnz 0x6
        "\xF4"              // 00000016 hlt
        "\x01\x00\x00\x00"  // 00000017 dd 1
        "\x0a\x00\x00\x00"; // 0000001B dd 10
*/
