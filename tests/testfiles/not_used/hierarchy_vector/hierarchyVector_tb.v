module hierarchy_tb;
  reg [4:0] a,b;
  wire [4:0] c;

  initial begin
    #1 a= 1;
    b= 1;
    #2 a= 0;
    b= 1;
    #2 a= 0;
    b= 0;
    #2 a= 1;
    b= 0;
  end


  hierarchyVector uut(a,b,c);

	initial
		begin
			$dumpfile("test.vcd");  
			$dumpvars(1, hierarchy_tb);		//writing the vcd file
		end

	//initial
	  //$monitor("At time %t, value = %h (%0d)", $time, a, b, c);

endmodule
