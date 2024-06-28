module design_tb;
   reg a,b,s;
   wire c;
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
   end

   des t1(.a(a), .s(s), .b(b), .c(c));
  
   initial
      begin
         $dumpfile(`DUMP_FILE_NAME);
        $dumpvars(1);		//writing the vcd file
      end
      
   //initial
      //$monitor("At time %t, value = %h (%0d)", $time, a, b, c);

endmodule
