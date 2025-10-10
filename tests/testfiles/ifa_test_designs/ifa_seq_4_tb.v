module ifa_seq_4_tb;
  reg clk;
  reg [3:0] a, b, c;
  output [3:0] d, e, f, g, h, i, j, k;

  ifa_seq_4 d1(clk,a,b,c,d,e,f,g,h,j,k,l);
  
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
    c = 9;
    #2
    a = 0;
    b = 15;
    c = 9;
    #2
    a = 3;
    b = 0;
    c = 9;
    #2
    a = 3;
    b = 15;
    c = 9;
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
