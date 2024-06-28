module hierarchy_tb;
  reg a,b;
  wire c;

  reg clk = 0;
  always #1 clk = ~clk;

   initial begin
      a = 0;
      b = 0;
      #4;
      a = 0;
      b = 1;
      #4;
      a = 1;
      b = 0;
      #4;
      a = 1;
      b = 1;
      #4;
      a = 1;
      b = 0;
      #4;
      a = 0;
      b = 1;
      #4;
      $finish;
   end

  hierarchy_seq t1(.clk(clk),.a(a),.b(b),.c(c));
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);
			$dumpvars;		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, a, b, c);

endmodule
