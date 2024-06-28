module M6 (in_0,in_1,out_0,out_1);
   input [6:0] in_0;
   input [6:0] in_1;
   output [6:0] out_0;
   output [6:0] out_1;

   assign out_0 = in_0 & in_1;
   assign out_1 = in_0 | in_1;
endmodule



module M5 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [6:0] in_0;
   input [6:0] in_1;
   input [6:0] in_2;
   input [6:0] in_3;
   input [6:0] in_4;
   input [6:0] in_5;
   output [6:0] out_0;
   output [6:0] out_1;
   output [6:0] out_2;
   output [6:0] out_3;
   output [6:0] out_4;

	wire [6:0] w_0;
	wire [6:0] w_1;
	wire [6:0] w_2;
	wire [6:0] w_3;
	wire [6:0] w_4;
	wire [6:0] w_5;
   wire [6:0] w_6;
   wire [6:0] w_7;
   wire [6:0] w_8;
   wire [6:0] w_9;
   wire [6:0] w_10;
   wire [6:0] w_11;
   wire [6:0] w_12;
   wire [6:0] w_13;
   wire [6:0] w_14;
   wire [6:0] w_15;
   wire [6:0] w_16;
   wire [6:0] w_17;
   wire [6:0] w_18;
   wire [6:0] w_19;
   wire [6:0] w_20;
   wire [6:0] w_21;
   

   assign w_0 = w_10 + w_11;
   assign w_1 = w_12 + w_13;
   assign w_2 = w_14 + w_18; 
   assign w_3 = w_15 + w_19;
   assign w_4 = w_16 + w_20;
   assign w_5 = w_17 + w_21;
   assign w_6 = w_10 + w_11;
   assign w_7 = w_14 + w_18;
   assign w_8 = w_16 + w_20;
   assign w_9 = w_15 + w_19;

	assign out_0 = w_0 + w_1;
	assign out_1 = w_2 & w_3;
	assign out_2 = w_4 | w_5;
	assign out_3 = w_6 + w_7;
	assign out_4 = w_8 & w_9;

   M6 m0(in_0, in_5,w_12, w_13);
   M6 m1(in_1, in_4,w_16, w_17);
   M6 m2(in_2, in_3,w_10, w_11);
   M6 m3(in_3, in_2,w_14, w_15);
   M6 m4(in_4, in_1,w_18, w_19);
   M6 m5(in_5, in_0,w_20, w_21);

endmodule


module M4 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [6:0] in_0;
   input [6:0] in_1;
   input [6:0] in_2;
   input [6:0] in_3;
   input [6:0] in_4;
   input [6:0] in_5;
   output [6:0] out_0;
   output [6:0] out_1;
   output [6:0] out_2;
   output [6:0] out_3;
   output [6:0] out_4;

	wire [6:0] w_0;
	wire [6:0] w_1;
	wire [6:0] w_2;
	wire [6:0] w_3;
	wire [6:0] w_4;

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4;

   M5 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);
   

endmodule


module M3 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [6:0] in_0;
   input [6:0] in_1;
   input [6:0] in_2;
   input [6:0] in_3;
   input [6:0] in_4;
   input [6:0] in_5;
   output [6:0] out_0;
   output [6:0] out_1;
   output [6:0] out_2;
   output [6:0] out_3;
   output [6:0] out_4;

	wire [6:0] w_0;
	wire [6:0] w_1;
	wire [6:0] w_2;
	wire [6:0] w_3;
	wire [6:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_2;

   M4 m0(in_0, in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);
   

endmodule


module M2 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [6:0] in_0;
   input [6:0] in_1;
   input [6:0] in_2;
   input [6:0] in_3;
   input [6:0] in_4;
   input [6:0] in_5;
   output [6:0] out_0;
   output [6:0] out_1;
   output [6:0] out_2;
   output [6:0] out_3;
   output [6:0] out_4;

	wire [6:0] w_0;
	wire [6:0] w_1;
	wire [6:0] w_2;
	wire [6:0] w_3;
	wire [6:0] w_4;
	

	assign out_0 = w_0 + w_1;
	assign out_1 = w_1 & w_2;
	assign out_2 = w_2 | w_3;
	assign out_3 = w_3 + w_4;
	assign out_4 = w_4 & w_2;

   M3 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);
   

endmodule


