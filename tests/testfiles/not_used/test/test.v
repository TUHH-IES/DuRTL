module LoopConvLdWeight(
input clock,
input reset,
output io_req_ready,
input io_req_valid,
input [15:0] io_req_bits_outer_bounds_in_channels,
input [15:0] io_req_bits_outer_bounds_out_channels,
input [15:0] io_req_bits_outer_bounds_kernel_dim,
input [15:0] io_req_bits_inner_bounds_krows,
input [15:0] io_req_bits_inner_bounds_kcols,
input [15:0] io_req_bits_inner_bounds_kchs,
input [15:0] io_req_bits_derived_params_ochs,
input [15:0] io_req_bits_derived_params_out_channels_per_bank,
input [15:0] io_req_bits_derived_params_weight_spad_stride,
input [14:0] io_req_bits_addr_end,
input [39:0] io_req_bits_dram_addr,
input io_req_bits_loop_id,
input io_cmd_ready,
output io_cmd_valid,
output [6:0] io_cmd_bits_inst_funct,
output [63:0] io_cmd_bits_rs1,
output [63:0] io_cmd_bits_rs2,
output io_idle,
input io_rob_overloaded,
input io_wait_for_prev_loop,
output io_loop_id
);
`ifdef RANDOMIZE_REG_INIT
reg [31:0] _RAND_0;
reg [31:0] _RAND_1;
reg [31:0] _RAND_2;
reg [31:0] _RAND_3;
reg [31:0] _RAND_4;
reg [31:0] _RAND_5;
reg [31:0] _RAND_6;
reg [31:0] _RAND_7;
reg [31:0] _RAND_8;
reg [31:0] _RAND_9;
reg [31:0] _RAND_10;
reg [63:0] _RAND_11;
reg [31:0] _RAND_12;
reg [31:0] _RAND_13;
reg [31:0] _RAND_14;
reg [31:0] _RAND_15;
reg [31:0] _RAND_16;
`endif // RANDOMIZE_REG_INIT
wire command_p_clock; // @[LoopConv.scala 478:25]
wire command_p_reset; // @[LoopConv.scala 478:25]
wire command_p_io_in_ready; // @[LoopConv.scala 478:25]
wire command_p_io_in_valid; // @[LoopConv.scala 478:25]
wire [6:0] command_p_io_in_bits_cmd_inst_funct; // @[LoopConv.scala 478:25]
wire [63:0] command_p_io_in_bits_cmd_rs1; // @[LoopConv.scala 478:25]
wire [63:0] command_p_io_in_bits_cmd_rs2; // @[LoopConv.scala 478:25]
wire [67:0] command_p_io_in_bits_dram_addr; // @[LoopConv.scala 478:25]
wire [63:0] command_p_io_in_bits_spad_addr; // @[LoopConv.scala 478:25]
wire [15:0] command_p_io_in_bits_K; // @[LoopConv.scala 478:25]
wire [15:0] command_p_io_in_bits_J; // @[LoopConv.scala 478:25]
wire command_p_io_out_ready; // @[LoopConv.scala 478:25]
wire command_p_io_out_valid; // @[LoopConv.scala 478:25]
wire [6:0] command_p_io_out_bits_cmd_inst_funct; // @[LoopConv.scala 478:25]
wire [63:0] command_p_io_out_bits_cmd_rs1; // @[LoopConv.scala 478:25]
wire [63:0] command_p_io_out_bits_cmd_rs2; // @[LoopConv.scala 478:25]
wire [67:0] command_p_io_out_bits_dram_addr; // @[LoopConv.scala 478:25]
wire [63:0] command_p_io_out_bits_spad_addr; // @[LoopConv.scala 478:25]
wire [15:0] command_p_io_out_bits_K; // @[LoopConv.scala 478:25]
wire [15:0] command_p_io_out_bits_J; // @[LoopConv.scala 478:25]
wire command_p_io_busy; // @[LoopConv.scala 478:25]
reg [1:0] state; // @[LoopConv.scala 423:22]
reg [15:0] req_outer_bounds_in_channels; // @[LoopConv.scala 425:16]
reg [15:0] req_outer_bounds_out_channels; // @[LoopConv.scala 425:16]
reg [15:0] req_outer_bounds_kernel_dim; // @[LoopConv.scala 425:16]
reg [15:0] req_inner_bounds_krows; // @[LoopConv.scala 425:16]
reg [15:0] req_inner_bounds_kcols; // @[LoopConv.scala 425:16]
reg [15:0] req_inner_bounds_kchs; // @[LoopConv.scala 425:16]
reg [15:0] req_derived_params_ochs; // @[LoopConv.scala 425:16]
reg [15:0] req_derived_params_out_channels_per_bank; // @[LoopConv.scala 425:16]
reg [15:0] req_derived_params_weight_spad_stride; // @[LoopConv.scala 425:16]
reg [14:0] req_addr_end; // @[LoopConv.scala 425:16]
reg [39:0] req_dram_addr; // @[LoopConv.scala 425:16]
reg req_loop_id; // @[LoopConv.scala 425:16]
wire [15:0] max_chs_per_mvin_max_ochs_per_mvin = req_derived_params_ochs < 16'h40 ? req_derived_params_ochs : 16'h40; // @[LoopConv.scala 432:32]
wire [31:0] _B_rows_T_3 = req_derived_params_out_channels_per_bank * req_inner_bounds_kcols; // @[LoopConv.scala 438:27]
wire [47:0] _B_rows_T_4 = _B_rows_T_3 * req_inner_bounds_krows; // @[LoopConv.scala 438:35]
wire [63:0] B_rows = _B_rows_T_4 * req_inner_bounds_kchs; // @[LoopConv.scala 438:43]
wire [63:0] _GEN_63 = {{49'd0}, req_addr_end}; // @[LoopConv.scala 439:33]
wire [63:0] addr_start = _GEN_63 - B_rows; // @[LoopConv.scala 439:33]
wire [47:0] _dram_stride_T_3 = {{32'd0}, req_outer_bounds_out_channels}; // @[Mux.scala 101:16]
wire [48:0] dram_stride = _dram_stride_T_3 * 1'h1; // @[LoopConv.scala 444:6]
reg [15:0] och; // @[LoopConv.scala 447:16]
reg [15:0] krow; // @[LoopConv.scala 448:17]
reg [15:0] kcol; // @[LoopConv.scala 449:17]
reg [15:0] kch; // @[LoopConv.scala 450:16]
wire [31:0] _dram_offset_T = krow * req_outer_bounds_kernel_dim; // @[LoopConv.scala 453:35]
wire [47:0] _dram_offset_T_1 = _dram_offset_T * req_outer_bounds_in_channels; // @[LoopConv.scala 453:46]
wire [31:0] _dram_offset_T_2 = kcol * req_outer_bounds_in_channels; // @[LoopConv.scala 453:66]
wire [47:0] _GEN_64 = {{16'd0}, _dram_offset_T_2}; // @[LoopConv.scala 453:59]
wire [48:0] _dram_offset_T_3 = _dram_offset_T_1 + _GEN_64; // @[LoopConv.scala 453:59]
wire [48:0] _GEN_65 = {{33'd0}, kch}; // @[LoopConv.scala 453:79]
wire [49:0] _dram_offset_T_4 = _dram_offset_T_3 + _GEN_65; // @[LoopConv.scala 453:79]
wire [65:0] _dram_offset_T_5 = _dram_offset_T_4 * req_outer_bounds_out_channels; // @[LoopConv.scala 453:87]
wire [65:0] _GEN_66 = {{50'd0}, och}; // @[LoopConv.scala 453:102]
wire [66:0] _dram_offset_T_6 = _dram_offset_T_5 + _GEN_66; // @[LoopConv.scala 453:102]
wire [67:0] dram_offset = _dram_offset_T_6 * 1'h1; // @[LoopConv.scala 453:110]
wire [67:0] _dram_addr_T = dram_offset & 68'hffffffff; // @[LoopConv.scala 1523:17]
wire [67:0] _GEN_73 = {{28'd0}, req_dram_addr}; // @[LoopConv.scala 457:33]
wire [31:0] _spad_addr_T_6 = krow * req_inner_bounds_kcols; // @[LoopConv.scala 460:69]
wire [15:0] _spad_addr_T_15 = och / 5'h10; // @[LoopConv.scala 461:23]
wire [31:0] _spad_addr_T_16 = _spad_addr_T_15 * req_inner_bounds_krows; // @[LoopConv.scala 461:39]
wire [47:0] _spad_addr_T_17 = _spad_addr_T_16 * req_inner_bounds_kcols; // @[LoopConv.scala 461:47]
wire [63:0] _spad_addr_T_18 = _spad_addr_T_17 * req_inner_bounds_kchs; // @[LoopConv.scala 461:55]
wire [63:0] _spad_addr_T_20 = addr_start + _spad_addr_T_18; // @[LoopConv.scala 461:16]
wire [47:0] _spad_addr_T_22 = _spad_addr_T_6 * req_inner_bounds_kchs; // @[LoopConv.scala 461:77]
wire [63:0] _GEN_77 = {{16'd0}, _spad_addr_T_22}; // @[LoopConv.scala 461:62]
wire [63:0] _spad_addr_T_24 = _spad_addr_T_20 + _GEN_77; // @[LoopConv.scala 461:62]
wire [31:0] _spad_addr_T_25 = kcol * req_inner_bounds_kchs; // @[LoopConv.scala 461:91]
wire [63:0] _GEN_78 = {{32'd0}, _spad_addr_T_25}; // @[LoopConv.scala 461:84]
wire [63:0] _spad_addr_T_27 = _spad_addr_T_24 + _GEN_78; // @[LoopConv.scala 461:84]
wire [63:0] _GEN_79 = {{48'd0}, kch}; // @[LoopConv.scala 461:98]
wire [15:0] _J_T_1 = req_inner_bounds_kchs - kch; // @[LoopConv.scala 465:14]
wire [15:0] _J_T_7 = req_derived_params_ochs - och; // @[LoopConv.scala 466:14]
wire [13:0] config_cmd_rs1_stride = req_derived_params_weight_spad_stride[13:0]; // @[LoopConv.scala 485:28 488:25]
wire [63:0] config_cmd__rs1 = {34'hfe000000,config_cmd_rs1_stride,3'h0,13'h109}; // @[LoopConv.scala 493:36]
wire _io_req_ready_T = state == 2'h0; // @[LoopConv.scala 504:25]
wire _io_req_ready_T_1 = ~command_p_io_busy; // @[LoopConv.scala 504:37]
wire _command_p_io_in_bits_cmd_T = state == 2'h1; // @[LoopConv.scala 509:41]
wire [63:0] config_cmd__rs2 = {{15'd0}, dram_stride}; // @[LoopConv.scala 481:24 495:18]
wire _command_p_io_out_ready_T = ~io_rob_overloaded; // @[LoopConv.scala 515:45]
wire [13:0] mvin_cmd_rs2_local_addr_result_result_data = command_p_io_out_bits_spad_addr[13:0]; // @[LocalAddr.scala 103:37]
wire mvin_cmd_rs2_local_addr_result_result_garbage_bit = command_p_io_out_bits_spad_addr[14]; // @[LocalAddr.scala 103:37]
wire [6:0] mvin_cmd_rs2_num_cols = command_p_io_out_bits_J[6:0]; // @[LoopConv.scala 521:28 524:27]
wire [4:0] mvin_cmd_rs2_num_rows = command_p_io_out_bits_K[4:0]; // @[LoopConv.scala 521:28 523:27]
wire [63:0] _io_cmd_bits_rs2_T = {11'h0,mvin_cmd_rs2_num_rows,9'h0,mvin_cmd_rs2_num_cols,1'h0,16'h0,
mvin_cmd_rs2_local_addr_result_result_garbage_bit,mvin_cmd_rs2_local_addr_result_result_data}; // @[LoopConv.scala 526:43]
wire [67:0] _GEN_0 = command_p_io_out_bits_cmd_inst_funct == 7'h1 ? command_p_io_out_bits_dram_addr : {{4'd0},
command_p_io_out_bits_cmd_rs1}; // @[LoopConv.scala 517:15 518:61 520:21]
wire _T_1 = command_p_io_in_ready & command_p_io_in_valid; // @[Decoupled.scala 50:35]
wire [15:0] next_kch_max = req_inner_bounds_kchs - 16'h1; // @[Util.scala 39:28]
wire [16:0] _next_kch_T = kch + 16'h10; // @[Util.scala 41:15]
wire [15:0] _next_kch_T_1 = kch + 16'h10; // @[Util.scala 41:15]
wire [16:0] _GEN_80 = {{1'd0}, next_kch_max}; // @[Util.scala 43:17]
wire _next_kch_T_4 = _next_kch_T > _GEN_80; // @[Util.scala 43:17]
wire [15:0] next_kch = _next_kch_T_4 ? 16'h0 : _next_kch_T_1; // @[Mux.scala 101:16]
wire _next_kcol_T = next_kch == 16'h0; // @[LoopConv.scala 538:59]
wire [15:0] next_kcol_max = req_inner_bounds_kcols - 16'h1; // @[Util.scala 39:28]
wire [16:0] _next_kcol_T_1 = kcol + 16'h1; // @[Util.scala 41:15]
wire [15:0] _next_kcol_T_2 = kcol + 16'h1; // @[Util.scala 41:15]
wire _next_kcol_T_3 = ~_next_kcol_T; // @[Util.scala 42:8]
wire [16:0] _GEN_81 = {{1'd0}, next_kcol_max}; // @[Util.scala 43:17]
wire _next_kcol_T_5 = _next_kcol_T_1 > _GEN_81; // @[Util.scala 43:17]
wire [15:0] _next_kcol_T_6 = _next_kcol_T_5 ? 16'h0 : _next_kcol_T_2; // @[Mux.scala 101:16]
wire [15:0] next_kcol = _next_kcol_T_3 ? kcol : _next_kcol_T_6; // @[Mux.scala 101:16]
wire _next_krow_T = next_kcol == 16'h0; // @[LoopConv.scala 539:60]
wire _next_krow_T_2 = next_kcol == 16'h0 & _next_kcol_T; // @[LoopConv.scala 539:68]
wire [15:0] next_krow_max = req_inner_bounds_krows - 16'h1; // @[Util.scala 39:28]
wire [16:0] _next_krow_T_3 = krow + 16'h1; // @[Util.scala 41:15]
wire [15:0] _next_krow_T_4 = krow + 16'h1; // @[Util.scala 41:15]
wire _next_krow_T_5 = ~_next_krow_T_2; // @[Util.scala 42:8]
wire [16:0] _GEN_82 = {{1'd0}, next_krow_max}; // @[Util.scala 43:17]
wire _next_krow_T_7 = _next_krow_T_3 > _GEN_82; // @[Util.scala 43:17]
wire [15:0] _next_krow_T_8 = _next_krow_T_7 ? 16'h0 : _next_krow_T_4; // @[Mux.scala 101:16]
wire [15:0] next_krow = _next_krow_T_5 ? krow : _next_krow_T_8; // @[Mux.scala 101:16]
wire _next_och_T = next_krow == 16'h0; // @[LoopConv.scala 540:60]
wire _next_och_T_4 = next_krow == 16'h0 & _next_krow_T & _next_kcol_T; // @[LoopConv.scala 540:89]
wire [15:0] next_och_max = req_derived_params_ochs - 16'h1; // @[Util.scala 39:28]
wire [16:0] _next_och_T_5 = och + max_chs_per_mvin_max_ochs_per_mvin; // @[Util.scala 41:15]
wire [15:0] _next_och_T_6 = och + max_chs_per_mvin_max_ochs_per_mvin; // @[Util.scala 41:15]
wire _next_och_T_7 = ~_next_och_T_4; // @[Util.scala 42:8]
wire [16:0] _GEN_83 = {{1'd0}, next_och_max}; // @[Util.scala 43:17]
wire _next_och_T_9 = _next_och_T_5 > _GEN_83; // @[Util.scala 43:17]
wire [15:0] _next_och_T_10 = _next_och_T_9 ? 16'h0 : _next_och_T_6; // @[Mux.scala 101:16]
wire [15:0] next_och = _next_och_T_7 ? och : _next_och_T_10; // @[Mux.scala 101:16]
wire [1:0] _state_T_7 = next_och == 16'h0 & _next_och_T & _next_krow_T & _next_kcol_T ? 2'h0 : 2'h2; // @[LoopConv.scala 547:19]
wire _T_3 = io_req_ready & io_req_valid; // @[Decoupled.scala 50:35]
Pipeline_12 command_p ( // @[LoopConv.scala 478:25]
.clock(command_p_clock),
.reset(command_p_reset),
.io_in_ready(command_p_io_in_ready),
.io_in_valid(command_p_io_in_valid),
.io_in_bits_cmd_inst_funct(command_p_io_in_bits_cmd_inst_funct),
.io_in_bits_cmd_rs1(command_p_io_in_bits_cmd_rs1),
.io_in_bits_cmd_rs2(command_p_io_in_bits_cmd_rs2),
.io_in_bits_dram_addr(command_p_io_in_bits_dram_addr),
.io_in_bits_spad_addr(command_p_io_in_bits_spad_addr),
.io_in_bits_K(command_p_io_in_bits_K),
.io_in_bits_J(command_p_io_in_bits_J),
.io_out_ready(command_p_io_out_ready),
.io_out_valid(command_p_io_out_valid),
.io_out_bits_cmd_inst_funct(command_p_io_out_bits_cmd_inst_funct),
.io_out_bits_cmd_rs1(command_p_io_out_bits_cmd_rs1),
.io_out_bits_cmd_rs2(command_p_io_out_bits_cmd_rs2),
.io_out_bits_dram_addr(command_p_io_out_bits_dram_addr),
.io_out_bits_spad_addr(command_p_io_out_bits_spad_addr),
.io_out_bits_K(command_p_io_out_bits_K),
.io_out_bits_J(command_p_io_out_bits_J),
.io_busy(command_p_io_busy)
);
assign io_req_ready = state == 2'h0 & ~command_p_io_busy; // @[LoopConv.scala 504:34]
assign io_cmd_valid = command_p_io_out_valid & _command_p_io_out_ready_T; // @[LoopConv.scala 516:42]
assign io_cmd_bits_inst_funct = command_p_io_out_bits_cmd_inst_funct; // @[LoopConv.scala 517:15]
assign io_cmd_bits_rs1 = _GEN_0[63:0];
assign io_cmd_bits_rs2 = command_p_io_out_bits_cmd_inst_funct == 7'h1 ? _io_cmd_bits_rs2_T :
command_p_io_out_bits_cmd_rs2; // @[LoopConv.scala 517:15 518:61 526:21]
assign io_idle = _io_req_ready_T & _io_req_ready_T_1; // @[LoopConv.scala 505:29]
assign io_loop_id = req_loop_id; // @[LoopConv.scala 506:14]
assign command_p_clock = clock;
assign command_p_reset = reset;
assign command_p_io_in_valid = state != 2'h0 & ~io_wait_for_prev_loop; // @[LoopConv.scala 508:43]
assign command_p_io_in_bits_cmd_inst_funct = state == 2'h1 ? 7'h0 : 7'h1; // @[LoopConv.scala 509:34]
assign command_p_io_in_bits_cmd_rs1 = state == 2'h1 ? config_cmd__rs1 : 64'h0; // @[LoopConv.scala 509:34]
assign command_p_io_in_bits_cmd_rs2 = state == 2'h1 ? config_cmd__rs2 : 64'h0; // @[LoopConv.scala 509:34]
assign command_p_io_in_bits_dram_addr = _GEN_73 + _dram_addr_T; // @[LoopConv.scala 457:33]
assign command_p_io_in_bits_spad_addr = _spad_addr_T_27 + _GEN_79; // @[LoopConv.scala 461:98]
assign command_p_io_in_bits_K = _J_T_1 > 16'h10 ? 16'h10 : _J_T_1; // @[LoopConv.scala 469:8]
assign command_p_io_in_bits_J = _J_T_7 > max_chs_per_mvin_max_ochs_per_mvin ? max_chs_per_mvin_max_ochs_per_mvin :
_J_T_7; // @[LoopConv.scala 466:8]
assign command_p_io_out_ready = io_cmd_ready & ~io_rob_overloaded; // @[LoopConv.scala 515:42]
always @(posedge clock) begin
if (reset) begin // @[LoopConv.scala 423:22]
state <= 2'h0; // @[LoopConv.scala 423:22]
end else if (_T_3) begin // @[LoopConv.scala 553:22]
state <= 2'h1; // @[LoopConv.scala 555:11]
end else if (_T_1) begin // @[LoopConv.scala 530:30]
if (_command_p_io_in_bits_cmd_T) begin // @[LoopConv.scala 531:29]
state <= 2'h2; // @[LoopConv.scala 532:13]
end else begin
state <= _state_T_7; // @[LoopConv.scala 547:13]
end
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_outer_bounds_in_channels <= io_req_bits_outer_bounds_in_channels; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_outer_bounds_out_channels <= io_req_bits_outer_bounds_out_channels; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_outer_bounds_kernel_dim <= io_req_bits_outer_bounds_kernel_dim; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_inner_bounds_krows <= io_req_bits_inner_bounds_krows; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_inner_bounds_kcols <= io_req_bits_inner_bounds_kcols; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_inner_bounds_kchs <= io_req_bits_inner_bounds_kchs; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_derived_params_ochs <= io_req_bits_derived_params_ochs; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_derived_params_out_channels_per_bank <= io_req_bits_derived_params_out_channels_per_bank; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_derived_params_weight_spad_stride <= io_req_bits_derived_params_weight_spad_stride; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_addr_end <= io_req_bits_addr_end; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_dram_addr <= io_req_bits_dram_addr; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
req_loop_id <= io_req_bits_loop_id; // @[LoopConv.scala 554:9]
end
if (_T_3) begin // @[LoopConv.scala 553:22]
och <= 16'h0; // @[LoopConv.scala 559:9]
end else if (_T_1) begin // @[LoopConv.scala 530:30]
if (!(_command_p_io_in_bits_cmd_T)) begin // @[LoopConv.scala 531:29]
if (!(_next_och_T_7)) begin // @[Mux.scala 101:16]
och <= _next_och_T_10;
end
end
end
if (_T_3) begin // @[LoopConv.scala 553:22]
krow <= 16'h0; // @[LoopConv.scala 558:10]
end else if (_T_1) begin // @[LoopConv.scala 530:30]
if (!(_command_p_io_in_bits_cmd_T)) begin // @[LoopConv.scala 531:29]
if (!(_next_krow_T_5)) begin // @[Mux.scala 101:16]
krow <= _next_krow_T_8;
end
end
end
if (_T_3) begin // @[LoopConv.scala 553:22]
kcol <= 16'h0; // @[LoopConv.scala 557:10]
end else if (_T_1) begin // @[LoopConv.scala 530:30]
if (!(_command_p_io_in_bits_cmd_T)) begin // @[LoopConv.scala 531:29]
if (!(_next_kcol_T_3)) begin // @[Mux.scala 101:16]
kcol <= _next_kcol_T_6;
end
end
end
if (_T_3) begin // @[LoopConv.scala 553:22]
kch <= 16'h0; // @[LoopConv.scala 556:9]
end else if (_T_1) begin // @[LoopConv.scala 530:30]
if (!(_command_p_io_in_bits_cmd_T)) begin // @[LoopConv.scala 531:29]
if (_next_kch_T_4) begin // @[Mux.scala 101:16]
kch <= 16'h0;
end else begin
kch <= _next_kch_T_1;
end
end
end
end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
`ifdef RANDOMIZE
`ifdef INIT_RANDOM
`INIT_RANDOM
`endif
`ifndef VERILATOR
`ifdef RANDOMIZE_DELAY
#`RANDOMIZE_DELAY begin end
`else
#0.002 begin end
`endif
`endif
`ifdef RANDOMIZE_REG_INIT
_RAND_0 = {1{`RANDOM}};
state = _RAND_0[1:0];
_RAND_1 = {1{`RANDOM}};
req_outer_bounds_in_channels = _RAND_1[15:0];
_RAND_2 = {1{`RANDOM}};
req_outer_bounds_out_channels = _RAND_2[15:0];
_RAND_3 = {1{`RANDOM}};
req_outer_bounds_kernel_dim = _RAND_3[15:0];
_RAND_4 = {1{`RANDOM}};
req_inner_bounds_krows = _RAND_4[15:0];
_RAND_5 = {1{`RANDOM}};
req_inner_bounds_kcols = _RAND_5[15:0];
_RAND_6 = {1{`RANDOM}};
req_inner_bounds_kchs = _RAND_6[15:0];
_RAND_7 = {1{`RANDOM}};
req_derived_params_ochs = _RAND_7[15:0];
_RAND_8 = {1{`RANDOM}};
req_derived_params_out_channels_per_bank = _RAND_8[15:0];
_RAND_9 = {1{`RANDOM}};
req_derived_params_weight_spad_stride = _RAND_9[15:0];
_RAND_10 = {1{`RANDOM}};
req_addr_end = _RAND_10[14:0];
_RAND_11 = {2{`RANDOM}};
req_dram_addr = _RAND_11[39:0];
_RAND_12 = {1{`RANDOM}};
req_loop_id = _RAND_12[0:0];
_RAND_13 = {1{`RANDOM}};
och = _RAND_13[15:0];
_RAND_14 = {1{`RANDOM}};
krow = _RAND_14[15:0];
_RAND_15 = {1{`RANDOM}};
kcol = _RAND_15[15:0];
_RAND_16 = {1{`RANDOM}};
kch = _RAND_16[15:0];
`endif // RANDOMIZE_REG_INIT
`endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule



module Pipeline_12(
input clock,
input reset,
output io_in_ready,
input io_in_valid,
input [6:0] io_in_bits_cmd_inst_funct,
input [63:0] io_in_bits_cmd_rs1,
input [63:0] io_in_bits_cmd_rs2,
input [67:0] io_in_bits_dram_addr,
input [63:0] io_in_bits_spad_addr,
input [15:0] io_in_bits_K,
input [15:0] io_in_bits_J,
input io_out_ready,
output io_out_valid,
output [6:0] io_out_bits_cmd_inst_funct,
output [63:0] io_out_bits_cmd_rs1,
output [63:0] io_out_bits_cmd_rs2,
output [67:0] io_out_bits_dram_addr,
output [63:0] io_out_bits_spad_addr,
output [15:0] io_out_bits_K,
output [15:0] io_out_bits_J,
output io_busy
);
`ifdef RANDOMIZE_REG_INIT
reg [31:0] _RAND_0;
reg [63:0] _RAND_1;
reg [63:0] _RAND_2;
reg [95:0] _RAND_3;
reg [63:0] _RAND_4;
reg [31:0] _RAND_5;
reg [31:0] _RAND_6;
reg [31:0] _RAND_7;
reg [63:0] _RAND_8;
reg [63:0] _RAND_9;
reg [95:0] _RAND_10;
reg [63:0] _RAND_11;
reg [31:0] _RAND_12;
reg [31:0] _RAND_13;
reg [31:0] _RAND_14;
reg [31:0] _RAND_15;
`endif // RANDOMIZE_REG_INIT
reg [6:0] stages_0_cmd_inst_funct; // @[Pipeline.scala 21:21]
reg [63:0] stages_0_cmd_rs1; // @[Pipeline.scala 21:21]
reg [63:0] stages_0_cmd_rs2; // @[Pipeline.scala 21:21]
reg [67:0] stages_0_dram_addr; // @[Pipeline.scala 21:21]
reg [63:0] stages_0_spad_addr; // @[Pipeline.scala 21:21]
reg [15:0] stages_0_K; // @[Pipeline.scala 21:21]
reg [15:0] stages_0_J; // @[Pipeline.scala 21:21]
reg [6:0] stages_1_cmd_inst_funct; // @[Pipeline.scala 21:21]
reg [63:0] stages_1_cmd_rs1; // @[Pipeline.scala 21:21]
reg [63:0] stages_1_cmd_rs2; // @[Pipeline.scala 21:21]
reg [67:0] stages_1_dram_addr; // @[Pipeline.scala 21:21]
reg [63:0] stages_1_spad_addr; // @[Pipeline.scala 21:21]
reg [15:0] stages_1_K; // @[Pipeline.scala 21:21]
reg [15:0] stages_1_J; // @[Pipeline.scala 21:21]
reg valids_0; // @[Pipeline.scala 22:25]
reg valids_1; // @[Pipeline.scala 22:25]
wire stalling_1 = valids_1 & ~io_out_ready; // @[Pipeline.scala 28:34]
wire stalling_0 = valids_0 & stalling_1; // @[Pipeline.scala 30:16]
wire _GEN_0 = io_out_ready ? 1'h0 : valids_1; // @[Pipeline.scala 36:24 37:19 22:25]
wire _T = ~stalling_1; // @[Pipeline.scala 40:12]
wire _GEN_1 = ~stalling_1 ? 1'h0 : valids_0; // @[Pipeline.scala 40:17 41:12 22:25]
wire _T_1 = io_in_ready & io_in_valid; // @[Decoupled.scala 50:35]
wire _GEN_2 = _T_1 | _GEN_1; // @[Pipeline.scala 45:22 46:19]
wire _GEN_3 = valids_0 | _GEN_0; // @[Pipeline.scala 49:16 50:12]
assign io_in_ready = ~stalling_0; // @[Pipeline.scala 27:20]
assign io_out_valid = valids_1; // @[Pipeline.scala 35:18]
assign io_out_bits_cmd_inst_funct = stages_1_cmd_inst_funct; // @[Pipeline.scala 58:17]
assign io_out_bits_cmd_rs1 = stages_1_cmd_rs1; // @[Pipeline.scala 58:17]
assign io_out_bits_cmd_rs2 = stages_1_cmd_rs2; // @[Pipeline.scala 58:17]
assign io_out_bits_dram_addr = stages_1_dram_addr; // @[Pipeline.scala 58:17]
assign io_out_bits_spad_addr = stages_1_spad_addr; // @[Pipeline.scala 58:17]
assign io_out_bits_K = stages_1_K; // @[Pipeline.scala 58:17]
assign io_out_bits_J = stages_1_J; // @[Pipeline.scala 58:17]
assign io_busy = io_in_valid | (valids_0 | valids_1); // @[Pipeline.scala 24:28]
always @(posedge clock) begin
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_cmd_inst_funct <= io_in_bits_cmd_inst_funct; // @[Pipeline.scala 56:19]
end
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_cmd_rs1 <= io_in_bits_cmd_rs1; // @[Pipeline.scala 56:19]
end
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_cmd_rs2 <= io_in_bits_cmd_rs2; // @[Pipeline.scala 56:19]
end
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_dram_addr <= io_in_bits_dram_addr; // @[Pipeline.scala 56:19]
end
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_spad_addr <= io_in_bits_spad_addr; // @[Pipeline.scala 56:19]
end
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_K <= io_in_bits_K; // @[Pipeline.scala 56:19]
end
if (_T_1) begin // @[Pipeline.scala 55:22]
stages_0_J <= io_in_bits_J; // @[Pipeline.scala 56:19]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_cmd_inst_funct <= stages_0_cmd_inst_funct; // @[Pipeline.scala 61:13]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_cmd_rs1 <= stages_0_cmd_rs1; // @[Pipeline.scala 61:13]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_cmd_rs2 <= stages_0_cmd_rs2; // @[Pipeline.scala 61:13]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_dram_addr <= stages_0_dram_addr; // @[Pipeline.scala 61:13]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_spad_addr <= stages_0_spad_addr; // @[Pipeline.scala 61:13]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_K <= stages_0_K; // @[Pipeline.scala 61:13]
end
if (_T) begin // @[Pipeline.scala 60:17]
stages_1_J <= stages_0_J; // @[Pipeline.scala 61:13]
end
if (reset) begin // @[Pipeline.scala 22:25]
valids_0 <= 1'h0; // @[Pipeline.scala 22:25]
end else begin
valids_0 <= _GEN_2;
end
if (reset) begin // @[Pipeline.scala 22:25]
valids_1 <= 1'h0; // @[Pipeline.scala 22:25]
end else begin
valids_1 <= _GEN_3;
end
end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
`ifdef RANDOMIZE
`ifdef INIT_RANDOM
`INIT_RANDOM
`endif
`ifndef VERILATOR
`ifdef RANDOMIZE_DELAY
#`RANDOMIZE_DELAY begin end
`else
#0.002 begin end
`endif
`endif
`ifdef RANDOMIZE_REG_INIT
_RAND_0 = {1{`RANDOM}};
stages_0_cmd_inst_funct = _RAND_0[6:0];
_RAND_1 = {2{`RANDOM}};
stages_0_cmd_rs1 = _RAND_1[63:0];
_RAND_2 = {2{`RANDOM}};
stages_0_cmd_rs2 = _RAND_2[63:0];
_RAND_3 = {3{`RANDOM}};
stages_0_dram_addr = _RAND_3[67:0];
_RAND_4 = {2{`RANDOM}};
stages_0_spad_addr = _RAND_4[63:0];
_RAND_5 = {1{`RANDOM}};
stages_0_K = _RAND_5[15:0];
_RAND_6 = {1{`RANDOM}};
stages_0_J = _RAND_6[15:0];
_RAND_7 = {1{`RANDOM}};
stages_1_cmd_inst_funct = _RAND_7[6:0];
_RAND_8 = {2{`RANDOM}};
stages_1_cmd_rs1 = _RAND_8[63:0];
_RAND_9 = {2{`RANDOM}};
stages_1_cmd_rs2 = _RAND_9[63:0];
_RAND_10 = {3{`RANDOM}};
stages_1_dram_addr = _RAND_10[67:0];
_RAND_11 = {2{`RANDOM}};
stages_1_spad_addr = _RAND_11[63:0];
_RAND_12 = {1{`RANDOM}};
stages_1_K = _RAND_12[15:0];
_RAND_13 = {1{`RANDOM}};
stages_1_J = _RAND_13[15:0];
_RAND_14 = {1{`RANDOM}};
valids_0 = _RAND_14[0:0];
_RAND_15 = {1{`RANDOM}};
valids_1 = _RAND_15[0:0];
`endif // RANDOMIZE_REG_INIT
`endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule