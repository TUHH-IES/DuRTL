module mux_tb;
	
	reg a, b, s;
	wire out;

	// instance of the counter module
	mux a1 (a, b, s, out);

	initial begin
		a = 0;
		b = 0;
		s = 0;
		#2
		a = 1;
		b = 0;
		s = 1;
		#2
		a = 1;
		b = 0;
		s = 0;
		#2
		a = 0;
		b = 1;
		s = 0;
		#2
		a = 0;
		b = 0;
		s = 0;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars();		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
