module cisc_cpu_p(
  input clk,
  input rst,
  output [31:0] bus_A_ins,
  input [31:0] bus_in_ins,
  output [31:0] bus_A_data,
  input [31:0] bus_in_data,
  output [31:0] bus_out,
  output bus_WE,
  output bus_RE_data,
  output bus_RE_ins,
  output [7:0] current_opcode);

//Kontrolleinheit der sequenziellen Implementierung
  reg [5:1] full;  
  wire [4:0] ue={ full[4:1], full[5] };

  always @(posedge clk or posedge rst) begin
    if(rst)
		full='b10000;
	else
		//full={ ue[4], ue[3], ue[2], ue[1], ue[0] };
		full='b11111;
  end
//Kontrolleinheit END

//Kontrolleinheit für die Pipeline
	reg [31:0] IR_IF;
	reg [31:0] IR_DEC;
	reg [31:0] IR_EXE;
	reg [31:0] IR_MEM;
	reg [31:0] IR_WB;
	reg [2:0] i = 0;
	//reg [2:0] r = 0;
	reg [31:0] bubble;
	
	//Schaltung wenn der neue Befehl auf ein Register zugreift dass von einem vorherigen Befehl noch bearbeitet wird -> Stalling
	always @(posedge clk) begin
		//if(RD_bus || RS_bus == RD_IF || RD_DEC) begin
		//	IP = IP_IF;				//halte die nächste Adresse
	//	end
	
	end
 	
//Kontrolleinheit END

initial begin
IR_IF<=0;
IR_DEC<=0;
IR_EXE<=0;
IR_MEM<=0;
IR_WB<=0;
bubble<=0;
ZF_IF = 0;
ZF_DEC = 0;
ZF_EXE = 0;
ZF_MEM = 0;
ZF_WB = 0;
end
//STUFE 1 Instruction Fetch
  always @(posedge clk)begin
  //move the instructions through the 5 steps; every IR entry is supposed to be processed by a different step
     if(!rst) begin
     IR_WB=IR_MEM;
     IR_MEM=IR_EXE;
     IR_EXE=IR_DEC;
     IR_DEC=IR_IF;
     IR_IF=bus_in_ins;//(RD_IF==RD_DEC || RD_EXE || RD_MEM)?bubble:(RS_IF == RS_DEC || RS_EXE || RS_MEM)?bubble:bus_in_ins;
     end
  end
//STUFE 1 ENDE

  reg [31:0] A, B, A_EXE, B_EXE, A_MEM, B_MEM;
  wire [31:0] Aop, Bop;

//STUFE 2

  reg ZF_IF, ZF_DEC, ZF_EXE, ZF_MEM, ZF_WB;//, ZF_bus;
  

/////////Dekodierung für Instruktion in bus_in_ins
  
  //OPcode & mod-Feld der Anweisung
  wire [7:0] opcode_bus=bus_in_ins[7:0]; //IR_IF[7:0];
  wire [1:0] mod_bus=bus_in_ins[15:14];// IR_IF[15:14];
  
  //RIadd/sub 5bit modifier
  wire [4:0] RImod_bus=bus_in_ins[15:11];//IR_IF[15:11];
  
//Instruktionen OPcodes
  wire load_bus=opcode_bus=='h8b && mod_bus==1;
  wire move_bus=opcode_bus=='h89 && mod_bus==3;
  wire store_bus=opcode_bus=='h89 && mod_bus==1;
  wire add_bus=opcode_bus=='h01;
  wire sub_bus=opcode_bus=='h29;
  wire jnez_bus=opcode_bus=='h75;
//Erweiterungen
  wire RIadd_bus=opcode_bus=='h83 && RImod_bus==24;
  wire RIsub_bus=opcode_bus=='h83 && RImod_bus==29;

//identifizierung für Speicher- und arithmetische Befehle
  wire memory_bus=load_bus || store_bus;
  wire aluop_bus=add_bus || sub_bus;
  wire aluop_k_bus=RIadd_bus || RIsub_bus;
  wire halt_bus=opcode_bus=='hf4;  //hlt instruction
