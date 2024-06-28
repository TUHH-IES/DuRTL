module M2 (in_0,in_1,out_0,out_1);
   input [4:0] in_0;
   input [4:0] in_1;
   output [4:0] out_0;
   output [4:0] out_1;

   assign out_0 = in_0 & in_1;
   assign out_1 = in_0 | in_1;
endmodule



module M1 (in_0,in_1,out_0);
   input [4:0] in_0;
   input [4:0] in_1;
   output [4:0] out_0;

	wire [4:0] w_0;
	wire [4:0] w_1;
	wire [4:0] w_2;
	wire [4:0] w_3;
	wire [4:0] w_4;
	wire [4:0] w_5;
	wire [4:0] w_6;
	wire [4:0] w_7;

	assign w_3 = w_0 + w_1;
	assign out_0 = w_2 & w_3;
   M2 m0(in_0,in_1,w_0,w_1);
   M2 m1(in_0,in_1,w_2,w_3);

endmodule

module mux2to1(in_0,in_1,in_2,data_out);
input [4:0] in_0;
input [4:0] in_1;
input [4:0] in_2;
output [4:0] data_out;
reg [4:0] data_out;
always @(in_0,in_1,in_2)
	begin
		if(in_2== 0)
			data_out =in_0+in_1+in_2;
		else
			data_out = in_1 & in_2;

	end
endmodule


module M0 (in_0,in_1,out_0);
   input [4:0] in_0;
   input [4:0] in_1;
   output [4:0] out_0;

	wire [4:0] w_0;
	wire [4:0] w_1;
	wire [4:0] w_2;
	wire [4:0] w_3;

	assign out_0 =  w_0;
	assign w_0 = w_0 & w_1;

   M1 m0(in_0,in_1,w_2);
   M1 m1(in_0,in_1,w_3);
   mux2to1  m2(in_0,in_1,w_0,out_0);


endmodule
