module andGate_tb;
	
	reg a, b, c;
	wire out;

	// instance of the counter module
	andGate a1 (a, b, c, out);

	initial begin
		a = 0;
		b = 0;
		c = 0;
		#2
		a = 1;
		b = 0;
		#2
		a = 1'bX;
		b = 0;
		#2
		a = 1'bX;
		b = 1'bX;
		c = 1'bX;
		#2
		a = 0;
		b = 1;
		#2
		a = 1;
		b = 1;
		c = 1;
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
		//$monitor("At time %t, value = %h (%0d)", $time, result, result);
endmodule // test