//END

//Berechnung der Register nummern
  wire [4:0] RD_bus=bus_in_ins[10:8];
  wire [4:0] RS_bus=bus_in_ins[13:11];
  wire [4:0] Aad_bus=memory_bus?6:RD_bus,		//Aad bekommt den Wert 6 für das esi Register für RMmov & MRmov; für alle anderen Befehle, Aad bekommt den Wert für das Register RD
             Bad_bus=RS_bus;
//END
//nextIP-Modul
  wire [31:0] distance_bus={ { 24 { bus_in_ins[15] } }, bus_in_ins[15:8] };
  wire [31:0] displacement_bus={ { 24 { bus_in_ins[23] } }, bus_in_ins[23:16] };
  //wire btaken_bus=jnez_bus && !ZF_bus;
  wire [1:0] length_bus=memory_bus                 ?3:
                    (aluop_bus || move_bus || jnez_bus || aluop_k_bus)?2:
                                            1;

/////////Dekodierung bus_in_ins ende

  /////////Dekodierung für Instruktion in IR_IF
  
  //OPcode & mod-Feld der Anweisung
  wire [7:0] opcode_IF=IR_IF[7:0]; //IR_IF[7:0];
  wire [1:0] mod_IF=IR_IF[15:14];// IR_IF[15:14];
  
  //RIadd/sub 5bit modifier
  wire [4:0] RImod_IF=IR_IF[15:11];//IR_IF[15:11];
  
//Instruktionen OPcodes
  wire load_IF=opcode_IF=='h8b && mod_IF==1;
  wire move_IF=opcode_IF=='h89 && mod_IF==3;
  wire store_IF=opcode_IF=='h89 && mod_IF==1;
  wire add_IF=opcode_IF=='h01;
  wire sub_IF=opcode_IF=='h29;
  wire jnez_IF=opcode_IF=='h75;
//Erweiterungen
  wire RIadd_IF=opcode_IF=='h83 && RImod_IF==24;
  wire RIsub_IF=opcode_IF=='h83 && RImod_IF==29;

//identifizierung für Speicher- und arithmetische Befehle
  wire memory_IF=load_IF || store_IF;
  wire aluop_IF=add_IF || sub_IF;
  wire aluop_k_IF=RIadd_IF || RIsub_IF;
  wire halt_IF=opcode_IF=='hf4;  //hlt instruction
//END

//Berechnung der Register nummern
  wire [4:0] RD_IF=IR_IF[10:8];
  wire [4:0] RS_IF=IR_IF[13:11];
  wire [4:0] Aad_IF=memory_IF?6:RD_IF,		//Aad bekommt den Wert 6 für das esi Register für RMmov & MRmov; für alle anderen Befehle, Aad bekommt den Wert für das Register RD
             Bad_IF=RS_IF;
//END
//nextIP-Modul
  wire [31:0] distance_IF={ { 24 { IR_IF[15] } }, IR_IF[15:8] };
  wire [31:0] displacement_IF={ { 24 { IR_IF[23] } }, IR_IF[23:16] };
  wire btaken_IF=jnez_IF && !ZF_IF;
  wire [1:0] length_IF=memory_IF                 ?3:
                    (aluop_IF || move_IF || jnez_IF || aluop_k_IF)?2:
                                            1;

/////////Dekodierung IR_IF ende
  
/////////Dekodierung für Instruktion in IR_DEC

//OPcode & mod-Feld der Anweisung
  wire [7:0] opcode_DEC=IR_DEC[7:0];
  wire [1:0] mod_DEC=IR_DEC[15:14];
  
  //RIadd/sub 5bit modifier
  wire [4:0] RImod_DEC=IR_DEC[15:11];

