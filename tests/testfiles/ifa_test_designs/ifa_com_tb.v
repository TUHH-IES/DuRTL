module ifa_seq_tb;
  reg a,b,c;
  output d,e;

  ifa_com d1(a,b,c,d,e);
  
  initial begin
    #0
    a = 0;
    b = 0;
    c = 0;
    #1
    a = 0;
    b = 0;
    c = 1;
    #1
    a = 0;
    b = 1;
    c = 1;
    #1
    a = 1;
    b = 0;
    c = 1;
    #1
    a = 1;
    b = 1;
    c = 1;
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
