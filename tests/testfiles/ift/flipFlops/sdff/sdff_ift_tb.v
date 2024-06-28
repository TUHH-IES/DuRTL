`timescale 1ns/1ps
module sdff_tb();
	reg [1:0] D;
	reg clk;
	wire[1:0] Q;
	reg SRST;
	
	reg [31:0] D_t;
	reg [31:0] CLK_t = 0;
	reg [31:0] SRST_t;
	wire [31:0] Q_t;
	
	reg[63:0] read_data [0:3];
	integer i;

	m_0 d1(.CLK(clk), .CLK_t(CLK_t), .D(D), .D_t(D_t), .SRST(SRST),.SRST_t(SRST_t), .Q(Q), .Q_t(Q_t));

	initial begin
  		clk=0;
     		forever #5 clk = ~clk;  
	end 

	initial begin
	$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<4; i+=1) begin
		{ SRST_t,D_t} = read_data[i]; 
		 #20;
		 D <= 'b00;
		 SRST <= 0;
		 #20;
		 D <= 'b00;
		 SRST <= 1;
		 #20;
		 D <= 'b01;
		 SRST <= 0;
		 #20;
		 D <= 'b01;
		 SRST <= 1;
		 #20;
		 D <= 'b10;
		 SRST <= 0;
		 #20;
		 D <= 'b10;
		 SRST <= 1;
		 #20;
		 D <= 'b11;
		 SRST <= 0;
		 #20;
		 D <= 'b11;
		 SRST <= 1;
		 #20;
		 end
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("../../examples/flipFlops/sdff/sdffExport.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
