`timescale 1us/100ns
module sr_tb();
	reg [1:0] SET;
	reg [1:0] CLR;
	wire[1:0] Q;
	
	reg [31:0] SET_t;
	reg [31:0] CLR_t;
	wire [31:0] Q_t;


	reg[63:0] read_data [0:3];
	integer i;
	
	m_0 d1(.CLR(CLR),.CLR_t(CLR_t), .Q(Q), .Q_t(Q_t), .SET(SET), .SET_t(SET_t));

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=3; i+=1) begin
		{ CLR_t,SET_t} = read_data[i]; 
		 #40;
		 SET <= 'b00;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 SET <= 'b01;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 SET <= 'b11;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 SET <= 'b10;
		 CLR <= 'b00;
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
		//$dumpfile("sr.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