//Instruktionen OPcodes
  wire load_DEC=opcode_DEC=='h8b && mod_DEC==1;
  wire move_DEC=opcode_DEC=='h89 && mod_DEC==3;
  wire store_DEC=opcode_DEC=='h89 && mod_DEC==1;
  wire add_DEC=opcode_DEC=='h01;
  wire sub_DEC=opcode_DEC=='h29;
  wire jnez_DEC=opcode_DEC=='h75;
//Erweiterungen
  wire RIadd_DEC=opcode_DEC=='h83 && RImod_DEC==24;
  wire RIsub_DEC=opcode_DEC=='h83 && RImod_DEC==29;

//identifizierung für Speicher- und arithmetische Befehle
  wire memory_DEC=load_DEC || store_DEC;
  wire aluop_DEC=add_DEC || sub_DEC;
  wire aluop_k_DEC=RIadd_DEC || RIsub_DEC;
  wire halt_DEC=opcode_DEC=='hf4;  //hlt instruction
//END

//Berechnung der Register nummern
  wire [4:0] RD_DEC=IR_DEC[10:8];
  wire [4:0] RS_DEC=IR_DEC[13:11];
  wire [4:0] Aad_DEC=memory_DEC?6:RD_DEC,		//Aad bekommt den Wert 6 für das esi Register für RMmov & MRmov; für alle anderen Befehle, Aad bekommt den Wert für das Register RD
             Bad_DEC=RS_DEC;
//END
//nextIP-Modul
  wire [31:0] distance_DEC={ { 24 { IR_DEC[15] } }, IR_DEC[15:8] };
  wire [31:0] displacement_DEC={ { 24 { IR_DEC[23] } }, IR_DEC[23:16] };
  wire btaken_DEC=jnez_DEC && !ZF_DEC;
  wire [1:0] length_DEC=memory_DEC                 ?3:
                    (aluop_DEC || move_DEC || jnez_DEC || aluop_k_DEC)?2:
                                            1;

/////////Dekodierung IR_DEC ende

/////////Dekodierung für Instruktion in IR_EXE

//OPcode & mod-Feld der Anweisung
  wire [7:0] opcode_EXE=IR_EXE[7:0];
  wire [1:0] mod_EXE=IR_EXE[15:14];
  
  //RIadd/sub 5bit modifier
  wire [4:0] RImod_EXE=IR_EXE[15:11];

//Instruktionen OPcodes
  wire load_EXE=opcode_EXE=='h8b && mod_EXE==1;
  wire move_EXE=opcode_EXE=='h89 && mod_EXE==3;
  wire store_EXE=opcode_EXE=='h89 && mod_EXE==1;
  wire add_EXE=opcode_EXE=='h01;
  wire sub_EXE=opcode_EXE=='h29;
  wire jnez_EXE=opcode_EXE=='h75;
//Erweiterungen
  wire RIadd_EXE=opcode_EXE=='h83 && RImod_EXE==24;
  wire RIsub_EXE=opcode_EXE=='h83 && RImod_EXE==29;
  wire valid = (load_EXE||move_EXE||store_EXE||add_EXE||sub_EXE||jnez_EXE||RIadd_EXE||RIsub_EXE)?1:0;

//identifizierung für Speicher- und arithmetische Befehle
  wire memory_EXE=valid?(load_EXE || store_EXE):0;
  wire aluop_EXE=valid?(add_EXE || sub_EXE):0;
  wire aluop_k_EXE=valid?(RIadd_EXE || RIsub_EXE):0;
  wire halt_EXE=opcode_EXE=='hf4;  //hlt instruction
//END

//Berechnung der Register nummern
  wire [4:0] RD_EXE=valid?IR_EXE[10:8]:0;
  wire [4:0] RS_EXE=valid?IR_EXE[13:11]:0;
  wire [4:0] Aad_EXE=valid?(memory_EXE?6:RD_EXE):0,		//Aad bekommt den Wert 6 für das esi Register für RMmov & MRmov; für alle anderen Befehle, Aad bekommt den Wert für das Register RD
             Bad_EXE=valid?RS_EXE:0;
