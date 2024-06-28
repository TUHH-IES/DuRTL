`timescale 1ns/1ps
module tb_DLATCH();
	reg [1:0] D;
	reg EN;
	wire[1:0] Q;

	m_0 d1(.D(D), .EN(EN), .Q(Q));

	initial begin
  		EN=0;
     		forever #10 EN = ~EN;  
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
		//$dumpfile("../../examples/flipFlops/dlatch/dlatchExport.vcd");
		$dumpvars(0,EN,D,Q);		//writing the vcd file
	end
 	
endmodule 

