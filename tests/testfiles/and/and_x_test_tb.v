module anda_tb;
  reg a;
  reg [31:0] a_t, b_t;
  reg b;
  output c;
  output [31:0] c_t;

  anda d1( .a(a), .a_t(a_t), .b(b), .b_t(b_t), .c(c), .c_t(c_t));
  initial begin
    #0
    a = 0;
    b = 0;
    a_t = 32'b00000000000000000000000000000001;
    b_t = 32'b00000000000000000000000000000010;
    #1
    a = 1'bx;
    b = 0;
    #1
    a = 1'bx;
    b = 1;
    #1
    a = 1'bx;
    b = 1'bx;
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
