`timescale 1ns/1ps
module dff_tb();
	reg [1:0] D;
	reg CLK;
	wire[1:0] Q;
	
	reg [31:0] D_t;
	reg [31:0] CLK_t = 0;
	wire [31:0] Q_t;
	
	reg[31:0] read_data [0:7];
	integer i;
	
	m_0 d1(.CLK(CLK),.CLK_t(CLK_t),.D(D),.D_t(D_t),.Q(Q),.Q_t(Q_t));

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=7; i+=1) begin
		{D_t} = read_data[i];
		#50;
		end
	end

	initial begin
  		CLK=0;
     		forever #10 CLK = ~CLK;
	end 

	initial begin 
		 #100;
		 D <= 'b00;
		 #100;
		 D <= 'b01;
		 #100;
		 D <= 'b10;
		 #100;
		 D <= 'b11;
		 #100;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
