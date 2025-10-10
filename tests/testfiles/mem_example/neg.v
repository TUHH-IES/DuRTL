module mem_example(
    a,b,clk,
    c);
  input clk;
  input wire [4:0] a;
  input wire [4:0] b;
  output wire [4:0] c;

  reg [31:0] k [0:15];

  always @(posedge clk) begin
    
    
  end

endmodule