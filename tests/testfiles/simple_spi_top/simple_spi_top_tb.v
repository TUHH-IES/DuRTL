/////////////////////////////////////////////////////////////////////
////                                                             ////
////  simple_spi ('HC11 compatible) testbench                    ////
////                                                             ////
////  Author: Richard Herveille                                  ////
////          richard@asics.ws                                   ////
////          www.asics.ws                                       ////
////                                                             ////
////////////////////////////////////////////////////////////////////
////                                                             ////
//// Copyright (C) 2004 Richard Herveille                        ////
////                    richard@asics.ws                         ////
////                                                             ////
//// This source file may be used and distributed without        ////
//// restriction provided that this copyright statement is not   ////
//// removed from the file and that any derivative work contains ////
//// the original copyright notice and the associated disclaimer.////
////                                                             ////
////     THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY     ////
//// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   ////
//// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS   ////
//// FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR      ////
//// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,         ////
//// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    ////
//// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE   ////
//// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR        ////
//// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF  ////
//// LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT  ////
//// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  ////
//// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         ////
//// POSSIBILITY OF SUCH DAMAGE.                                 ////
////                                                             ////
/////////////////////////////////////////////////////////////////////

//  CVS Log
//
//  $Id: tst_bench_top.v,v 1.1 2004-02-28 16:01:47 rherveille Exp $
//
//  $Date: 2004-02-28 16:01:47 $
//  $Revision: 1.1 $
//  $Author: rherveille $
//  $Locker:  $
//  $State: Exp $
//
// Change History:
//               $Log: not supported by cvs2svn $
//
//

`timescale 1ns / 1ns

module tst_bench_top();

	//
	// wires && regs
	//
	reg  clk;
	reg  rstn;

	wire [31:0] adr;
	wire [ 7:0] dat_i, dat_o;
	wire we;
	wire stb;
	wire cyc;
	wire ack;
	wire inta;

	reg [1:0] cpol, cpha;
	reg [2:0] e;

	wire sck, mosi, miso;
	reg [7:0] q;

	parameter SPCR = 2'b00;
	parameter SPSR = 2'b01;
	parameter SPDR = 2'b10;
	parameter SPER = 2'b11;

	//
	// Module body
	//
	integer n;

	// generate clock
	always #5 clk = ~clk;

	// hookup wishbone master model
	wb_master_model #(8, 32) u0 (
		.clk (clk),
		.rst (rstn),
		.adr (adr),
		.din (dat_i),
		.dout(dat_o),
		.cyc (cyc),
		.stb (stb),
		.we  (we),
		.sel (),
		.ack (ack),
		.err (1'b0),
		.rty (1'b0)
	);

	// hookup spi core
	simple_spi_top spi_top (
		// wishbone interface
		.clk_i (clk),
		.rst_i (rstn),
		.cyc_i (cyc),
		.stb_i (stb),
		.adr_i (adr[1:0]),
		.we_i  (we),
		.dat_i (dat_o),
		.dat_o (dat_i),
		.ack_o (ack),
		.inta_o(inta),

		.sck_o (sck),
		.mosi_o(mosi),
		.miso_i(miso)
	);

	// hookup spi slave model
	spi_slave_model spi_slave (
		.csn(1'b0),
		.sck(sck),
		.di(mosi),
		.do(miso)
	);

	initial
	  begin
	      `ifdef WAVES
	         $shm_open("waves");
	         $shm_probe("AS",tst_bench_top,"AS");
	         $display("INFO: Signal dump enabled ...\n\n");
	      `endif

//	      force spi_slave.debug = 1'b1; // enable spi_slave debug information
	      force spi_slave.debug = 1'b0; // disable spi_slave debug information

	      $display("\nstatus: %t Testbench started\n\n", $time);


	      // initially values
	      clk = 0;

	      // reset system
	      rstn = 1'b1; // negate reset
	      #2;
	      rstn = 1'b0; // assert reset
	      repeat(1) @(posedge clk);
	      rstn = 1'b1; // negate reset

	      $display("status: %t done reset", $time);

	      @(posedge clk);

	      //
	      // program core
	      //
	      for (cpol=0; cpol<=1; cpol=cpol+1)
	      for (cpha=0; cpha<=1; cpha=cpha+1) 
	      for (e=0; e<=3; e=e+1)
	      begin
	          //set cpol/cpha in spi slave model
	          force spi_slave.cpol=cpol[0];
	          force spi_slave.cpha=cpha[0];
	          $display("cpol:%b, cpha:%b, e:%b", cpol[0],cpha[0],e[1:0]);

	          // program internal registers

	          // load control register
	          u0.wb_write(1, SPCR, {4'b0101,cpol[0],cpha[0],e[1:0]} );
	          //verify control register
	          u0.wb_cmp  (0, SPCR, {4'b0101,cpol[0],cpha[0],e[1:0]} );

	          
	          // load extended control register
	          u0.wb_write(1,SPER,8'h0);
	          //verify extended control register
	          u0.wb_cmp (0,SPER,8'h0);

	          //fill memory
	          for(n=0;n<8;n=n+1) begin
	            u0.wb_write(1,SPDR,{cpol[0],cpha[0],e[1:0],n[3:0]});
	            //wait for transfer to finish
	            u0.wb_read(1,SPSR,q);
	            while(~q[7]) u0.wb_read(1,SPSR,q);
	            //clear 'spif' bit
	            u0.wb_write(1,SPSR,8'h80);
	          end

	          //verify memory
	          for(n=0;n<8;n=n+1) begin
	            u0.wb_write(1,SPDR,~n);
	            //wait for transfer to finish
	            u0.wb_read(1,SPSR,q);
	            while(~q[7]) u0.wb_read(1,SPSR,q);
	            //clear 'spif' bit
	            u0.wb_write(1,SPSR,8'h80);
	            //verify memory content
	            u0.wb_cmp(0,SPDR,{cpol[0],cpha[0],e[1:0],n[3:0]});
	          end
	      end

	      // check tip bit
//	      u0.wb_read(1, SR, q);
//	      while(q[1])
//	      u0.wb_read(1, SR, q); // poll it until it is zero
//	      $display("status: %t tip==0", $time);

	      #250000; // wait 250us
	      $display("\n\nstatus: %t Testbench done", $time);
	      $finish;
	  end

 initial
      begin
         $dumpfile(`DUMP_FILE_NAME);
        $dumpvars(1);		//writing the vcd file
      end

