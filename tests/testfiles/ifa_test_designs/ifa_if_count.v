module anda(
    a, clk,
    c);
  input wire a;
  output wire c;

  always @posedge clk
    begin
      if (a)
        c <= 0;
      else
        c <= 1;
    end

endmodule