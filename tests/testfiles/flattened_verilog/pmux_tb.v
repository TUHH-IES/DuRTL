module pmux_tb;
	
	reg [3:0] a;
	reg [11:0] b;
	reg [2:0] s;
	wire [3:0] out;

	// instance of the counter module
	pmux a1 (a, b, s, out);

	initial begin
		a = 0;
		b = 12'b000000000000;
		s = 0;
		#2
		a = 0;
		b = 12'b001000000000;
		#2
		a = 3;
		b = 12'b000010000000;
		#2
		a = 3;
		b = 12'b000000000010;
		s = 1;
		#2
		a = 0;
		b = 12'b000010000010;
		s = 2;
		#2
		a = 1;
		b = 12'b101110000000;
		s = 4;
		#2
		a = 0;
		b = 12'b000110000000;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars();		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
