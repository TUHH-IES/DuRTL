module andVector_tb;
	
	reg [3:0] a, b;
	wire [3:0] out;

	// instance of the counter module
	andVector a1 (a, b, out);

	initial begin
		a = 0;
		b = 0;
		#2
		a = 2;
		b = 0;
		#2
		a = 3;
		b = 1;
		#2
		a = 4;
		b = 1;
		#2
		a = 3;
		b = 2;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars();		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, result, result);
endmodule // test