//END
//nextIP-Modul
  wire [31:0] distance_EXE={ { 24 { IR_EXE[15] } }, IR_EXE[15:8] };
  wire [31:0] displacement_EXE={ { 24 { IR_EXE[23] } }, IR_EXE[23:16] };
  wire btaken_EXE=jnez_EXE && !ZF_EXE;
  wire [1:0] length_EXE=memory_EXE                ?3:
                    (aluop_EXE || move_EXE || jnez_EXE || aluop_k_EXE)?2:
                                            1;

/////////Dekodierung IR_EXE ende

/////////Dekodierung für Instruktion in IR_MEM

//OPcode & mod-Feld der Anweisung
  wire [7:0] opcode_MEM=IR_MEM[7:0];
  wire [1:0] mod_MEM=IR_MEM[15:14];
  
  //RIadd/sub 5bit modifier
  wire [4:0] RImod_MEM=IR_MEM[15:11];

//Instruktionen OPcodes
  wire load_MEM=opcode_MEM=='h8b && mod_MEM==1;
  wire move_MEM=opcode_MEM=='h89 && mod_MEM==3;
  wire store_MEM=opcode_MEM=='h89 && mod_MEM==1;
  wire add_MEM=opcode_MEM=='h01;
  wire sub_MEM=opcode_MEM=='h29;
  wire jnez_MEM=opcode_MEM=='h75;
//Erweiterungen
  wire RIadd_MEM=opcode_MEM=='h83 && RImod_MEM==24;
  wire RIsub_MEM=opcode_MEM=='h83 && RImod_MEM==29;

//identifizierung für Speicher- und arithmetische Befehle
  wire memory_MEM=load_MEM || store_MEM;
  wire aluop_MEM=add_MEM || sub_MEM;
  wire aluop_k_MEM=RIadd_MEM || RIsub_MEM;
  wire halt_MEM=opcode_MEM=='hf4;  //hlt instruction
//END

//Berechnung der Register nummern
  wire [4:0] RD_MEM=IR_MEM[10:8];
  wire [4:0] RS_MEM=IR_MEM[13:11];
  wire [4:0] Aad_MEM=memory_MEM?6:RD_MEM,		//Aad bekommt den Wert 6 für das esi Register für RMmov & MRmov; für alle anderen Befehle, Aad bekommt den Wert für das Register RD
             Bad_MEM=RS_MEM;
//END
//nextIP-Modul
  wire [31:0] distance_MEM={ { 24 { IR_MEM[15] } }, IR_MEM[15:8] };
  wire [31:0] displacement_MEM={ { 24 { IR_MEM[23] } }, IR_MEM[23:16] };
  wire btaken_MEM=jnez_MEM && !ZF_MEM;
  wire [1:0] length_MEM=memory_MEM                ?3:
                    (aluop_MEM || move_MEM || jnez_MEM || aluop_k_MEM)?2:
                                            1;

/////////Dekodierung IR_MEM ende

/////////Dekodierung für Instruktion in IR_WB

//OPcode & mod-Feld der Anweisung
  wire [7:0] opcode_WB=IR_WB[7:0];
  wire [1:0] mod_WB=IR_WB[15:14];
  
  //RIadd/sub 5bit modifier
  wire [4:0] RImod_WB=IR_WB[15:11];

//Instruktionen OPcodes
  wire load_WB=opcode_WB=='h8b && mod_WB==1;
  wire move_WB=opcode_WB=='h89 && mod_WB==3;
  wire store_WB=opcode_WB=='h89 && mod_WB==1;
  wire add_WB=opcode_WB=='h01;
  wire sub_WB=opcode_WB=='h29;
  wire jnez_WB=opcode_WB=='h75;
//Erweiterungen
  wire RIadd_WB=opcode_WB=='h83 && RImod_WB==24;
  wire RIsub_WB=opcode_WB=='h83 && RImod_WB==29;

//identifizierung für Speicher- und arithmetische Befehle
  wire memory_WB=load_WB || store_WB;
  wire aluop_WB=add_WB || sub_WB;
  wire aluop_k_WB=RIadd_WB || RIsub_WB;
  wire halt_WB=opcode_WB=='hf4;  //hlt instruction
