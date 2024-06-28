`timescale 1us/100ns
module dffsre_tb();
	reg D;
	reg SET;
	reg CLR;
	reg clk;
	reg EN;
	wire Q;
	
	reg [31:0] D_t;
	reg [31:0] CLK_t = 0;
	reg [31:0] CLR_t;
	reg [31:0] EN_t;
	reg [31:0] SET_t;
	wire [31:0] Q_t;
	
	
	reg[127:0] read_data [0:15];
	integer i;
	

	m_0 d1(.CLK(clk),.CLK_t(CLK_t),.CLR(CLR),.CLR_t(CLR_t), .D(D), .D_t(D_t), .EN(EN), .EN_t(EN_t), .Q(Q), .Q_t(Q_t),.SET(SET), .SET_t(SET_t));

	initial begin
  		clk=0;
     		forever #10 clk = ~clk;  
	end 

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=15; i+=1) begin
		{ D_t,EN_t,CLR_t, SET_t} = read_data[i]; 
		 #40;
		 EN <= 0;
		 D <= 'b0;
		 SET <= 'b0;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b10;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b1;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 EN <= 1;
		  D <= 'b0;
		 SET <= 'b0;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b10;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b1;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 end
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		
		//$dumpfile("dffsre.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
