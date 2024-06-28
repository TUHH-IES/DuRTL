/* Generated by Yosys 0.33+72 (git sha1 ac8b31e00, clang 10.0.0-4ubuntu1 -fPIC -Os) */

module SPI_Slave(reset, In_Data, slaveCPOL, slaveCPHA, slaveSCLK, slaveCS_, slaveMISO, slaveMOSI, slave_start, read_data);
  wire [7:0] _000_;
  wire _001_;
  wire [7:0] _002_;
  wire _003_;
  wire [7:0] _004_;
  wire _005_;
  wire [7:0] _006_;
  wire _007_;
  wire [7:0] _008_;
  wire _009_;
  wire [7:0] _010_;
  wire _011_;
  wire _012_;
  wire _013_;
  wire _014_;
  wire _015_;
  wire _016_;
  wire _017_;
  wire _018_;
  wire _019_;
  wire _020_;
  wire _021_;
  wire _022_;
  wire _023_;
  wire _024_;
  wire _025_;
  wire _026_;
  wire _027_;
  wire _028_;
  wire _029_;
  wire _030_;
  wire _031_;
  wire _032_;
  wire _033_;
  wire _034_;
  wire _035_;
  wire _036_;
  wire _037_;
  wire _038_;
  wire _039_;
  wire _040_;
  wire _041_;
  wire _042_;
  wire [7:0] _043_;
  wire _044_;
  wire [7:0] _045_;
  wire _046_;
  wire [7:0] _047_;
  wire _048_;
  wire [7:0] _049_;
  wire _050_;
  wire [7:0] _051_;
  wire _052_;
  wire [7:0] _053_;
  wire _054_;
  wire _055_;
  wire _056_;
  wire _057_;
  wire _058_;
  wire _059_;
  wire _060_;
  wire [7:0] _061_;
  wire _062_;
  wire [7:0] _063_;
  wire _064_;
  wire _065_;
  wire _066_;
  wire _067_;
  wire _068_;
  wire [7:0] _069_;
  wire _070_;
  wire _071_;
  wire _072_;
  input [7:0] In_Data;
  wire [7:0] In_Data;
  reg [7:0] memReg;
  output [7:0] read_data;
  wire [7:0] read_data;
  input reset;
  wire reset;
  input slaveCPHA;
  wire slaveCPHA;
  input slaveCPOL;
  wire slaveCPOL;
  input slaveCS_;
  wire slaveCS_;
  output slaveMISO;
  reg slaveMISO;
  input slaveMOSI;
  wire slaveMOSI;
  input slaveSCLK;
  wire slaveSCLK;
  input slave_start;
  wire slave_start;
  assign _015_ = ~ slaveCS_;
  assign _017_ = ! { slaveCPOL, slaveCPHA };
  assign _018_ = { slaveCPOL, slaveCPHA } == 2'h2;
  assign _019_ = { slaveCPOL, slaveCPHA } == 2'h3;
  assign _020_ = { slaveCPOL, slaveCPHA } == 2'h1;
  assign _021_ = ~ slaveCS_;
  assign _023_ = { slaveCPOL, slaveCPHA } == 2'h1;
  assign _024_ = { slaveCPOL, slaveCPHA } == 2'h3;
  assign _025_ = ! { slaveCPOL, slaveCPHA };
  assign _026_ = { slaveCPOL, slaveCPHA } == 2'h2;
  assign _027_ = _017_ || _018_;
  assign _028_ = _020_ || _019_;
  assign _029_ = _023_ || _024_;
  assign _030_ = _025_ || _026_;
  always @(posedge slaveSCLK)
    slaveMISO <= _041_;
  always @(posedge slaveSCLK)
    memReg <= _047_;
  always @(negedge slaveSCLK)
    slaveMISO <= _071_;
  always @(negedge slaveSCLK)
    memReg <= _069_;
  always @(negedge slaveCS_)
    slaveMISO <= memReg[0];
  always @(posedge reset)
    memReg <= In_Data;
  assign _031_ = _032_ ? memReg[0] : slaveMISO;
  assign _033_ = _034_ ? _031_ : 1'hx;
  assign _035_ = slaveCS_ ? 1'hx : _033_;
  assign _037_ = _038_ ? _014_ : slaveMISO;
  assign _039_ = slaveCS_ ? 1'hx : _037_;
  assign _041_ = slaveCS_ ? slaveMISO : _013_;
  assign _043_ = _044_ ? { slaveMOSI, memReg[7:1] } : memReg;
  assign _045_ = _046_ ? _043_ : memReg;
  assign _047_ = slaveCS_ ? memReg : _045_;
  assign _049_ = _050_ ? { slaveMOSI, memReg[7:1] } : memReg;
  assign _051_ = _052_ ? _049_ : 8'hxx;
  assign _053_ = slaveCS_ ? 8'hxx : _051_;
  assign _055_ = _056_ ? memReg[0] : slaveMISO;
  assign _057_ = _058_ ? _055_ : 1'hx;
  assign _059_ = slaveCS_ ? 1'hx : _057_;
  assign _061_ = _062_ ? _008_ : memReg;
  assign _063_ = slaveCS_ ? 8'hxx : _061_;
  assign _065_ = _066_ ? _009_ : slaveMISO;
  assign _067_ = slaveCS_ ? 1'hx : _065_;
  assign _069_ = slaveCS_ ? memReg : _006_;
  assign _071_ = slaveCS_ ? slaveMISO : _007_;
  assign read_data = memReg;
  assign _011_ = _012_;
  assign _002_ = _004_;
  assign _003_ = _005_;
  assign _001_ = memReg[0];
  assign _000_ = In_Data;
  assign _032_ = _029_;
  assign _034_ = _022_;
  assign _036_ = _021_;
  assign _014_ = _035_;
  assign _038_ = _022_;
  assign _040_ = _021_;
  assign _013_ = _039_;
  assign _042_ = _021_;
  assign _012_ = _041_;
  assign _044_ = _030_;
  assign _046_ = _022_;
  assign _048_ = _021_;
  assign _010_ = _047_;
  assign _050_ = _028_;
  assign _052_ = _016_;
  assign _054_ = _015_;
  assign _008_ = _053_;
  assign _056_ = _027_;
  assign _058_ = _016_;
  assign _060_ = _015_;
  assign _009_ = _059_;
  assign _062_ = _016_;
  assign _064_ = _015_;
  assign _006_ = _063_;
  assign _066_ = _016_;
  assign _068_ = _015_;
  assign _007_ = _067_;
  assign _070_ = _015_;
  assign _004_ = _069_;
  assign _072_ = _015_;
  assign _005_ = _071_;
  assign _016_ = slave_start;
  assign _022_ = slave_start;
endmodule
