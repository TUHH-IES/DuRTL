`timescale 1ns/1ps
module aldff_tb();
	reg [1:0] D;
	reg [1:0] AD;
	reg CLK;
	reg ALOAD;
	wire[1:0] Q;
	
	reg [31:0] D_t;
	reg [31:0] CLK_t = 0;
	reg [31:0] ALOAD_t;
	reg [31:0] AD_t;
	wire [31:0] Q_t;

	reg[95:0] read_data [0:7];
	integer i;

	m_0 d1(AD, AD_t, ALOAD, ALOAD_t, CLK, CLK_t, D, D_t, Q, Q_t);

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=7; i+=1) begin
		{AD_t, ALOAD_t, D_t} = read_data[i];
		 #40;
		 D <= 'b00;
		 AD <= 'b11;
		 #5;
		 ALOAD <= 1;
		 #5;
		 ALOAD <= 0;
		 #40;
		 D <= 'b01;
		 AD <= 'b1;
		 #5;
		 ALOAD <= 1;
		 #5;
		 ALOAD <= 0;
		 #40;
		 D <= 'b10;
		 AD <= 'b01;
		 #5;
		 ALOAD <= 1;
		 #5;
		 ALOAD <= 0;
		 #40;
		 D <= 'b11;
		 AD <= 'b00;
		 #5;
		 ALOAD <= 1;
		 #5;
		 ALOAD <= 0;
		 #40;
		end
		$finish;
	end

	initial begin
  		CLK=0;
     		forever #10 CLK = ~CLK;  
	end 

	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("../../examples/flipFlops/aldff/aldffExport.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