endmodule

///////////////////////////////////////////////////////////////////////
////                                                               ////
////  WISHBONE rev.B2 Wishbone Master model                        ////
////                                                               ////
////                                                               ////
////  Author: Richard Herveille                                    ////
////          richard@asics.ws                                     ////
////          www.asics.ws                                         ////
////                                                               ////
////  Downloaded from: http://www.opencores.org/projects/mem_ctrl  ////
////                                                               ////
///////////////////////////////////////////////////////////////////////
////                                                               ////
//// Copyright (C) 2001 Richard Herveille                          ////
////                    richard@asics.ws                           ////
////                                                               ////
//// This source file may be used and distributed without          ////
//// restriction provided that this copyright statement is not     ////
//// removed from the file and that any derivative work contains   ////
//// the original copyright notice and the associated disclaimer.  ////
////                                                               ////
////     THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY       ////
//// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED     ////
//// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     ////
//// FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR        ////
//// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,           ////
//// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES      ////
//// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE     ////
//// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR          ////
//// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    ////
//// LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT    ////
//// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT    ////
//// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           ////
//// POSSIBILITY OF SUCH DAMAGE.                                   ////
////                                                               ////
///////////////////////////////////////////////////////////////////////

//  CVS Log
//
//  $Id: wb_master_model.v,v 1.1 2004-02-28 16:01:47 rherveille Exp $
//
//  $Date: 2004-02-28 16:01:47 $
//  $Revision: 1.1 $
//  $Author: rherveille $
//  $Locker:  $
//  $State: Exp $
//
// Change History:
//

module wb_master_model(clk, rst, adr, din, dout, cyc, stb, we, sel, ack, err, rty);

parameter dwidth = 32;
parameter awidth = 32;

input               clk, rst;
output [awidth:1]   adr;
input  [dwidth:1]   din;
output [dwidth:1]   dout;
output              cyc, stb;
output              we;
output [dwidth/8:1] sel;
input               ack, err, rty;

////////////////////////////////////////////////////////////////////
//
// Local Wires
//

reg [awidth:1]   adr;
reg [dwidth:1]   dout;
reg              cyc, stb;
reg              we;
reg [dwidth/8:1] sel;

reg [dwidth:1]   q;

////////////////////////////////////////////////////////////////////
//
// Memory Logic
//

