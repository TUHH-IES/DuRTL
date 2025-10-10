module ifa_seq_tb;
  reg clk,a,b,c;
  output d,e;

  ifa_seq d1(clk,a,b,c,d,e);
  
  initial
    clk = 0;

  always #1 clk = ~clk;
  
  initial begin
    #0
    a = 0;
    b = 0;
    c = 0;
    #2
    a = 0;
    b = 0;
    c = 1;
    #2
    a = 0;
    b = 1;
    c = 1;
    #2
    a = 1;
    b = 0;
    c = 1;
    #2
    a = 1;
    b = 1;
    c = 1;
    #2
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
