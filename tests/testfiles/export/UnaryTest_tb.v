module UnaryTest_tb;
  reg [31:0] a;
  reg [31:0] read_data [0:9];
  integer i;
  output [31:0] c;
  output [31:0] c1;
  output [31:0] c2;
  output [31:0] c3;
  output [31:0] c4;
  output [31:0] c5;
  

  UnaryTest d1( a, c, c1,c2,c3,c4, c5);
  initial begin
    $readmemb(`INPUT_FILE_NAME, read_data);
      for(i = 0; i<=9; i = i + 1) begin
      {a} =  read_data[i];
      #2;
    end
  end


  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars();		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, value a = %b value a = %b value a = %b", $time, a, c);
endmodule
