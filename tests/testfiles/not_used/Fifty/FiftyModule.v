module M50 (in_0,in_1,out_0,out_1);
   input [16:0] in_0;
   input [16:0] in_1;
   output [16:0] out_0;
   output [16:0] out_1;

   assign out_0 = in_0&in_1;
   assign out_1 = in_0|in_1;
endmodule



module M49 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	wire [16:0] w_5;
   

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_5;

   M50 m0(in_0,in_1,w_0,w_1);
   M50 m1(in_2,in_3,w_2,w_3);
   M50 m2(in_4,in_5,w_4,w_5);

endmodule


module M48 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M49 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M47 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M48 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M46 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M47 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M45 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M46 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M44 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M45 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M43 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M44 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M42 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M43 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M41 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M42 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M40 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M41 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M39 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M40 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M38 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M39 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M37 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M38 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M36 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M37 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M35 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;


	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M36 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M34 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M35 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M33 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M34 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M32 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;


	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M33 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M31 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M32 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M30 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M31 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M29 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M30 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M28 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M29 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M27 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M28 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M26 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M27 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M25 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M26 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M24 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M25 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M23 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M24 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M22 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M23 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M21 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M22 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M20 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M21 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M19 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M20 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M18 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M19 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M17 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M18 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M16 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M17 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M15 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M16 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M14 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M15 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M13 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M14 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M12 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M13 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M11 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M12 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M10 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;


	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M11 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M9 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M10 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M8 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M9 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M7 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M8 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M6 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M7 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M5 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;


	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M6 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M4 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;


	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M5 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M3 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M4 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M2 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_3;

   M3 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);

endmodule


module M1 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;
	wire [16:0] w_4;
   wire [16:0] w_5;
	wire [16:0] w_6;
	wire [16:0] w_7;
	wire [16:0] w_8;
	wire [16:0] w_9;
	wire [16:0] w_10;
	wire [16:0] w_11;
	wire [16:0] w_12;
	wire [16:0] w_13;
	wire [16:0] w_14;
	wire [16:0] w_15;
	wire [16:0] w_16;
	wire [16:0] w_17;
	wire [16:0] w_18;
	wire [16:0] w_19;

	assign w_15 = w_0 + w_1;
	assign w_16 = w_2 & w_3;
	assign w_17 = w_4 | w_5;
	assign w_18 = w_6 + w_7;
	assign w_19 = w_8 & w_9;
	assign out_0 = w_10 | w_15;
	assign out_1 = w_11 + w_16;
	assign out_2 = w_12 & w_17;
	assign out_3 = w_13 | w_18;
	assign out_4 = w_14 + w_19;
   M2 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);
   M2 m1(in_0,in_1,in_2,in_3,in_4,in_5,w_5,w_6,w_7,w_8,w_9);
   M2 m2(in_0,in_1,in_2,in_3,in_4,in_5,w_10,w_11,w_12,w_13,w_14);
   M2 m3(in_0,in_1,in_2,in_3,in_4,in_5,w_15,w_16,w_17,w_18,w_19);

endmodule

module mux2to1(in_0,in_1,in_2,in_3,in_4,in_5,data_out);
input [16:0] in_0;
input [16:0] in_1;
input [16:0] in_2;
input [16:0] in_3;
input [16:0] in_4;
input [16:0] in_5;
output [16:0] data_out;
reg [16:0] data_out;
always @(in_0,in_1,in_2,in_3,in_4,in_5)
	begin
		if(in_5== 0)
			data_out =in_0+in_1+in_2+in_3+in_4+in_5;
		else
			data_out = in_4 & in_5;

	end
endmodule


module M0 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [16:0] in_0;
   input [16:0] in_1;
   input [16:0] in_2;
   input [16:0] in_3;
   input [16:0] in_4;
   input [16:0] in_5;
   output [16:0] out_0;
   output [16:0] out_1;
   output [16:0] out_2;
   output [16:0] out_3;
   output [16:0] out_4;

	wire [16:0] w_0;
	wire [16:0] w_1;
	wire [16:0] w_2;
	wire [16:0] w_3;

	wire [16:0] w_6;
	wire [16:0] w_7;
	wire [16:0] w_8;
	wire [16:0] w_9;
	wire [16:0] w_10;
   wire [16:0] w_11;
	wire [16:0] w_12;
	wire [16:0] w_13;
	wire [16:0] w_14;
	wire [16:0] w_15;

	assign out_0 =  w_0;
	assign out_1 =  w_1;
	assign out_2 =  w_2 + w_15;
	assign out_3 =  w_3 + w_10;
	assign w_0 = w_6 & w_11;
	assign w_1 = w_7 | w_12;
	assign w_2 = w_8 + w_13;
	assign w_3 = w_9 & w_14;


   M1 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_6,w_7,w_8,w_9,w_10);
   M1 m1(in_0,in_1,in_2,in_3,in_4,in_5,w_11,w_12,w_13,w_14,w_15);
   mux2to1  m2(in_0,in_1,in_2,in_3,in_4,in_5,out_4);



endmodule
