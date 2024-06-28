module eth_wishbone_tb();

// WISHBONE common
reg           WB_CLK_I = 0;       // WISHBONE clock
reg  [31:0]   WB_DAT_I = 0;       // WISHBONE data input
wire [31:0]   WB_DAT_O;       // WISHBONE data output

// WISHBONE slave
reg   [9:2]   WB_ADR_I = 0;       // WISHBONE address input
reg           WB_WE_I = 0;        // WISHBONE write enable input
reg   [3:0]   BDCs = 0;           // Buffer descriptors are selected
wire          WB_ACK_O;       // WISHBONE acknowledge output

// WISHBONE master
wire  [29:0]  m_wb_adr_o;     //
wire   [3:0]  m_wb_sel_o;     //
wire          m_wb_we_o;      //
wire  [31:0]  m_wb_dat_o;     //
wire          m_wb_cyc_o;     //
wire          m_wb_stb_o;     //
reg   [31:0]  m_wb_dat_i = 0;     //
reg           m_wb_ack_i = 0;     //
reg           m_wb_err_i = 0;     //

wire   [2:0]  m_wb_cti_o;     // Cycle Type Identifier
wire   [1:0]  m_wb_bte_o;     // Burst Type Extension

reg           Reset = 0;       // Reset signal

// Rx Status signals
reg           InvalidSymbol = 0;    // Invalid symbol was received during reception in 100 Mbps mode
reg           LatchedCrcError = 0;  // CRC error
reg           RxLateCollision = 0;  // Late collision occured while receiving frame
reg           ShortFrame = 0;       // Frame shorter then the minimum size
                                  // (r_MinFL) was received while small
                                  // packets are enabled (r_RecSmall)
reg           DribbleNibble = 0;    // Extra nibble received
reg           ReceivedPacketTooBig = 0;// Received packet is bigger than r_MaxFL
reg    [15:0] RxLength = 0;         // Length of the incoming frame
reg           LoadRxStatus = 0;     // Rx status was loaded
reg           ReceivedPacketGood = 0;  // Received packet's length and CRC are
                                     // good
reg           AddressMiss = 0;      // When a packet is received AddressMiss
                                  // status is written to the Rx BD
reg           r_RxFlow = 0;
reg           r_PassAll = 0;
reg           ReceivedPauseFrm = 0;

// Tx Status signals
reg     [3:0] RetryCntLatched = 0;  // Latched Retry Counter
reg           RetryLimit = 0;       // Retry limit reached (Retry Max value +1
                                  // attempts were made)
reg           LateCollLatched = 0;  // Late collision occured
reg           DeferLatched = 0;     // Defer indication (Frame was defered
                                  // before sucessfully sent)
wire          RstDeferLatched;
reg           CarrierSenseLost = 0; // Carrier Sense was lost during the
                                  // frame transmission

// Tx
reg           MTxClk = 0;         // Transmit clock (from PHY)
reg           TxUsedData = 0;     // Transmit packet used data
reg           TxRetry = 0;        // Transmit packet retry
reg           TxAbort = 0;        // Transmit packet abort
reg           TxDone = 0;         // Transmission ended
wire          TxStartFrm;     // Transmit packet start frame
wire          TxEndFrm;       // Transmit packet end frame
wire  [7:0]   TxData;         // Transmit packet data byte
wire          TxUnderRun;     // Transmit packet under-run
wire          PerPacketCrcEn; // Per packet crc enable
wire          PerPacketPad;   // Per packet pading

// Rx
reg           MRxClk = 0;         // Receive clock (from PHY)
reg   [7:0]   RxData = 0;         // Received data byte (from PHY)
reg           RxValid = 0;        //
reg           RxStartFrm = 0;     //
reg           RxEndFrm = 0;       //
reg           RxAbort = 0;        // This signal is set when address doesn't
                                // match.
wire          RxStatusWriteLatched_sync2;

//Register
reg           r_TxEn = 0;         // Transmit enable
reg           r_RxEn = 0;         // Receive enable
reg   [7:0]   r_TxBDNum = 0;      // Receive buffer descriptor number

// Interrupts
wire TxB_IRQ;
wire TxE_IRQ;
wire RxB_IRQ;
wire RxE_IRQ;
wire Busy_IRQ;


// Bist
`ifdef ETH_BIST
reg   mbist_si_i = 0;       // bist scan serial in
wire  mbist_so_o;       // bist scan serial out
reg [`ETH_MBIST_CTRL_WIDTH - 1:0] mbist_ctrl_i = 0; // bist chain shift control
`endif

`ifdef WISHBONE_DEBUG
   wire [31:0]                       dbg_dat0;
