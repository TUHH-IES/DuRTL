module qifa_tb();
    reg logic clk;
    reg logic rst;
    reg logic [2:0] secret;
    reg logic [1:0] inp;
    wire logic [5:0] out;

  always #1 clk <= !clk;
  qifa_exampleSimple top(clk, rst, secret, inp, out);


  initial begin
    clk=0;
    rst= 1;
    secret= 2;
    inp= 1;
    # 3
    rst= 0;
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
