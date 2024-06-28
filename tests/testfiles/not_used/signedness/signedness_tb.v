module Test_tb;
  reg signed [2:0] a;
  reg signed [1:0] b;
  wire signed [4:0] c;
  reg unsigned [2:0] au;
  reg unsigned [1:0] bu;
  wire unsigned [4:0] cu;
  integer i;
  integer j;

  signedness mod( a, b, c, au,bu,cu);

  initial begin
    a= -4;
    b= 1;
    au= -4;
    bu= 1;
    #2;
    //*/
    for(i = -20; i<=20; i = i + 1) 
      for(j = -20; j<=20; j = j + 1) begin
      a= i; b=j;
      au= i; bu=j;
      #2;
    end
    //*/
    a= -2;
    b= -2;
    au= -2;
    bu= -2;
    #2;
  end
  
  initial
	begin
		$dumpfile("verilog.vcd");  
		$dumpvars(1,  a, b, c, au, bu, cu);		//writing the vcd file
	end
	
endmodule