`endif

initial begin
#0
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
WB_DAT_I = 1;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 2;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
Reset = 1;
WB_DAT_I = 3;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 4;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
Reset = 0;
WB_DAT_I = 5;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 6;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
WB_DAT_I = 7;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 8;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
WB_DAT_I = 9;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 10;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
WB_DAT_I = 11;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 12;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
WB_DAT_I = 13;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 14;
#2
WB_CLK_I = 0;
MTxClk = 0;
MRxClk = 0;
WB_DAT_I = 15;
#2
WB_CLK_I = 1;
MTxClk = 1;
MRxClk = 1;
WB_DAT_I = 16;
#2
$finish;
end

eth_wishbone dut
  (

   // WISHBONE common
   .WB_CLK_I(WB_CLK_I), .WB_DAT_I(WB_DAT_I), .WB_DAT_O(WB_DAT_O),

   // WISHBONE slave
   .WB_ADR_I(WB_ADR_I), .WB_WE_I(WB_WE_I), .WB_ACK_O(WB_ACK_O),
   .BDCs(BDCs),

   .Reset(Reset),

   // WISHBONE master
   .m_wb_adr_o(m_wb_adr_o), .m_wb_sel_o(m_wb_sel_o), .m_wb_we_o(m_wb_we_o),
   .m_wb_dat_o(m_wb_dat_o), .m_wb_dat_i(m_wb_dat_i), .m_wb_cyc_o(m_wb_cyc_o),
   .m_wb_stb_o(m_wb_stb_o), .m_wb_ack_i(m_wb_ack_i), .m_wb_err_i(m_wb_err_i),

   .m_wb_cti_o(m_wb_cti_o), .m_wb_bte_o(m_wb_bte_o),

   //TX
   .MTxClk(MTxClk), .TxStartFrm(TxStartFrm), .TxEndFrm(TxEndFrm), .TxUsedData(TxUsedData), .TxData(TxData),
   .TxRetry(TxRetry), .TxAbort(TxAbort), .TxUnderRun(TxUnderRun), .TxDone(TxDone), .PerPacketCrcEn(PerPacketCrcEn),
   .PerPacketPad(PerPacketPad),

   //RX
   .MRxClk(MRxClk), .RxData(RxData), .RxValid(RxValid), .RxStartFrm(RxStartFrm), .RxEndFrm(RxEndFrm), .RxAbort(RxAbort),
   .RxStatusWriteLatched_sync2(RxStatusWriteLatched_sync2),

   // Register
   .r_TxEn(r_TxEn), .r_RxEn(r_RxEn), .r_TxBDNum(r_TxBDNum), .r_RxFlow(r_RxFlow), .r_PassAll(r_PassAll),

   // Interrupts
   .TxB_IRQ(TxB_IRQ), .TxE_IRQ(TxE_IRQ), .RxB_IRQ(RxB_IRQ), .RxE_IRQ(RxE_IRQ), .Busy_IRQ(Busy_IRQ),

   // Rx Status
   .InvalidSymbol(InvalidSymbol), .LatchedCrcError(LatchedCrcError), .RxLateCollision(RxLateCollision), .ShortFrame(ShortFrame), .DribbleNibble(DribbleNibble),
   .ReceivedPacketTooBig(ReceivedPacketTooBig), .RxLength(RxLength), .LoadRxStatus(LoadRxStatus), .ReceivedPacketGood(ReceivedPacketGood),
   .AddressMiss(AddressMiss),
   .ReceivedPauseFrm(ReceivedPauseFrm),

   // Tx Status
   .RetryCntLatched(RetryCntLatched), .RetryLimit(RetryLimit), .LateCollLatched(LateCollLatched), .DeferLatched(DeferLatched), .RstDeferLatched(RstDeferLatched),
   .CarrierSenseLost(CarrierSenseLost)

   // Bist
`ifdef ETH_BIST
   ,
   // debug chain signals
   .mbist_si_i(mbist_si_i),       // bist scan serial in
   .mbist_so_o(mbist_so_o),       // bist scan serial out
   .mbist_ctrl_i(mbist_ctrl_i)        // bist chain shift control
`endif

`ifdef WISHBONE_DEBUG
   ,
   .dbg_dat0(dbg_dat0)
`endif

   );

   	initial begin
       	$dumpfile(`DUMP_FILE_NAME);
       	$dumpvars(0);
       end

endmodule