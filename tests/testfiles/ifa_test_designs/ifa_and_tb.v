module anda_tb;
  reg a;
  output c;

  anda d1( a, c);
  initial begin
    #0
    a = 0;
    #1
    a = 1;
    #1
    $finish;
  end


  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(0);		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, value a = %b value a = %b value a = %b", $time, a, b, c);
endmodule
