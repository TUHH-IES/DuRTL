module counter( clk, clk_t, rst, rst_t, en, en_t, count, count_t);

	input clk, rst, en;
	input [31:0] clk_t;
	input [31:0] rst_t;
	input [31:0] en_t;
	output reg[3:0] count;
	output reg[31:0] count_t;

	initial begin
	    count_t = 0;
	end

	always @(posedge clk)
		if (rst) begin
			count <= 4'd0;
			count_t <= rst_t;
	    end else if (en) begin
			count <= count + 4'd1;
			count_t <= count_t | en_t;
        end
endmodule
