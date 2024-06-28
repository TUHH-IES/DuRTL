module design_tb;
   reg a,b,s;
   wire c;
   reg [31:0] a_t,b_t,s_t;
   wire [31:0] c_t;

   reg [128:0] read_data [0:128];
   integer i, t;

   `include "tag_tb.v";

// values for hierarchy ift flow test
  initial begin
      a = 0;
      b = 0;
      s = 0;
      #1;
      a = 0;
      b = 1;
      s = 0;
      #1;
      a = 1;
      b = 0;
      s = 0;
      #1;
      a = 1;
      b = 1;
      s = 0;
      #1;
      a = 0;
      b = 0;
      s = 1;
      #1;
      a = 0;
      b = 1;
      s = 1;
      #1;
      a = 1;
      b = 0;
      s = 1;
      #1;
      a = 1;
      b = 1;
      s = 1;
      $finish;
   end

   design t1(.a(a), .a_t(a_t), .b(b), .b_t(b_t), .s(s), .s_t(s_t), .c(c), .c_t(c_t));
      
   initial begin
      $readmemb(`INPUT_FILE_NAME, read_data);
      for(i = 0; i<=11; i++) begin
      {a_t, b_t, s_t, t} =  read_data[i];
      #t;
      end
   end

   initial
      begin
         $dumpfile(`DUMP_FILE_NAME);
        $dumpvars(1);		//writing the vcd file
      end
      
   //initial
      //$monitor("At time %t, value = %h (%0d)", $time, a, b, c);

endmodule
