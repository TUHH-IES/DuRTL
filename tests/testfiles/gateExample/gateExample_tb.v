module gateExample_tb;
  reg a;
  reg b;
  reg c;
  reg d;
  reg[3:0] read_data [0:15];
  integer i;
  output e;
  output f;

  gateExample d1( a, b, c, d, e, f);
  initial begin
    #0
    a=0;b=0;c=0;d=0;
    #2
    a=0;b=0;c=0;d=1;
    #2
    a=0;b=0;c=1;d=0;
    #2
    a=0;b=0;c=1;d=1;
    #2
    a=0;b=1;c=0;d=0;
    #2
    a=0;b=1;c=0;d=1;
    #2
    a=0;b=1;c=1;d=0;
    #2
    a=0;b=1;c=1;d=1;
    #2
    a=1;b=0;c=0;d=0;
    #2
    a=1;b=0;c=0;d=1;
    #2
    a=1;b=0;c=1;d=0;
    #2
    a=1;b=0;c=1;d=1;
    #2
    a=1;b=1;c=0;d=0;
    #2
    a=1;b=1;c=0;d=1;
    #2
    a=1;b=1;c=1;d=0;
    #2
    a=1;b=1;c=1;d=1;
    #2
    $finish;
  end

  initial
	begin
	    $dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("gateExample.vcd");
		$dumpvars(1,  a, b, c, d, e, f);		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, value a = %b value a = %b value a = %b", $time, a, b, c);
endmodule