module M1 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
   input [6:0] in_0;
   input [6:0] in_1;
   input [6:0] in_2;
   input [6:0] in_3;
   input [6:0] in_4;
   input [6:0] in_5;
   output [6:0] out_0;
   output [6:0] out_1;
   output [6:0] out_2;
   output [6:0] out_3;
   output [6:0] out_4;

	wire [6:0] w_0;
	wire [6:0] w_1;
	wire [6:0] w_2;
	wire [6:0] w_3;
	wire [6:0] w_4;
	
	wire [6:0] w_6;
	wire [6:0] w_7;
	wire [6:0] w_8;
	wire [6:0] w_9;
	wire [6:0] w_10;
	
	wire [6:0] w_12;
	wire [6:0] w_13;
	wire [6:0] w_14;
	wire [6:0] w_15;
	wire [6:0] w_16;
	wire [6:0] w_17;
	wire [6:0] w_18;
	wire [6:0] w_19;
   wire [6:0] w_20;
   wire [6:0] w_21;


	assign w_12 = w_0 + w_1;
	assign w_13 = w_2 & w_3;
	assign w_14 = w_4;
	assign w_15 = w_6 + w_7;
	assign w_16 = w_8 & w_9;
   assign w_17 = w_10 + w_1;

   
	assign out_0 = w_12;
	assign out_1 = w_13;
	assign out_2 = w_14;
	assign out_3 = w_15;
	assign out_4 = w_16;
   M2 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_0,w_1,w_2,w_3,w_4);
   M2 m1(in_0,in_1,in_2,in_3,in_4,in_5,w_6,w_7,w_8,w_9,w_10);
   
   
endmodule

module mux2to1(in_0,in_1,in_2,in_3,in_4,in_5,data_out);
input [6:0] in_0;
input [6:0] in_1;
input [6:0] in_2;
input [6:0] in_3;
input [6:0] in_4;
input [6:0] in_5;
output [6:0] data_out;
reg [6:0] data_out;
always @(in_0,in_1,in_2,in_3,in_4,in_5)
	begin
		if(in_5== 0)
			data_out =in_0+in_1+in_2+in_3+in_4+in_5;
		else
			data_out = in_4 & in_5;

	end
endmodule


module M0 (in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4, out_5);
   input [6:0] in_0;
   input [6:0] in_1;
   input [6:0] in_2;
   input [6:0] in_3;
   input [6:0] in_4;
   input [6:0] in_5;
   output [6:0] out_0;
   output [6:0] out_1;
   output [6:0] out_2;
   output [6:0] out_3;
   output [6:0] out_4;
   output [6:0] out_5;

	wire [6:0] w_0;
	wire [6:0] w_1;
	wire [6:0] w_2;
	wire [6:0] w_3;
	wire [6:0] w_4;
	wire [6:0] w_5;
	wire [6:0] w_6;
	wire [6:0] w_7;
	wire [6:0] w_8;
	wire [6:0] w_9;
	wire [6:0] w_10;
	wire [6:0] w_11;
	wire [6:0] w_12;
	wire [6:0] w_13;
	wire [6:0] w_14;
	wire [6:0] w_15;
   wire [6:0] w_16;
	wire [6:0] w_17;
	wire [6:0] w_18;
	wire [6:0] w_19;
	wire [6:0] w_20;

	assign out_0 =  w_16;
	assign out_1 =  w_17;
	assign out_2 =  w_18;
	assign out_3 =  w_19;
   assign out_5 =  w_20;

	assign w_0 = w_11 | w_6;
	assign w_1 = w_12 + w_7;
	assign w_2 = w_13 & w_8;
	assign w_3 = w_14 | w_9;
	assign w_4 = w_15 + w_10;
	assign w_5 = w_15 + w_10;
	assign w_16 = w_11 | w_6;
	assign w_17 = w_12 + w_7;
	assign w_18 = w_13 & w_8;
	assign w_19 = w_14 | w_9;
	assign w_20 = w_15 + w_10;

   M1 m0(in_0,in_1,in_2,in_3,in_4,in_5,w_6,w_7,w_8,w_9,w_10);
   M1 m1(in_0,in_1,in_2,in_3,in_4,in_5,w_11,w_12,w_13,w_14,w_15);
   mux2to1  m2(w_0,w_1,w_2,w_3,w_4,w_5,out_4);


endmodule
