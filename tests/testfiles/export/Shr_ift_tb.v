module Test_tb;
  reg [3:0] a;
  reg [3:0] b;
  reg [31:0] a_t;
  reg [31:0] b_t;
  reg[7:0] read_data [0:3];
  integer i;
  output [3:0] c;
  output [31:0] c_t;

  Shr d1( a, a_t, b, b_t, c, c_t);

  
  initial begin
    $readmemb(`INPUT_FILE_NAME, read_data);
      for(i = 0; i<=3; i = i + 1) begin
      {a, b} =  read_data[i];
      #2;
    end
  end

  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars();		//writing the vcd file
	end
	
//always@(a, b, c)
		//$monitor("At time %t, a = %b b = %b, ", $time, a, b);
endmodule
