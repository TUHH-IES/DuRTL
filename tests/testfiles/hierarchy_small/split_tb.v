module split_tb;
	
	reg [1:0] a;
	reg [1:0] b;
	wire [1:0] out;

	// instance of the counter module
	split a1 (.a(a),.out(out));

	initial begin
		a = 2'b00;
		b = 3'b000;
		#1
		a = 2'b01;
		b = 3'b000;
		#1
		a = 2'b10;
		b = 3'b101;
		#1
		a = 2'b11;
		b = 3'b001;
		#1

		$finish;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars(0);		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
