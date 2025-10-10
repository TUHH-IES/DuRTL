module ifa_seq_4(
    clk,a,b,c,
    d,e,f,g,h,j,k,l);
  input wire clk;
  input wire [3:0] a,b,c;
  output reg  [3:0] d,e,f,g,h,j,k,l;
  
  always @(posedge clk) begin
    d <= a & b & c;
    e <= a | b | c;
  end
   always @(posedge clk) begin
    f <= d;
    g <= e;
  end
   always @(posedge clk) begin
    h <= ~f;
    j <= ~g;
  end
   always @(posedge clk) begin
    k <= ~h;
    l <= j;
  end

endmodule