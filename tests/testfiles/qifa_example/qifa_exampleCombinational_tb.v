module qifa_tb();
    reg logic [2:0] secret;
    reg logic [1:0] inp;
    wire logic [5:0] out;

  qifa_exampleSimple top(secret, inp, out);


  initial begin
    secret= 2;
    inp= 1;
    # 2
    inp= 0;
    # 4
    secret = 4;
    # 4
    inp = 2;
    # 10
    $finish;
  end

  initial begin
    $dumpfile("trace.vcd");
    $dumpvars();
  end

endmodule    
