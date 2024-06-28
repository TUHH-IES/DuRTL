module hierarchy_tb;
  reg a,b;
  wire c;

   initial begin
      a = 0;
      b = 0;
      #8;
      a = 0;
      b = 1;
      #8;
      a = 1;
      b = 0;
      #8;
      a = 1;
      b = 1;
      #8;
   
   end

  hierarchy t1(a,b,c);
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);
			$dumpvars;		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, a, b, c);

endmodule