initial
	begin
		//adr = 32'hxxxx_xxxx;
		//adr = 0;
		adr  = {awidth{1'bx}};
		dout = {dwidth{1'bx}};
		cyc  = 1'b0;
		stb  = 1'bx;
		we   = 1'hx;
		sel  = {dwidth/8{1'bx}};
		#1;
		$display("\nINFO: WISHBONE MASTER MODEL INSTANTIATED (%m)\n");
	end

////////////////////////////////////////////////////////////////////
//
// Wishbone write cycle
//

task wb_write;
	input   delay;
	integer delay;

	input	[awidth:1] a;
	input	[dwidth:1] d;

	begin

		// wait initial delay
		repeat(delay) @(posedge clk);

		// assert wishbone signal
		#1;
		adr  = a;
		dout = d;
		cyc  = 1'b1;
		stb  = 1'b1;
		we   = 1'b1;
		sel  = {dwidth/8{1'b1}};
		@(posedge clk);

		// wait for acknowledge from slave
		while(~ack) @(posedge clk);

		// negate wishbone signals
		#1;
		cyc  = 1'b0;
		stb  = 1'bx;
		adr  = {awidth{1'bx}};
		dout = {dwidth{1'bx}};
		we   = 1'hx;
		sel  = {dwidth/8{1'bx}};

	end
endtask

////////////////////////////////////////////////////////////////////
//
// Wishbone read cycle
//

task wb_read;
	input   delay;
	integer delay;

	input   [awidth:1] a;
	output  [dwidth:1] d;

	begin

		// wait initial delay
		repeat(delay) @(posedge clk);

		// assert wishbone signals
		#1;
		adr  = a;
		dout = {dwidth{1'bx}};
		cyc  = 1'b1;
		stb  = 1'b1;
		we   = 1'b0;
		sel  = {dwidth/8{1'b1}};
		@(posedge clk);

		// wait for acknowledge from slave
		while(~ack) @(posedge clk);

		// negate wishbone signals
		#1;
		cyc  = 1'b0;
		stb  = 1'bx;
		adr  = {awidth{1'bx}};
		dout = {dwidth{1'bx}};
		we   = 1'hx;
		sel  = {dwidth/8{1'bx}};
		d    = din;

	end
endtask

////////////////////////////////////////////////////////////////////
//
// Wishbone compare cycle (read data from location and compare with expected data)
//

task wb_cmp;
	input   delay;
	integer delay;

	input [awidth:1] a;
	input [dwidth:1] d_exp;

	begin
	  wb_read (delay, a, q);

	  if (d_exp !== q)
	    $display("Data compare error. Received %h, expected %h at time %t", q, d_exp, $time);
	end
endtask

endmodule


/////////////////////////////////////////////////////////////////////
////                                                             ////
////  SPI Slave Model                                            ////
////                                                             ////
////  Authors: Richard Herveille (richard@asics.ws) www.asics.ws ////
////                                                             ////
////  http://www.opencores.org/projects/simple_spi/              ////
////                                                             ////
/////////////////////////////////////////////////////////////////////
////                                                             ////
//// Copyright (C) 2004 Richard Herveille                        ////
////                         richard@asics.ws                    ////
////                                                             ////
//// This source file may be used and distributed without        ////
//// restriction provided that this copyright statement is not   ////
//// removed from the file and that any derivative work contains ////
//// the original copyright notice and the associated disclaimer.////
////                                                             ////
////     THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY     ////
//// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   ////
//// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS   ////
//// FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR      ////
//// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,         ////
//// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    ////
//// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE   ////
//// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR        ////
//// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF  ////
//// LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT  ////
//// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  ////
//// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE         ////
//// POSSIBILITY OF SUCH DAMAGE.                                 ////
////                                                             ////
/////////////////////////////////////////////////////////////////////

//  CVS Log
//
//  $Id: spi_slave_model.v,v 1.1 2004-02-28 16:01:47 rherveille Exp $
//
//  $Date: 2004-02-28 16:01:47 $
//  $Revision: 1.1 $
//  $Author: rherveille $
//  $Locker:  $
//  $State: Exp $
//
// Change History:
//               $Log: not supported by cvs2svn $
//
//
//


// Requires: Verilog2001

module spi_slave_model (
	input  wire csn,
	input  wire sck,
	input  wire di,
	output wire do
);

	//
	// Variable declaration
	//
	wire debug = 1'b1;

	wire cpol = 1'b0;
	wire cpha  = 1'b0;

	reg [7:0] mem [7:0]; // initiate memory
	reg [2:0] mem_adr;   // memory address
	reg [7:0] mem_do;    // memory data output

	reg [7:0] sri, sro;  // 8bit shift register

	reg [2:0] bit_cnt;
	reg       ld;

	wire clk;

	//
	// module body
	//

	assign clk = cpol ^ cpha ^ sck;

	// generate shift registers
	always @(posedge clk)
	  sri <= #1 {sri[6:0],di};

	always @(posedge clk)
	  if (&bit_cnt)
	    sro <= #1 mem[mem_adr];
	  else
	    sro <= #1 {sro[6:0],1'bx};

	assign do = sro[7];

	//generate bit-counter
	always @(posedge clk, posedge csn)
	  if(csn)
	    bit_cnt <= #1 3'b111;
	  else
	    bit_cnt <= #1 bit_cnt - 3'h1;

	//generate access done signal
        always @(posedge clk)
	  ld <= #1 ~(|bit_cnt);

	always @(negedge clk)
          if (ld) begin
	    mem[mem_adr] <= #1 sri;
	    mem_adr      <= #1 mem_adr + 1'b1;
	  end

	initial
	begin
	  bit_cnt=3'b111;
	  mem_adr = 0;
	  sro = mem[mem_adr];
	end
endmodule


