/* Generated by Yosys 0.29 (git sha1 9c5a60eb201, clang 14.0.0-1ubuntu1 -fPIC -Os) */

module cisc_cpu_p(clk, rst, bus_A_ins, bus_in_ins, bus_A_data, bus_in_data, bus_out, bus_WE, bus_RE_data, bus_RE_ins, current_opcode);
  wire [31:0] _000_;
  wire [31:0] _001_;
  wire [31:0] _002_;
  wire [31:0] _003_;
  wire [31:0] _004_;
  wire [31:0] _005_;
  wire [31:0] _006_;
  wire [31:0] _007_;
  wire [31:0] _008_;
  wire [31:0] _009_;
  wire [31:0] _010_;
  wire [31:0] _011_;
  wire [31:0] _012_;
  wire [31:0] _013_;
  wire [31:0] _014_;
  wire [31:0] _015_;
  wire [31:0] _016_;
  wire [31:0] _017_;
  wire [31:0] _018_;
  wire _019_;
  wire [31:0] _020_;
  wire [31:0] _021_;
  wire [31:0] _022_;
  wire [31:0] _023_;
  wire [31:0] _024_;
  wire [31:0] _025_;
  wire [31:0] _026_;
  wire [31:0] _027_;
  wire [31:0] _028_;
  wire _029_;
  wire [31:0] _030_;
  wire [31:0] _031_;
  wire [31:0] _032_;
  wire [31:0] _033_;
  wire [31:0] _034_;
  wire [31:0] _035_;
  wire [31:0] _036_;
  wire [31:0] _037_;
  wire [31:0] _038_;
  wire [31:0] _039_;
  wire [31:0] _040_;
  wire [31:0] _041_;
  wire [31:0] _042_;
  wire [31:0] _043_;
  wire [31:0] _044_;
  wire [31:0] _045_;
  wire [31:0] _046_;
  wire [31:0] _047_;
  wire [31:0] _048_;
  wire _049_;
  wire _050_;
  wire _051_;
  wire _052_;
  wire _053_;
  wire _054_;
  wire _055_;
  wire _056_;
  wire _057_;
  wire _058_;
  wire _059_;
  wire _060_;
  wire _061_;
  wire _062_;
  wire _063_;
  wire _064_;
  wire _065_;
  wire _066_;
  wire _067_;
  wire _068_;
  wire _069_;
  wire _070_;
  wire _071_;
  wire _072_;
  wire _073_;
  wire _074_;
  wire _075_;
  wire _076_;
  wire _077_;
  wire _078_;
  wire _079_;
  wire _080_;
  wire _081_;
  wire _082_;
  wire _083_;
  wire _084_;
  wire _085_;
  wire _086_;
  wire _087_;
  wire _088_;
  wire _089_;
  wire _090_;
  wire _091_;
  wire _092_;
  wire _093_;
  wire _094_;
  wire [31:0] _095_;
  wire [31:0] _096_;
  wire [31:0] _097_;
  wire _098_;
  wire _099_;
  wire _100_;
  wire _101_;
  wire _102_;
  wire [31:0] _103_;
  wire _104_;
  wire _105_;
  wire _106_;
  wire _107_;
  wire _108_;
  wire _109_;
  wire _110_;
  wire _111_;
  wire _112_;
  wire _113_;
  wire _114_;
  wire _115_;
  wire _116_;
  wire _117_;
  wire _118_;
  wire _119_;
  wire [31:0] _120_;
  wire _121_;
  wire _122_;
  wire _123_;
  wire _124_;
  wire _125_;
  wire _126_;
  wire _127_;
  wire _128_;
  wire [31:0] _129_;
  wire [31:0] _130_;
  wire _131_;
  wire _132_;
  wire _133_;
  wire _134_;
  wire _135_;
  wire _136_;
  wire _137_;
  wire _138_;
  wire [31:0] _139_;
  wire [31:0] _140_;
  wire _141_;
  wire _142_;
  wire _143_;
  wire _144_;
  wire _145_;
  wire _146_;
  wire _147_;
  wire _148_;
  wire [31:0] _149_;
  wire [31:0] _150_;
  wire _151_;
  wire _152_;
  wire _153_;
  wire _154_;
  wire _155_;
  wire _156_;
  wire _157_;
  wire _158_;
  wire [31:0] _159_;
  wire [31:0] _160_;
  wire _161_;
  wire _162_;
  wire _163_;
  wire _164_;
  wire _165_;
  wire _166_;
  wire _167_;
  wire _168_;
  wire [31:0] _169_;
  wire [31:0] _170_;
  wire _171_;
  wire _172_;
  wire _173_;
  wire _174_;
  wire _175_;
  wire _176_;
  wire _177_;
  wire _178_;
  wire [31:0] _179_;
  wire [31:0] _180_;
  wire _181_;
  wire _182_;
  wire _183_;
  wire _184_;
  wire _185_;
  wire _186_;
  wire _187_;
  wire _188_;
  wire [31:0] _189_;
  wire [31:0] _190_;
  wire _191_;
  wire _192_;
  wire _193_;
  wire _194_;
  wire _195_;
  wire _196_;
  wire _197_;
  wire _198_;
  wire [31:0] _199_;
  wire [31:0] _200_;
  wire [31:0] _201_;
  wire [31:0] _202_;
  wire [31:0] _203_;
  wire [31:0] _204_;
  wire [31:0] _205_;
  wire [31:0] _206_;
  wire [31:0] _207_;
  wire _208_;
  wire [31:0] _209_;
  wire [31:0] _210_;
  wire [31:0] _211_;
  wire [31:0] _212_;
  wire [31:0] _213_;
  wire [31:0] _214_;
  wire [31:0] _215_;
  wire [31:0] _216_;
  wire [31:0] _217_;
  wire [31:0] _218_;
  wire [31:0] _219_;
  wire [31:0] _220_;
  wire [31:0] _221_;
  wire [31:0] _222_;
  wire [31:0] _223_;
  wire [31:0] _224_;
  wire [31:0] _225_;
  wire [31:0] _226_;
  wire [31:0] _227_;
  wire [31:0] _228_;
  wire [4:0] _229_;
  wire [31:0] ALU_K2_EXE;
  wire [31:0] ALU_K_EXE;
  wire [31:0] ALU_op2_EXE;
  wire [31:0] ALUout_EXE;
  reg [31:0] A_EXE;
  wire [4:0] Aad_EXE;
  wire [31:0] Aop;
  wire [4:0] Bad_DEC;
  wire [4:0] Bad_EXE;
  wire [4:0] Bad_IF;
  wire [4:0] Bad_MEM;
  wire [4:0] Bad_WB;
  wire [4:0] Bad_bus;
  wire [31:0] Bop;
  reg [31:0] C_EXE;
  reg [31:0] C_MEM;
  reg [31:0] C_WB;
  wire [31:0] IP;
  reg [31:0] IP_IF;
  reg [31:0] IR_DEC = 32'd0;
  reg [31:0] IR_EXE = 32'd0;
  reg [31:0] IR_IF = 32'd0;
  reg [31:0] IR_MEM = 32'd0;
  reg [31:0] IR_WB = 32'd0;
  wire [31:0] K;
  reg [31:0] MAR_EXE;
  reg [31:0] MDRr_MEM;
  reg [31:0] MDRr_WB;
  reg [31:0] MDRw_EXE;
  reg [31:0] MDRw_MEM;
  wire [4:0] RD_DEC;
  wire [4:0] RD_EXE;
  wire [4:0] RD_IF;
  wire [4:0] RD_MEM;
  wire [4:0] RD_WB;
  wire [4:0] RD_bus;
  wire RIadd_EXE;
  wire RIadd_WB;
  wire RIadd_bus;
  wire [4:0] RImod_DEC;
  wire [4:0] RImod_EXE;
  wire [4:0] RImod_IF;
  wire [4:0] RImod_MEM;
  wire [4:0] RImod_WB;
  wire [4:0] RImod_bus;
  wire RIsub_EXE;
  wire RIsub_WB;
  wire RIsub_bus;
  wire [4:0] RS_DEC;
  wire [4:0] RS_EXE;
  wire [4:0] RS_IF;
  wire [4:0] RS_MEM;
  wire [4:0] RS_WB;
  wire [4:0] RS_bus;
  reg [31:0] _R_0_ ;
  reg [31:0] _R_1_ ;
  reg [31:0] _R_2_ ;
  reg [31:0] _R_3_ ;
  reg [31:0] _R_4_ ;
  reg [31:0] _R_5_ ;
  reg [31:0] _R_6_ ;
  reg [31:0] _R_7_ ;
  wire ZF_DEC;
  reg ZF_EXE = 1'h0;
  wire ZF_IF;
  reg ZF_WB = 1'h0;
  wire add_EXE;
  wire add_WB;
  wire add_bus;
  wire aluop_EXE;
  wire aluop_WB;
  wire aluop_bus;
  wire aluop_k_EXE;
  wire aluop_k_WB;
  wire aluop_k_bus;
  wire btaken_WB;
  wire [31:0] bubble;
  output [31:0] bus_A_data;
  wire [31:0] bus_A_data;
  output [31:0] bus_A_ins;
  wire [31:0] bus_A_ins;
  output bus_RE_data;
  wire bus_RE_data;
  output bus_RE_ins;
  wire bus_RE_ins;
  output bus_WE;
  wire bus_WE;
  input [31:0] bus_in_data;
  wire [31:0] bus_in_data;
  input [31:0] bus_in_ins;
  wire [31:0] bus_in_ins;
  output [31:0] bus_out;
  wire [31:0] bus_out;
  input clk;
  wire clk;
  output [7:0] current_opcode;
  wire [7:0] current_opcode;
  wire [31:0] displacement_DEC;
  wire [31:0] displacement_EXE;
  wire [31:0] displacement_IF;
  wire [31:0] displacement_MEM;
  wire [31:0] displacement_WB;
  wire [31:0] displacement_bus;
  wire [31:0] distance_DEC;
  wire [31:0] distance_EXE;
  wire [31:0] distance_IF;
  wire [31:0] distance_MEM;
  wire [31:0] distance_WB;
  wire [31:0] distance_bus;
  reg [5:1] full;
  wire halt_bus;
  wire [2:0] i;
  wire jnez_EXE;
  wire jnez_WB;
  wire jnez_bus;
  wire [1:0] length_bus;
  wire load_EXE;
  wire load_MEM;
  wire load_WB;
  wire load_bus;
  wire memory_EXE;
  wire memory_bus;
  wire [1:0] mod_DEC;
  wire [1:0] mod_EXE;
  wire [1:0] mod_IF;
  wire [1:0] mod_MEM;
  wire [1:0] mod_WB;
  wire [1:0] mod_bus;
  wire move_EXE;
  wire move_WB;
  wire move_bus;
  wire [7:0] opcode_DEC;
  wire [7:0] opcode_EXE;
  wire [7:0] opcode_IF;
  wire [7:0] opcode_MEM;
  wire [7:0] opcode_WB;
  wire [7:0] opcode_bus;
  input rst;
  wire rst;
  wire store_EXE;
  wire store_MEM;
  wire store_bus;
  wire sub_EXE;
  wire sub_WB;
  wire sub_bus;
  wire [4:0] ue;
  wire valid;
  assign _039_ = IP_IF + { IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15:8] };
  assign _040_ = IP_IF + length_bus;
  assign _041_ = IP_IF + length_bus;
  assign _042_ = _020_ + { IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15:8] };
  assign _043_ = A_EXE + ALU_op2_EXE;
  assign ALUout_EXE = _043_ + sub_EXE;
  assign _044_ = A_EXE + ALU_K2_EXE;
  assign ALU_K_EXE = _044_ + RIsub_EXE;
  assign _045_ = rst ? A_EXE : _000_;
  assign _046_ = rst ? MAR_EXE : _008_;
  assign _047_ = rst ? MDRw_EXE : _010_;
  assign _048_ = rst ? C_EXE : _001_;
  assign halt_bus = bus_in_ins[7:0] == 8'hf4;
  assign _049_ = IR_EXE[15:14] == 2'h1;
  assign _050_ = IR_EXE[7:0] == 8'h8b;
  assign _051_ = IR_EXE[7:0] == 8'h89;
  assign _052_ = IR_EXE[15:14] == 2'h3;
  assign _053_ = IR_EXE[7:0] == 8'h89;
  assign _054_ = IR_EXE[15:14] == 2'h1;
  assign add_EXE = IR_EXE[7:0] == 8'h01;
  assign sub_EXE = IR_EXE[7:0] == 8'h29;
  assign jnez_EXE = IR_EXE[7:0] == 8'h75;
  assign _055_ = IR_EXE[7:0] == 8'h83;
  assign _056_ = IR_EXE[15:11] == 5'h18;
  assign _057_ = IR_EXE[7:0] == 8'h83;
  assign _058_ = IR_EXE[15:11] == 5'h1d;
  assign _059_ = IR_MEM[7:0] == 8'h8b;
  assign _060_ = IR_MEM[15:14] == 2'h1;
  assign _061_ = IR_MEM[7:0] == 8'h89;
  assign _062_ = IR_MEM[15:14] == 2'h1;
  assign _063_ = IR_WB[7:0] == 8'h8b;
  assign _064_ = IR_WB[15:14] == 2'h1;
  assign _065_ = IR_WB[7:0] == 8'h89;
  assign _066_ = IR_WB[15:14] == 2'h3;
  assign add_WB = IR_WB[7:0] == 8'h01;
  assign sub_WB = IR_WB[7:0] == 8'h29;
  assign jnez_WB = IR_WB[7:0] == 8'h75;
  assign _067_ = IR_WB[7:0] == 8'h83;
  assign _068_ = IR_WB[15:11] == 5'h18;
  assign _069_ = IR_WB[7:0] == 8'h83;
  assign _070_ = IR_WB[15:11] == 5'h1d;
  assign _071_ = ! ALUout_EXE;
  assign _072_ = ! ALU_K_EXE;
  assign _073_ = bus_in_ins[7:0] == 8'h8b;
  assign _074_ = bus_in_ins[15:14] == 2'h1;
  assign _075_ = bus_in_ins[7:0] == 8'h89;
  assign _076_ = bus_in_ins[15:14] == 2'h3;
  assign _077_ = bus_in_ins[7:0] == 8'h89;
  assign _078_ = bus_in_ins[15:14] == 2'h1;
  assign add_bus = bus_in_ins[7:0] == 8'h01;
  assign sub_bus = bus_in_ins[7:0] == 8'h29;
  assign jnez_bus = bus_in_ins[7:0] == 8'h75;
  assign _079_ = bus_in_ins[7:0] == 8'h83;
  assign _080_ = bus_in_ins[15:11] == 5'h18;
  assign _081_ = bus_in_ins[7:0] == 8'h83;
  assign _082_ = bus_in_ins[15:11] == 5'h1d;
  assign load_EXE = _050_ && _049_;
  assign move_EXE = _051_ && _052_;
  assign store_EXE = _053_ && _054_;
  assign RIadd_EXE = _055_ && _056_;
  assign RIsub_EXE = _057_ && _058_;
  assign load_MEM = _059_ && _060_;
  assign bus_WE = _061_ && _062_;
  assign load_WB = _063_ && _064_;
  assign move_WB = _065_ && _066_;
  assign RIadd_WB = _067_ && _068_;
  assign RIsub_WB = _069_ && _070_;
  assign btaken_WB = jnez_WB && _084_;
  assign _083_ = full[3] && load_MEM;
  assign bus_RE_data = full[3] && load_MEM;
  assign load_bus = _073_ && _074_;
  assign move_bus = _075_ && _076_;
  assign store_bus = _077_ && _078_;
  assign RIadd_bus = _079_ && _080_;
  assign RIsub_bus = _081_ && _082_;
  assign _084_ = ! ZF_WB;
  assign memory_bus = load_bus || store_bus;
  assign aluop_bus = add_bus || sub_bus;
  assign aluop_k_bus = RIadd_bus || RIsub_bus;
  assign _085_ = aluop_bus || move_bus;
  assign _086_ = _085_ || jnez_bus;
  assign _087_ = _086_ || aluop_k_bus;
  assign _088_ = load_EXE || move_EXE;
  assign _089_ = _088_ || store_EXE;
  assign _090_ = _089_ || add_EXE;
  assign _091_ = _090_ || sub_EXE;
  assign _092_ = _091_ || jnez_EXE;
  assign _093_ = _092_ || RIadd_EXE;
  assign _094_ = _093_ || RIsub_EXE;
  assign _095_ = load_EXE || store_EXE;
  assign _096_ = add_EXE || sub_EXE;
  assign _097_ = RIadd_EXE || RIsub_EXE;
  assign aluop_WB = add_WB || sub_WB;
  assign aluop_k_WB = RIadd_WB || RIsub_WB;
  assign _098_ = aluop_EXE || aluop_k_EXE;
  assign _099_ = _071_ || _072_;
  assign _100_ = aluop_WB || move_WB;
  assign _101_ = _100_ || load_WB;
  assign _102_ = _101_ || aluop_k_WB;
  assign _103_ = ~ Bop;
  always @(posedge clk, posedge rst)
    if (rst) MDRr_WB <= 32'd0;
    else MDRr_WB <= MDRr_MEM;
  always @(posedge clk, posedge rst)
    if (rst) _R_0_  <= 32'd0;
    else _R_0_  <= _011_;
  always @(posedge clk, posedge rst)
    if (rst) _R_1_  <= 32'd0;
    else _R_1_  <= _012_;
  always @(posedge clk, posedge rst)
    if (rst) _R_2_  <= 32'd0;
    else _R_2_  <= _013_;
  always @(posedge clk, posedge rst)
    if (rst) _R_3_  <= 32'd0;
    else _R_3_  <= _014_;
  always @(posedge clk, posedge rst)
    if (rst) _R_4_  <= 32'd0;
    else _R_4_  <= _015_;
  always @(posedge clk, posedge rst)
    if (rst) _R_5_  <= 32'd0;
    else _R_5_  <= _016_;
  always @(posedge clk, posedge rst)
    if (rst) _R_6_  <= 32'd0;
    else _R_6_  <= _017_;
  always @(posedge clk, posedge rst)
    if (rst) _R_7_  <= 32'd0;
    else _R_7_  <= _018_;
  always @(posedge clk)
    ZF_WB <= ZF_EXE;
  always @(posedge clk)
    MDRw_MEM <= MDRw_EXE;
  always @(posedge clk)
    C_MEM <= C_EXE;
  always @(posedge clk)
    C_WB <= C_MEM;
  always @(posedge clk)
    MDRr_MEM <= _009_;
  always @(posedge clk)
    A_EXE <= _045_;
  always @(posedge clk, posedge rst)
    if (rst) ZF_EXE <= 1'h0;
    else ZF_EXE <= _019_;
  always @(posedge clk)
    MAR_EXE <= _046_;
  always @(posedge clk)
    MDRw_EXE <= _047_;
  always @(posedge clk)
    C_EXE <= _048_;
  always @(posedge clk, posedge rst)
    if (rst) IP_IF <= 32'd0;
    else IP_IF <= _002_;
  always @(posedge clk)
    IR_IF <= _005_;
  always @(posedge clk)
    IR_DEC <= _003_;
  always @(posedge clk)
    IR_EXE <= _004_;
  always @(posedge clk)
    IR_MEM <= _006_;
  always @(posedge clk)
    IR_WB <= _007_;
  always @(posedge clk, posedge rst)
    if (rst) full <= 5'h10;
    else full <= 5'h1f;
  function [31:0] _356_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _356_ = b[31:0];
      8'b??????1?:
        _356_ = b[63:32];
      8'b?????1??:
        _356_ = b[95:64];
      8'b????1???:
        _356_ = b[127:96];
      8'b???1????:
        _356_ = b[159:128];
      8'b??1?????:
        _356_ = b[191:160];
      8'b?1??????:
        _356_ = b[223:192];
      8'b1???????:
        _356_ = b[255:224];
      default:
        _356_ = a;
    endcase
  endfunction
  assign Bop = _356_(32'hxxxxxxxx, { _R_0_ , _R_1_ , _R_2_ , _R_3_ , _R_4_ , _R_5_ , _R_6_ , _R_7_  }, { _111_, _110_, _109_, _108_, _107_, _106_, _105_, _104_ });
  assign _104_ = Bad_EXE[2:0] == 3'h7;
  assign _105_ = Bad_EXE[2:0] == 3'h6;
  assign _106_ = Bad_EXE[2:0] == 3'h5;
  assign _107_ = Bad_EXE[2:0] == 3'h4;
  assign _108_ = Bad_EXE[2:0] == 3'h3;
  assign _109_ = Bad_EXE[2:0] == 3'h2;
  assign _110_ = Bad_EXE[2:0] == 3'h1;
  assign _111_ = ! Bad_EXE[2:0];
  function [31:0] _365_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _365_ = b[31:0];
      8'b??????1?:
        _365_ = b[63:32];
      8'b?????1??:
        _365_ = b[95:64];
      8'b????1???:
        _365_ = b[127:96];
      8'b???1????:
        _365_ = b[159:128];
      8'b??1?????:
        _365_ = b[191:160];
      8'b?1??????:
        _365_ = b[223:192];
      8'b1???????:
        _365_ = b[255:224];
      default:
        _365_ = a;
    endcase
  endfunction
  assign Aop = _365_(32'hxxxxxxxx, { _R_0_ , _R_1_ , _R_2_ , _R_3_ , _R_4_ , _R_5_ , _R_6_ , _R_7_  }, { _119_, _118_, _117_, _116_, _115_, _114_, _113_, _112_ });
  assign _112_ = Aad_EXE[2:0] == 3'h7;
  assign _113_ = Aad_EXE[2:0] == 3'h6;
  assign _114_ = Aad_EXE[2:0] == 3'h5;
  assign _115_ = Aad_EXE[2:0] == 3'h4;
  assign _116_ = Aad_EXE[2:0] == 3'h3;
  assign _117_ = Aad_EXE[2:0] == 3'h2;
  assign _118_ = Aad_EXE[2:0] == 3'h1;
  assign _119_ = ! Aad_EXE[2:0];
  function [31:0] _374_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _374_ = b[31:0];
      8'b??????1?:
        _374_ = b[63:32];
      8'b?????1??:
        _374_ = b[95:64];
      8'b????1???:
        _374_ = b[127:96];
      8'b???1????:
        _374_ = b[159:128];
      8'b??1?????:
        _374_ = b[191:160];
      8'b?1??????:
        _374_ = b[223:192];
      8'b1???????:
        _374_ = b[255:224];
      default:
        _374_ = a;
    endcase
  endfunction
  assign _120_ = _374_(32'hxxxxxxxx, { _228_, _R_0_ , _R_0_ , _R_0_ , _R_0_ , _R_0_ , _R_0_ , _R_0_  }, { _128_, _127_, _126_, _125_, _124_, _123_, _122_, _121_ });
  assign _121_ = _229_[2:0] == 3'h7;
  assign _122_ = _229_[2:0] == 3'h6;
  assign _123_ = _229_[2:0] == 3'h5;
  assign _124_ = _229_[2:0] == 3'h4;
  assign _125_ = _229_[2:0] == 3'h3;
  assign _126_ = _229_[2:0] == 3'h2;
  assign _127_ = _229_[2:0] == 3'h1;
  assign _128_ = ! _229_[2:0];
  assign _129_ = _102_ ? _120_ : 32'hxxxxxxxx;
  assign _031_ = full[4] ? _129_ : 32'hxxxxxxxx;
  function [31:0] _385_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _385_ = b[31:0];
      8'b??????1?:
        _385_ = b[63:32];
      8'b?????1??:
        _385_ = b[95:64];
      8'b????1???:
        _385_ = b[127:96];
      8'b???1????:
        _385_ = b[159:128];
      8'b??1?????:
        _385_ = b[191:160];
      8'b?1??????:
        _385_ = b[223:192];
      8'b1???????:
        _385_ = b[255:224];
      default:
        _385_ = a;
    endcase
  endfunction
  assign _130_ = _385_(32'hxxxxxxxx, { _R_7_ , _R_7_ , _R_7_ , _R_7_ , _R_7_ , _R_7_ , _R_7_ , _228_ }, { _138_, _137_, _136_, _135_, _134_, _133_, _132_, _131_ });
  assign _131_ = _229_[2:0] == 3'h7;
  assign _132_ = _229_[2:0] == 3'h6;
  assign _133_ = _229_[2:0] == 3'h5;
  assign _134_ = _229_[2:0] == 3'h4;
  assign _135_ = _229_[2:0] == 3'h3;
  assign _136_ = _229_[2:0] == 3'h2;
  assign _137_ = _229_[2:0] == 3'h1;
  assign _138_ = ! _229_[2:0];
  assign _139_ = _102_ ? _130_ : 32'hxxxxxxxx;
  assign _038_ = full[4] ? _139_ : 32'hxxxxxxxx;
  function [31:0] _396_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _396_ = b[31:0];
      8'b??????1?:
        _396_ = b[63:32];
      8'b?????1??:
        _396_ = b[95:64];
      8'b????1???:
        _396_ = b[127:96];
      8'b???1????:
        _396_ = b[159:128];
      8'b??1?????:
        _396_ = b[191:160];
      8'b?1??????:
        _396_ = b[223:192];
      8'b1???????:
        _396_ = b[255:224];
      default:
        _396_ = a;
    endcase
  endfunction
  assign _140_ = _396_(32'hxxxxxxxx, { _R_6_ , _R_6_ , _R_6_ , _R_6_ , _R_6_ , _R_6_ , _228_, _R_6_  }, { _148_, _147_, _146_, _145_, _144_, _143_, _142_, _141_ });
  assign _141_ = _229_[2:0] == 3'h7;
  assign _142_ = _229_[2:0] == 3'h6;
  assign _143_ = _229_[2:0] == 3'h5;
  assign _144_ = _229_[2:0] == 3'h4;
  assign _145_ = _229_[2:0] == 3'h3;
  assign _146_ = _229_[2:0] == 3'h2;
  assign _147_ = _229_[2:0] == 3'h1;
  assign _148_ = ! _229_[2:0];
  assign _149_ = _102_ ? _140_ : 32'hxxxxxxxx;
  assign _037_ = full[4] ? _149_ : 32'hxxxxxxxx;
  function [31:0] _407_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _407_ = b[31:0];
      8'b??????1?:
        _407_ = b[63:32];
      8'b?????1??:
        _407_ = b[95:64];
      8'b????1???:
        _407_ = b[127:96];
      8'b???1????:
        _407_ = b[159:128];
      8'b??1?????:
        _407_ = b[191:160];
      8'b?1??????:
        _407_ = b[223:192];
      8'b1???????:
        _407_ = b[255:224];
      default:
        _407_ = a;
    endcase
  endfunction
  assign _150_ = _407_(32'hxxxxxxxx, { _R_5_ , _R_5_ , _R_5_ , _R_5_ , _R_5_ , _228_, _R_5_ , _R_5_  }, { _158_, _157_, _156_, _155_, _154_, _153_, _152_, _151_ });
  assign _151_ = _229_[2:0] == 3'h7;
  assign _152_ = _229_[2:0] == 3'h6;
  assign _153_ = _229_[2:0] == 3'h5;
  assign _154_ = _229_[2:0] == 3'h4;
  assign _155_ = _229_[2:0] == 3'h3;
  assign _156_ = _229_[2:0] == 3'h2;
  assign _157_ = _229_[2:0] == 3'h1;
  assign _158_ = ! _229_[2:0];
  assign _159_ = _102_ ? _150_ : 32'hxxxxxxxx;
  assign _036_ = full[4] ? _159_ : 32'hxxxxxxxx;
  function [31:0] _418_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _418_ = b[31:0];
      8'b??????1?:
        _418_ = b[63:32];
      8'b?????1??:
        _418_ = b[95:64];
      8'b????1???:
        _418_ = b[127:96];
      8'b???1????:
        _418_ = b[159:128];
      8'b??1?????:
        _418_ = b[191:160];
      8'b?1??????:
        _418_ = b[223:192];
      8'b1???????:
        _418_ = b[255:224];
      default:
        _418_ = a;
    endcase
  endfunction
  assign _160_ = _418_(32'hxxxxxxxx, { _R_4_ , _R_4_ , _R_4_ , _R_4_ , _228_, _R_4_ , _R_4_ , _R_4_  }, { _168_, _167_, _166_, _165_, _164_, _163_, _162_, _161_ });
  assign _161_ = _229_[2:0] == 3'h7;
  assign _162_ = _229_[2:0] == 3'h6;
  assign _163_ = _229_[2:0] == 3'h5;
  assign _164_ = _229_[2:0] == 3'h4;
  assign _165_ = _229_[2:0] == 3'h3;
  assign _166_ = _229_[2:0] == 3'h2;
  assign _167_ = _229_[2:0] == 3'h1;
  assign _168_ = ! _229_[2:0];
  assign _169_ = _102_ ? _160_ : 32'hxxxxxxxx;
  assign _035_ = full[4] ? _169_ : 32'hxxxxxxxx;
  function [31:0] _429_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _429_ = b[31:0];
      8'b??????1?:
        _429_ = b[63:32];
      8'b?????1??:
        _429_ = b[95:64];
      8'b????1???:
        _429_ = b[127:96];
      8'b???1????:
        _429_ = b[159:128];
      8'b??1?????:
        _429_ = b[191:160];
      8'b?1??????:
        _429_ = b[223:192];
      8'b1???????:
        _429_ = b[255:224];
      default:
        _429_ = a;
    endcase
  endfunction
  assign _170_ = _429_(32'hxxxxxxxx, { _R_3_ , _R_3_ , _R_3_ , _228_, _R_3_ , _R_3_ , _R_3_ , _R_3_  }, { _178_, _177_, _176_, _175_, _174_, _173_, _172_, _171_ });
  assign _171_ = _229_[2:0] == 3'h7;
  assign _172_ = _229_[2:0] == 3'h6;
  assign _173_ = _229_[2:0] == 3'h5;
  assign _174_ = _229_[2:0] == 3'h4;
  assign _175_ = _229_[2:0] == 3'h3;
  assign _176_ = _229_[2:0] == 3'h2;
  assign _177_ = _229_[2:0] == 3'h1;
  assign _178_ = ! _229_[2:0];
  assign _179_ = _102_ ? _170_ : 32'hxxxxxxxx;
  assign _034_ = full[4] ? _179_ : 32'hxxxxxxxx;
  function [31:0] _440_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _440_ = b[31:0];
      8'b??????1?:
        _440_ = b[63:32];
      8'b?????1??:
        _440_ = b[95:64];
      8'b????1???:
        _440_ = b[127:96];
      8'b???1????:
        _440_ = b[159:128];
      8'b??1?????:
        _440_ = b[191:160];
      8'b?1??????:
        _440_ = b[223:192];
      8'b1???????:
        _440_ = b[255:224];
      default:
        _440_ = a;
    endcase
  endfunction
  assign _180_ = _440_(32'hxxxxxxxx, { _R_2_ , _R_2_ , _228_, _R_2_ , _R_2_ , _R_2_ , _R_2_ , _R_2_  }, { _188_, _187_, _186_, _185_, _184_, _183_, _182_, _181_ });
  assign _181_ = _229_[2:0] == 3'h7;
  assign _182_ = _229_[2:0] == 3'h6;
  assign _183_ = _229_[2:0] == 3'h5;
  assign _184_ = _229_[2:0] == 3'h4;
  assign _185_ = _229_[2:0] == 3'h3;
  assign _186_ = _229_[2:0] == 3'h2;
  assign _187_ = _229_[2:0] == 3'h1;
  assign _188_ = ! _229_[2:0];
  assign _189_ = _102_ ? _180_ : 32'hxxxxxxxx;
  assign _033_ = full[4] ? _189_ : 32'hxxxxxxxx;
  function [31:0] _451_;
    input [31:0] a;
    input [255:0] b;
    input [7:0] s;
    casez (s) // synopsys parallel_case
      8'b???????1:
        _451_ = b[31:0];
      8'b??????1?:
        _451_ = b[63:32];
      8'b?????1??:
        _451_ = b[95:64];
      8'b????1???:
        _451_ = b[127:96];
      8'b???1????:
        _451_ = b[159:128];
      8'b??1?????:
        _451_ = b[191:160];
      8'b?1??????:
        _451_ = b[223:192];
      8'b1???????:
        _451_ = b[255:224];
      default:
        _451_ = a;
    endcase
  endfunction
  assign _190_ = _451_(32'hxxxxxxxx, { _R_1_ , _228_, _R_1_ , _R_1_ , _R_1_ , _R_1_ , _R_1_ , _R_1_  }, { _198_, _197_, _196_, _195_, _194_, _193_, _192_, _191_ });
  assign _191_ = _229_[2:0] == 3'h7;
  assign _192_ = _229_[2:0] == 3'h6;
  assign _193_ = _229_[2:0] == 3'h5;
  assign _194_ = _229_[2:0] == 3'h4;
  assign _195_ = _229_[2:0] == 3'h3;
  assign _196_ = _229_[2:0] == 3'h2;
  assign _197_ = _229_[2:0] == 3'h1;
  assign _198_ = ! _229_[2:0];
  assign _199_ = _102_ ? _190_ : 32'hxxxxxxxx;
  assign _032_ = full[4] ? _199_ : 32'hxxxxxxxx;
  assign _200_ = _102_ ? _038_ : _R_7_ ;
  assign _028_ = full[4] ? _200_ : 32'hxxxxxxxx;
  assign _201_ = _102_ ? _037_ : _R_6_ ;
  assign _027_ = full[4] ? _201_ : 32'hxxxxxxxx;
  assign _202_ = _102_ ? _036_ : _R_5_ ;
  assign _026_ = full[4] ? _202_ : 32'hxxxxxxxx;
  assign _203_ = _102_ ? _035_ : _R_4_ ;
  assign _025_ = full[4] ? _203_ : 32'hxxxxxxxx;
  assign _204_ = _102_ ? _034_ : _R_3_ ;
  assign _024_ = full[4] ? _204_ : 32'hxxxxxxxx;
  assign _205_ = _102_ ? _033_ : _R_2_ ;
  assign _023_ = full[4] ? _205_ : 32'hxxxxxxxx;
  assign _206_ = _102_ ? _032_ : _R_1_ ;
  assign _022_ = full[4] ? _206_ : 32'hxxxxxxxx;
  assign _207_ = _102_ ? _031_ : _R_0_ ;
  assign _021_ = full[4] ? _207_ : 32'hxxxxxxxx;
  assign _018_ = full[4] ? _028_ : _R_7_ ;
  assign _017_ = full[4] ? _027_ : _R_6_ ;
  assign _016_ = full[4] ? _026_ : _R_5_ ;
  assign _015_ = full[4] ? _025_ : _R_4_ ;
  assign _014_ = full[4] ? _024_ : _R_3_ ;
  assign _013_ = full[4] ? _023_ : _R_2_ ;
  assign _012_ = full[4] ? _022_ : _R_1_ ;
  assign _011_ = full[4] ? _021_ : _R_0_ ;
  assign _009_ = _083_ ? bus_in_data : MDRr_MEM;
  assign _208_ = _098_ ? _099_ : ZF_EXE;
  assign _029_ = full[2] ? _208_ : 1'hx;
  assign _019_ = full[2] ? _029_ : ZF_EXE;
  assign _010_ = full[2] ? Bop : MDRw_EXE;
  assign _001_ = full[2] ? _227_ : C_EXE;
  assign _008_ = full[2] ? _225_ : MAR_EXE;
  assign _000_ = full[2] ? Aop : A_EXE;
  assign _209_ = btaken_WB ? _042_ : _020_;
  assign _030_ = full[1] ? _209_ : 32'hxxxxxxxx;
  assign _210_ = halt_bus ? IP_IF : _041_;
  assign _020_ = full[1] ? _210_ : 32'hxxxxxxxx;
  assign _002_ = full[1] ? _030_ : IP_IF;
  assign _005_ = rst ? IR_IF : bus_in_ins;
  assign _003_ = rst ? IR_DEC : IR_IF;
  assign _004_ = rst ? IR_EXE : IR_DEC;
  assign _006_ = rst ? IR_MEM : IR_EXE;
  assign _007_ = rst ? IR_WB : IR_MEM;
  assign { _211_[31:2], length_bus } = memory_bus ? 32'd3 : _212_;
  assign _212_ = _087_ ? 32'd2 : 32'd1;
  assign { _213_[31:1], valid } = _094_ ? 32'd1 : 32'd0;
  assign { _214_[31:1], memory_EXE } = valid ? _095_ : 32'd0;
  assign { _215_[31:1], aluop_EXE } = valid ? _096_ : 32'd0;
  assign { _216_[31:1], aluop_k_EXE } = valid ? _097_ : 32'd0;
  assign { _217_[31:5], RD_EXE } = valid ? { 29'h00000000, IR_EXE[10:8] } : 32'd0;
  assign { _218_[31:5], RS_EXE } = valid ? { 29'h00000000, IR_EXE[13:11] } : 32'd0;
  assign _219_ = memory_EXE ? 32'd6 : { 27'h0000000, RD_EXE };
  assign { _220_[31:5], Aad_EXE } = valid ? _219_ : 32'd0;
  assign { _221_[31:5], Bad_EXE } = valid ? { 27'h0000000, RS_EXE } : 32'd0;
  assign _222_ = halt_bus ? IP_IF : _040_;
  assign _223_ = btaken_WB ? _039_ : _222_;
  assign bus_A_ins = rst ? 32'd0 : _223_;
  assign _224_ = sub_EXE ? _103_ : Bop;
  assign ALU_op2_EXE = memory_EXE ? { IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23:16] } : _224_;
  assign ALU_K2_EXE = RIsub_EXE ? 32'd4294967285 : 32'd10;
  assign _225_ = aluop_k_EXE ? ALU_K_EXE : ALUout_EXE;
  assign _226_ = aluop_k_EXE ? ALU_K_EXE : ALUout_EXE;
  assign _227_ = move_EXE ? Bop : _226_;
  assign bus_A_data = full[3] ? MAR_EXE : 32'd0;
  assign _228_ = load_WB ? MDRr_WB : C_WB;
  assign _229_ = load_WB ? { 2'h0, IR_WB[13:11] } : { 2'h0, IR_WB[10:8] };
  assign _211_[1:0] = length_bus;
  assign _213_[0] = valid;
  assign _214_[0] = memory_EXE;
  assign _215_[0] = aluop_EXE;
  assign _216_[0] = aluop_k_EXE;
  assign _217_[4:0] = RD_EXE;
  assign _218_[4:0] = RS_EXE;
  assign _220_[4:0] = Aad_EXE;
  assign _221_[4:0] = Bad_EXE;
  assign Bad_DEC = { 2'h0, IR_DEC[13:11] };
  assign Bad_IF = { 2'h0, IR_IF[13:11] };
  assign Bad_MEM = { 2'h0, IR_MEM[13:11] };
  assign Bad_WB = { 2'h0, IR_WB[13:11] };
  assign Bad_bus = { 2'h0, bus_in_ins[13:11] };
  assign IP = bus_A_ins;
  assign K = 32'd10;
  assign RD_DEC = { 2'h0, IR_DEC[10:8] };
  assign RD_IF = { 2'h0, IR_IF[10:8] };
  assign RD_MEM = { 2'h0, IR_MEM[10:8] };
  assign RD_WB = { 2'h0, IR_WB[10:8] };
  assign RD_bus = { 2'h0, bus_in_ins[10:8] };
  assign RImod_DEC = IR_DEC[15:11];
  assign RImod_EXE = IR_EXE[15:11];
  assign RImod_IF = IR_IF[15:11];
  assign RImod_MEM = IR_MEM[15:11];
  assign RImod_WB = IR_WB[15:11];
  assign RImod_bus = bus_in_ins[15:11];
  assign RS_DEC = { 2'h0, IR_DEC[13:11] };
  assign RS_IF = { 2'h0, IR_IF[13:11] };
  assign RS_MEM = { 2'h0, IR_MEM[13:11] };
  assign RS_WB = { 2'h0, IR_WB[13:11] };
  assign RS_bus = { 2'h0, bus_in_ins[13:11] };
  assign ZF_DEC = 1'h0;
  assign ZF_IF = 1'h0;
  assign bubble = 32'd0;
  assign bus_RE_ins = 1'h1;
  assign bus_out = MDRw_MEM;
  assign current_opcode = IR_WB[7:0];
  assign displacement_DEC = { IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23], IR_DEC[23:16] };
  assign displacement_EXE = { IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23], IR_EXE[23:16] };
  assign displacement_IF = { IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23], IR_IF[23:16] };
  assign displacement_MEM = { IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23], IR_MEM[23:16] };
  assign displacement_WB = { IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23], IR_WB[23:16] };
  assign displacement_bus = { bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23], bus_in_ins[23:16] };
  assign distance_DEC = { IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15], IR_DEC[15:8] };
  assign distance_EXE = { IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15], IR_EXE[15:8] };
  assign distance_IF = { IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15], IR_IF[15:8] };
  assign distance_MEM = { IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15], IR_MEM[15:8] };
  assign distance_WB = { IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15], IR_WB[15:8] };
  assign distance_bus = { bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15], bus_in_ins[15:8] };
  assign i = 3'h0;
  assign mod_DEC = IR_DEC[15:14];
  assign mod_EXE = IR_EXE[15:14];
  assign mod_IF = IR_IF[15:14];
  assign mod_MEM = IR_MEM[15:14];
  assign mod_WB = IR_WB[15:14];
  assign mod_bus = bus_in_ins[15:14];
  assign opcode_DEC = IR_DEC[7:0];
  assign opcode_EXE = IR_EXE[7:0];
  assign opcode_IF = IR_IF[7:0];
  assign opcode_MEM = IR_MEM[7:0];
  assign opcode_WB = IR_WB[7:0];
  assign opcode_bus = bus_in_ins[7:0];
  assign store_MEM = bus_WE;
  assign ue = { full[4:1], full[5] };
endmodule