//END

//Berechnung der Register nummern
  wire [4:0] RD_WB=IR_WB[10:8];
  wire [4:0] RS_WB=IR_WB[13:11];
  wire [4:0] Aad_WB=memory_WB?6:RD_WB,		//Aad bekommt den Wert 6 für das esi Register für RMmov & MRmov; für alle anderen Befehle, Aad bekommt den Wert für das Register RD
             Bad_WB=RS_WB;
//END
//nextIP-Modul
  wire [31:0] distance_WB={ { 24 { IR_WB[15] } }, IR_WB[15:8] };
  wire [31:0] displacement_WB={ { 24 { IR_WB[23] } }, IR_WB[23:16] };
  wire btaken_WB=jnez_WB && !ZF_WB;
  wire [1:0] length_WB=memory_WB                ?3:
                    (aluop_WB || move_WB || jnez_WB || aluop_k_WB)?2:
                                            1;

/////////Dekodierung IR_WB ende
          
          
 /*                         
  wire [31:0] distance_IN={ { 24 { bus_A_ins[15] } }, bus_A_ins[15:8] };
  wire [31:0] displacement_IN={ { 24 { bus_A_ins[23] } }, bus_A_ins[23:16] };
  wire btaken_IF=jnez_IF && !ZF_IF;
  wire [1:0] length_IF=memory_IF                 ?3:
                    (aluop_IF || move_IF || jnez_IF || aluop_k_IF)?2:
                                            1;
   */                       
                                            
//nextIP //leicht erweitert vom Buch(rst;!halt- teile)
  reg [31:0] IP_IF;
  //nächster Wert für den Instruction Pointer wird direkt von der nächsten Adresse auf dem Adress-Bus berechnet
  //Wenn ein F4 hlt befehl kommt, wird der IP auf den letzten Wert von IP_IF gehalten, welches die Adresse für den hlt Befehl ist
  wire [31:0] IP = rst?0:btaken_WB?IP_IF+distance_WB:!halt_bus?IP_IF+length_bus:IP_IF;
  always @(posedge clk or posedge rst) begin
    if(rst)
	    IP_IF=0;
    else if(ue[1]) begin
    //Wenn !hlt, dann wird IP_IF um die berechnete Länge der neuen Instruktion erhöht
      if(!halt_bus) IP_IF=IP_IF+length_bus;
    //Wenn btaken gesetzt ist wird IP_IF um die Sprungdistanz erhöht; die Länge der neuen Instruktion wird
    //abgezogen, da sie in der vorigen Zeile zusätzlich addiert wurde und dadurch die Sprunglänge verfälscht
      if(btaken_WB) IP_IF=IP_IF+distance_WB;
    end
end

//NEXTIP END
  reg [31:0] MAR_EXE, MAR_MEM, MDRw_EXE, MDRw_MEM;//, C_EXE,
  reg [31:0] C_MEM, C_WB;    
	//Konstante K die in RIadd/sub verwendet wird
  reg [31:0] K = 10;

//STAGE 3 ALU-Modul
  wire [31:0] ALU_op2_EXE=memory_EXE?displacement_EXE:sub_EXE?~Bop:Bop;
  wire [31:0] ALUout_EXE=A_EXE+ALU_op2_EXE+sub_EXE; 				//esi+displacement für RM- und MRmov;; für ADD ALU_op2=B führt zu ALUout=A+B;; für SUB ALUout=A+~B+1

