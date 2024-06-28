`timescale 1ns/1ps
module adlatch_tb();
	reg [1:0] D;
	reg EN;
	wire[1:0] Q;
	reg ARST;
	
	reg [31:0] D_t;
	reg [31:0] EN_t;
	reg [31:0] ARST_t;
	wire [31:0] Q_t;
	
	reg[95:0] read_data [0:7];
	integer i;
	
	m_0 d1(.D(D),.D_t(D_t),.ARST(ARST), .ARST_t(ARST_t), .EN(EN), .EN_t(EN_t), .Q(Q), .Q_t(Q_t));

	initial begin
  		EN=0;
     		forever #10 EN = ~EN;  
	end 

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=7; i+=1) begin
		{ARST_t,EN_t, D_t} = read_data[i]; 
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
		//$dumpfile("adlatch.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
