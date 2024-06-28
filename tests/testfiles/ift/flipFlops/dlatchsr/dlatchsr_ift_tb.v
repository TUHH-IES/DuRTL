`timescale 1us/100ns
module dlatchsr_tb();
	reg [1:0] D;
	reg [1:0] SET;
	reg [1:0] CLR;
	reg EN;
	wire[1:0] Q;
	
	reg [31:0] D_t;
	reg [31:0] EN_t;
	reg [31:0] CLR_t;
	reg [31:0] SET_t;
	wire [31:0] Q_t;
	
	
	reg[127:0] read_data [0:15];
	integer i;
	
	m_0 d1(.CLR(CLR),.CLR_t(CLR_t), .D(D), .D_t(D_t), .EN(EN),.EN_t(EN_t), .Q(Q), .Q_t(Q_t),.SET(SET), .SET_t(SET_t));

	
	initial begin
  		EN=0;
     		forever #10 EN = ~EN;  
	end 

	initial begin 
	$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=15; i+=1) begin
		{ D_t, EN_t, CLR_t, SET_t} = read_data[i]; 
		 #40;
		 D <= 'b00;
		 SET <= 'b00;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b00;
		 SET <= 'b01;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b01;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b01;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b01;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b00;
		 SET <= 'b11;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b11;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b11;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b11;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b00;
		 SET <= 'b10;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b10;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b10;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b10;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 end
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("dlatchsr.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
