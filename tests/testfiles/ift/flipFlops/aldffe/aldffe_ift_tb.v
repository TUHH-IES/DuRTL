`timescale 1ns/1ps
module aldffe_tb();
	reg [1:0] D;
	reg [1:0] AD;
	reg clk;
	reg aload;
	reg EN;
	wire[1:0] Q;
	
	reg [31:0] D_t;
	reg [31:0] CLK_t = 0;
	reg [31:0] ALOAD_t;
	reg [31:0] AD_t;
	reg [31:0] EN_t;
	wire [31:0] Q_t;
	
	reg[127:0] read_data [0:15];
	integer i;
	

	m_0 d1(AD, AD_t, aload, ALOAD_t, clk, CLK_t, D, D_t, EN, EN_t, Q, Q_t);

	initial begin
  		clk=0;
     		forever #10 clk = ~clk;  
	end 

	initial begin 
	$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<16; i+=1) begin
		{AD_t, ALOAD_t, D_t, EN_t} = read_data[i];
		 #40;
		 EN <= 1;
		 D <= 'b00;
		 AD <= 'b11;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b01;
		 AD <= 'b1;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b10;
		 AD <= 'b01;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b11;
		 AD <= 'b00;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 
		 EN <= 0;
		 D <= 'b00;
		 AD <= 'b11;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b01;
		 AD <= 'b1;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b10;
		 AD <= 'b01;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b11;
		 AD <= 'b00;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 end
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("../../examples/flipFlops/aldffe/aldffeExport.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