//Berechnungen für RIadd/sub
  //wire [31:0] ALU_K2_EXE = RIsub_EXE?~K:K;
  wire [31:0] ALU_K2_EXE=RIsub_EXE?~K:K;
  wire [31:0] ALU_K_EXE=A_EXE+ALU_K2_EXE+RIsub_EXE;
  //wire [31:0] C_EXE=move_EXE?B_EXE:aluop_k_EXE?ALU_K_EXE:ALUout_EXE;
  reg [31:0] C_EXE;

  always @(posedge clk or posedge rst)
    if(rst)
	   ZF_EXE=0;
    else if(ue[2]) begin
	    A_EXE=Aop;							//A beinhaltet den Wert für RD
      B_EXE=Bop;							//B beinhaltet den Wert für RS
		//RIadd/sub: aluop_k abfrage zu MAR und C hinzugefügt 
      MAR_EXE=aluop_k_EXE?ALU_K_EXE:ALUout_EXE;
      C_EXE=move_EXE?B_EXE:aluop_k_EXE?ALU_K_EXE:ALUout_EXE;
      MDRw_EXE=B_EXE;
      if(aluop_EXE || aluop_k_EXE) ZF_EXE=(ALUout_EXE==0||ALU_K_EXE==0); 			//RIadd/sub: aluop_k als Bedingung hinzugefügt;; ZF=(... ||ALU_K==0) hinzugefügt
    end
//STAGE 3 ALU-Modul END

  reg [31:0] MDRr_MEM, MDRr_WB;
  
  //STAGE 4 MEMORY
  always @(posedge clk) begin
    if(ue[3] && load_MEM) MDRr_MEM=bus_in_data;
      //if(load) MDRr=bus_in_data;
    ZF_MEM = ZF_EXE;
    ZF_WB = ZF_MEM;
    MAR_MEM = MAR_EXE;
    C_WB = C_MEM;
    A_MEM = A_EXE;
    B_MEM = B_EXE;
    C_MEM = C_EXE;
    MDRw_MEM = MDRw_EXE;
  end
  
  assign bus_A_data=ue[3]?MAR_EXE:0;
  assign bus_RE_data=(ue[3] && load_MEM);
  //STAGE 4
  
  assign bus_A_ins=IP;//ue[0]?IP:0;
  assign bus_RE_ins=1;//ue[0];
  
//Register-File START
  reg [31:0] R[7:0];
  
  //A ist der Wert von Register esi
  assign Aop=R[Aad_EXE];					//Aop bekommt den Wert des Registers R[Aad]; dies ist entweder esi oder RD
  assign Bop=R[Bad_EXE];
  
  assign bus_WE=store_MEM;//ue[3] && store;
  assign bus_out=MDRw_MEM;

// //STAGE 5 WRITE BACK
//   always @(posedge clk or posedge rst) begin
//       if(rst) begin
// 	      R[00]=0; R[01]=0; R[02]=0; R[03]=0; R[04]=0; R[05]=0; R[06]=0; R[07]=0;      
// 	    end
//   end

  always @(posedge clk or posedge rst) begin
    /// this causes an error in yosys:: "ERROR: Async reset \rst yields non-constant value 32'mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm for signal \MDRr_WB"
    /// separating the if(rst) into its own always block seems to have fixed the issue
    MDRr_WB <= MDRr_MEM;
    if(rst) begin
	    R[00]=0; R[01]=0; R[02]=0; R[03]=0; R[04]=0; R[05]=0; R[06]=0; R[07]=0;  
      MDRr_WB <= 0;
	  end 
	  else 
    if(ue[4]) 
      if(aluop_WB || move_WB || load_WB || aluop_k_WB)		//RIadd/sub: aluop_k als Bedingung hinzugefügt
        R[load_WB?RS_WB:RD_WB]=load_WB?MDRr_WB:C_WB; 
  end
//Register-File END		 
  assign current_opcode = opcode_WB;
  
  // synthesis translate_off
  
//outgoing wires 
  wire [31:0] eax = R[0];
  wire [31:0] ecx = R[1];
  wire [31:0] edx = R[2];
  wire [31:0] ebx = R[3];
  wire [31:0] esp = R[4];
  wire [31:0] ebp = R[5];
  wire [31:0] esi = R[6];
  wire [31:0] edi = R[7];
  wire [7:0] regs = IR_IF[15:8];
  
  // synthesis translate_on
  
endmodule