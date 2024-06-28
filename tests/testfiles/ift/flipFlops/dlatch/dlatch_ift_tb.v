`timescale 1ns/1ps
module dlatch_tb();
	reg [1:0] D;
	reg EN;
	wire[1:0] Q;

	reg [31:0] D_t;
	reg [31:0] EN_t;
	wire [31:0] Q_t;
	
	reg[31:0] read_data [0:1];
	integer i;
	
	m_0 d1(.D(D),.D_t(D_t), .EN(EN), .EN_t(EN_t), .Q(Q), .Q_t(Q_t));

	initial begin
  		EN=0;
     		forever #5 EN = ~EN;  
	end 

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=1; i+=1) begin
		{ D_t, EN_t} = read_data[i]; 
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
		//$dumpfile("../../examples/flipFlops/dlatch/dlatchExport.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 

