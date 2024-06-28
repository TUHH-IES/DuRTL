module skeleton(clk,a,b,c,d,e,f);
input clk,a,b,c;
output d,e,f;

reg d1, d2, e2;
wire e1;
assign d = d1;
assign e = e2;

always@(posedge clk) begin
    if (a == 0)
        d1 <= e1;
    else
        d1 <= a;
    d2 <= d1;
    e2 <= e1;
end

assign e1 = d2 & b;
assign f = b & c;
endmodule