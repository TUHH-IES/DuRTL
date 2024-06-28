module Test_tb;
  reg [63:0] a;
  reg [63:0] b;
  output [63:0] c;
  
  integer i;
  real a_1, b_1;
  reg [63:0] init = 64'b0;
  reg [63:0] initVal = 64'b00000000000000000000000000000000000000000011111111111111111111;
  reg [63:0] incValX = 64'b11110000011111100001111;
  reg [63:0] incValY = 64'b11110000000000011001101;


function real IncTestValues1;
    input [63:0] x;
    begin
        IncTestValues1 = x + incValX;
    end
  endfunction

function real IncTestValues2;
    input [63:0] x;
    begin
        IncTestValues2 = x + incValY;
    end
  endfunction

  task IncVar1;
  input [63:0] Var1;
  input [63:0] Var2;
    begin
     a = IncTestValues1(Var1);

     b = IncTestValues2(Var2);
    end
  endtask

  initial begin
    a_1 = initVal;
    b_1 = init;
    for(i = 0; i<300; i = i + 1)
    begin
      IncVar1(a_1, b_1);
      a_1 = a;
      b_1 = b;
      #2;
    end
  end

  initial
	begin
		$dumpfile("../../examples/ReadWrite.vcd");
		$dumpvars(1,  a, b);		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, a = %b, b = %b", $time, a, b);
endmodule
