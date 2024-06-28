module hierarchy_tb;
   reg a,b;
   wire c;
   reg [31:0] a_t,b_t;
   wire [31:0] c_t;

   reg [95:0] read_data [0:95];
   integer i, t;
   
   initial begin
      a = 0;
      b = 0;
      #8;
      a = 0;
      b = 1;
      #8;
      a = 1;
      b = 0;
      #8;
      a = 1;
      b = 1;
      #8;
   
   end

   hierarchy t1(.a(a), .a_t(a_t), .b(b), .b_t(b_t), .c(c), .c_t(c_t));
      
   initial begin
      $readmemb(`INPUT_FILE_NAME, read_data);
      for(i = 0; i<=15; i++) begin
      {a_t, b_t, t} =  read_data[i];
      #t;
      end
   end

   initial
      begin
         $dumpfile(`DUMP_FILE_NAME);
        $dumpvars();		//writing the vcd file
      end
      
   //initial
      //$monitor("At time %t, value = %h (%0d)", $time, a, b, c);

endmodule
