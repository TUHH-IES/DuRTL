`timescale 1ns/1ps
module dffe_tb();
	reg [1:0] D;
	reg CLK;
	reg EN;
	wire[1:0] Q;
	
	reg [31:0] D_t = 0;
	reg [31:0] CLK_t = 0;
	reg [31:0] EN_t;
	wire [31:0] Q_t;

	reg[63:0] read_data [0:3];
	integer i;
	
	m_0 d1(.CLK(CLK),.CLK_t(CLK_t),.D(D),.D_t(D_t),.EN(EN),.EN_t(EN_t),.Q(Q),.Q_t(Q_t));

	

	initial begin
  		CLK=0;
     		forever #4 CLK = ~CLK;  
	end 

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<4; i+=1) begin
			{D_t, EN_t} = read_data[i];
			#5;
			#10;
			EN <= 'b0;	
			#10;
			D <= 'b00;
			#10;
			D <= 'b01;
			#10;
			D <= 'b10;
			#10;
			D <= 'b11;
			#10;
			EN <= 'b1;	
			#10;
			D <= 'b00;
			#10;
			D <= 'b01;
			#10;
			D <= 'b10;
			#10;
			D <= 'b11;
			#10;
		end
		$finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
