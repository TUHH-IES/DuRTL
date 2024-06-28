`timescale 1ns/1ps
module adff_tb();
	reg [1:0] D;
	reg CLK;
	wire[1:0] Q;
	reg ARST;
	
	reg [31:0] D_t;
	reg [31:0] CLK_t = 0;
	reg [31:0] ARST_t;
	wire [31:0] Q_t;
	
	reg[63:0] read_data [0:3];
	integer i;

	m_0 d1(ARST, ARST_t, CLK, CLK_t, D, D_t, Q, Q_t);

	initial begin
  		CLK=0;
     		forever #10 CLK = ~CLK;  
	end 

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		
		for(i = 0; i<4; i+=1) begin
		{ARST_t, D_t} = read_data[i];
		 #40;
		 D <= 'b00;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 D <= 'b01;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 D <= 'b10;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 D <= 'b11;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 end
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
