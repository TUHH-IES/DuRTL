module orGate_tb;
	
	reg a, b;
	wire out;

	// instance of the counter module
	orGate a1 (a, b, out);

	initial begin
		a = 0;
		b = 0;
		#2
		a = 1;
		b = 0;
		#2
		a = 0;
		b = 1;
		#2
		a = 1;
		b = 1;
		#2
		a = 0;
		b = 0;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars();		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
