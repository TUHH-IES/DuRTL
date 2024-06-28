module posGate_tb;
	
	reg signed [2:0] a;
	wire signed [2:0] out;

	// instance of the counter module
	posGate a1 (a, out);

	initial begin
		a = 0;
		#2
		a = -1;
		#2
		a = -2;
		#2
		a = 7;
		#2
		a = 0;
		#2
		a = 7;
		#2
		a = 4;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars();		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
