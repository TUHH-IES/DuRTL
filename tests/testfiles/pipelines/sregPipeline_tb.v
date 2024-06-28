module sregPipeline_tb;
  reg [8:0] a;
  reg en, reset;
  wire [8:0]c;
  integer i;

	reg clock = 0;
	always #1 clock = !clock;
  
  initial begin
    #1 a= 1;
    reset= 1;
    #2 
    reset= 0;
    en= 1;

    for (i=0; i<20; i= i+1)
      #2 a= a+1;

    #10     $finish ;
  end


  sregPipeline t1(a,c, en, clock, reset);
	
	initial
		begin
			$dumpfile("test.vcd");  
			$dumpvars;		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, a %d, c %d, en %d, clock %d, reset %d", $time, a, c, en, clock, reset);

endmodule
