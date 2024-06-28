// A shift register that may qualify a pipeline:
// * Data is consistently pushed from input to output cycle by cycle
// * Names match the one of a typical pipeline in a processor

module sregPipeline(inst, out, enable, clk, rst);
  input [8:0]inst;
  input enable, clk, rst;
  output [8:0]out;
  reg [8:0]out;
  reg [8:0] stFetch;
  reg [8:0] stDecode;
  reg [8:0] stOperands;
  reg [8:0] stExecute;
  reg [8:0] stWriteback;

  always @(posedge(clk))
  begin
    if (rst==1) 
    begin
        out <= 0;
        stWriteback <= 0;
        stExecute <= 0;
        stOperands <= 0;
        stDecode <= 0;
        stFetch <= 0;
    end
    else
      if (enable==1) 
      begin
        out <= stWriteback;
        stWriteback <= stExecute;
        stExecute <= stOperands;
        stOperands <= stDecode;
        stDecode <= stFetch;
        stFetch <= inst;
      end
  end

  
endmodule

