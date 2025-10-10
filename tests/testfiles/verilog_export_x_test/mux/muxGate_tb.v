module testDesign_tb;
   reg a, b, s;
   reg [31:0] a_t, b_t, s_t;
   output c;
   output [31:0] c_t;

   initial begin
      #0
      a = 0;
      b = 0;
      s = 0;
      a_t = 32'b00000000000000000000000000000001;
      b_t = 32'b00000000000000000000000000000010;
      s_t = 32'b00000000000000000000000000000100;
      #2
      a = 0;
      b = 1'bx;
      s = 1;
      #2
      a = 0;
      b = 1;
      s = 1'bx;
      #2
      a = 1'bx;
      b = 0;
      s = 1'bx;
      #2
         $finish;
   end


  muxGate dut(.a(a), .a_t(a_t), .b(b), .b_t(b_t), .s(s), .s_t(s_t), .c(c), .c_t(c_t));
	
	initial
    	begin
    		$dumpfile(`DUMP_FILE_NAME);
    		$dumpvars(0);		//writing the vcd file
    	end

endmodule
