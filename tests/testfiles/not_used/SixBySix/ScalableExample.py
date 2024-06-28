from ScalableModuleTestBench import TestBench
from MUX import mux2to1
from Docu import Docu
VarInput = 6;
VarOutput = 6;
varWidth = 6;
operation1 = 1;     #1 = Adder
operation2 = 2;     #2 = AND
opList = ["+", "&", "|", "MUX"]
operationNum = 1;
moduleNum = 6;
ModuleLevel = 3;

# randomly shuffle a sequence
#from numpy.random import seed
#from numpy.random import shuffle
#from random import seed
#from random import randint
# seed random number generator
#seed(1)
# prepare a sequence
#sequence = [i for i in range(10)]
#print(sequence)
# randomly shuffle the sequence
#shuffle(sequence)
#print(sequence)
#sequence.pop(randint(1, 10))
#value = sequence[0]; 
#print(value)
#Modvalue = value % 3
fileName = "SixBySixScalable"
f = open(fileName +".v", "w")
prod = " & "
match ModuleLevel:
    case 1:
        f.write("module Scalable(")
        for x in range(VarInput):
            f.write("in_" + str(x) + ",")
        for x in range (VarOutput):
            f.write("out_" + str(x))
            if(x < VarOutput - 1):
                f.write(",")
        f.write(");\n")
        for x in range(VarInput):
            if varWidth == 0:
                f.write("   input wire in_" + str(x) + ";\n")
            else:
                f.write("   input [" + str(varWidth) + ":0] in_" + str(x) + ";\n")
        
        for x in range (VarOutput):
            if varWidth == 0:
                f.write("out_wire in_" + str(x) + ";\n")
            else:
                f.write("   output [" + str(varWidth) + ":0] out_" + str(x) + ";\n")       

        for x in range (moduleNum + 1):    
            if varWidth == 0:
                f.write("wire wire" + str(x) + ";\n")
            else:
                f.write("   reg [" + str(varWidth) + ":0]" + "res" + str(x) + ";\n")
            
            #match operation1:
            #    case 1:

            #intial gate = two inputs and one output
        f.write("\n   assign ")
        f.write("res0 = ")
        for i in range (VarInput):
            f.write("in_"+str(i))
            if( i < VarOutput):
                f.write(" & ")
            f.write(";")
            #    case 2:
        for xi in range (moduleNum):
            f.write("\n   assign ")
            f.write("res" + str(xi + 1) + " = " + "in_" + str(xi+2) + prod + "res" + str(xi))


        f.write("\n   assign out_"+ str(VarOutput - 1) + " = res" + str(moduleNum) + "\n")
        f.write("\n endmodule")

    case 2: 
        scale = 0     
        VarInput = 2  
        VarOutput = 2
        f.write("module S(")
        for x in range(VarInput):
            f.write("in_" + str(x) + ",")
        for x in range (VarOutput):
            f.write("out_" + str(x))
            if(x < VarOutput - 1):
                f.write(",")
        f.write(");\n")
        for x in range(VarInput):
            if varWidth == 0:
                f.write("   input wire in_" + str(x) + ";\n")
            else:
                f.write("   input [" + str(varWidth) + ":0] in_" + str(x) + ";\n")
        for x in range (VarOutput):
            if varWidth == 0:
                f.write("   output wire out_" + str(x) + ";\n")
            else:
                f.write("   output [" + str(varWidth) + ":0] out_" + str(x) + ";\n")
        #Python random number generator
        #for x in range (VarOutput):
            # seed random number generator
            #seed(1)
            # prepare a sequence
            #sequence = [i for i in range(3)]
            #print(sequence)
            # randomly shuffle the sequence
            #shuffle(sequence)
            #print(sequence)
            #sequence.pop(randint(1, 3))
            #print(sequence)
            #value = sequence[0]; 
            #print(value)  
            #value = value + scale
            #scale = scale + 1
            #print(value)
        
        for x in range (VarOutput):
            
            match operation1:
                case 1:
                    f.write("\n   assign ")
                    f.write("out_" + str(x) +" = ")
                    for i in range (VarInput):
                        f.write("in_"+str(i))
                        if( i == 0):
                            f.write(" & ")
                    f.write(";")
                case 2:
                    f.write("\n   assign ")
                    f.write("out_" + str(x) +" = ")
                    for i in range (VarInput):
                        f.write("in_"+str(i))
                        if( i == 0):
                            f.write(" | ")
                    f.write(";")
                case _:
                    f.write("\n   assign ")
                    f.write("out_" + str(x) +" = ")
                    for i in range (VarInput):
                        f.write("in_"+str(i))
                        if( i == 0):
                            f.write(" ^ ")
                    f.write(";")
            operation1 = operation2
        f.write("\nendmodule\n\n")

        VarInput = 2 * moduleNum  
        VarOutput = 2 * moduleNum
        f.write("module MultiModuleOneLevel(")
        for x in range(VarInput):
            f.write("in_" + str(x) + ",")
        for x in range (VarOutput):
            f.write("out_" + str(x))
            if(x < VarOutput - 1):
                f.write(",")
        f.write(");\n")
        for x in range(VarInput):
            if(varWidth == 0):
                f.write("   input wire in_" + str(x) + ";\n")
            else:
                f.write("   input [" + str(varWidth) + ":0] in_" + str(x) + ";\n")
        for x in range (VarOutput):
            if(varWidth == 0):
                f.write("   output wire out_" + str(x))
            else:
                f.write("   output [" + str(varWidth) + ":0] out_" + str(x) + ";\n")
            #if(x < VarOutput):
        f.write("\n")
        #for x in range (VarOutput):
        #    if varWidth == 0:
        #        f.write("out_" + str(x) +" = ")
        #    else:
        #        f.write("   output [" + str(varWidth) + ":0] in_" + str(x) + ";\n")
        inNum = 0
        outNum = 0
        for numM in range (moduleNum):
            f.write("   S s" + str(numM) + "(in_" + str(inNum))
            inNum = inNum  + 1
            f.write(", in_" + str(inNum) )
            inNum = inNum +1
            f.write(", out_" + str(outNum) )
            outNum = outNum + 1
            f.write( ", out_" + str((outNum)) )
            outNum = outNum + 1
            f.write(")\n")

        f.write("\n endmodule")

    case 3:

        scale = 0     
        #VarInput = 2 * ModuleLevel  
        #VarOutput = 2 * ModuleLevel
        endVarInput = 2
        endVarOutput = 2
        operation1 = 1
        
        f.write("module M" + str(moduleNum) + " (")
        for x in range(endVarInput):
            f.write("in_" + str(x) + ",")
        for x in range (endVarOutput):
            f.write("out_" + str(x))
            if(x < endVarOutput - 1):
                f.write(",")
        f.write(");\n")
        for x in range(endVarInput):
            if varWidth == 0:
                f.write("   input wire in_" + str(x) + ";\n")
            else:
                f.write("   input [" + str(varWidth) + ":0] in_" + str(x) + ";\n")
        for x in range (endVarOutput):
            if varWidth == 0:
                f.write("   output wire out_" + str(x) + ";\n")
            else:
                f.write("   output [" + str(varWidth) + ":0] out_" + str(x) + ";\n")
        
        #for x in range (endVarOutput):
        #    f.write(" ========= assign out_" + str(x) + "= w_" + str(x) + ";\n")
        

        for x in range (endVarOutput):
            
            match operation1:
                case 1:
                    f.write("\n   assign ")
                    f.write("out_" + str(x) +" = ")
                    for i in range (endVarInput):
                        f.write("in_"+str(i))
                        if( i == 0):
                            f.write(" " + opList[1] + " ")
                    f.write(";")
                case 2:
                    f.write("\n   assign ")
                    f.write("out_" + str(x) +" = ")
                    for i in range (endVarInput):
                        f.write("in_"+str(i))
                        if( i == 0):
                            f.write(" " + opList[2] + " ")
                    f.write(";")
                case _:
                    f.write("\n   assign ")
                    f.write("out_" + str(x) +" = ")
                    for i in range (endVarInput):
                        f.write("in_"+str(i))
                        if( i == 0):
                            f.write(" ^ ")
                    f.write(";")
            operation1 = operation2
        f.write("\nendmodule\n\n")

        for mod in range (moduleNum):
            if(mod == (moduleNum -1)):
                f.write("\n")
                mux2to1(VarInput, VarOutput -1, varWidth, f)
                f.write("\n")
            modNum = moduleNum - mod -1
            f.write("\n\nmodule M" + str(modNum) + " (")
            for x in range(VarInput):
                f.write("in_" + str(x) + ",")
            for x in range (VarOutput -1):
                f.write("out_" + str(x))
                if(x < VarOutput - 2):
                    f.write(",")
            f.write(");\n")
            for x in range(VarInput):
                if(varWidth == 0):
                    f.write("   input wire in_" + str(x) + ";\n")
                else:
                    f.write("   input [" + str(varWidth) + ":0] in_" + str(x) + ";\n")
            for x in range (VarOutput-1):
                if(varWidth == 0):
                    f.write("   output wire out_" + str(x))
                else:
                    f.write("   output [" + str(varWidth) + ":0] out_" + str(x) + ";\n")
            f.write("\n")
            #if(x < VarOutput):
            #for x in range ((VarInput * 2) + VarOutput - 2):
            #    if (x != VarOutput -1):
            #        f.write("\t=======555555555555wire [" + str(varWidth) + ":0] w_" + str(x) + ";\n")
            #f.write("\n")
            opListNum = 0


            
            inNum = 0
            outNum = 0
            cnt = VarOutput;
            cnt1 = 0
            if(modNum == 1):
                for x in range ((VarInput * 2) + VarOutput + 2):
                    f.write("\twire [" + str(varWidth) + ":0] w_" + str(x) + ";\n")

                f.write("\n")
                cnt = ((VarInput * 2) + VarOutput - 3)
                numx = 0
                for x in range (VarOutput -1):
                    if(opListNum > 2):
                        opListNum = 0
                    f.write("\tassign w_" + str(cnt) + " = w_" + str(numx) + " " + opList[opListNum] + " w_" + str(numx + 1) + ";\n")
                    opListNum = opListNum + 1
                    cnt = cnt + 1
                    numx = numx + 2
                
                for x in range (VarOutput -1):
                    if(opListNum > 2):
                        opListNum = 0
                    f.write("\tassign out_" + str(x) + " = w_" + str(numx) + " " + opList[opListNum] + " w_" + str(numx + VarOutput -1) + ";\n")
                    opListNum = opListNum + 1
                    if(opListNum > 2):
                        opListNum = 0
                    cnt = cnt + 1
                    numx = numx + 1
            
            elif(modNum == 0):
                for x in range ((VarInput * 2) + VarOutput - 2):
                    #if (x != VarOutput -1):
                        f.write("\twire [" + str(varWidth) + ":0] w_" + str(x) + ";\n")
                f.write("\n")

                for x in range (VarOutput -1):
                    if(opListNum > 2):
                        opListNum = 0
                    f.write("\tassign out_" + str(x) + " =  w_" + str(x) + ";\n")
                    opListNum = opListNum + 1

                numx = VarOutput
                for x in range (VarOutput -1):
                    if(opListNum > 2):
                        opListNum = 0
                    f.write("\tassign w_" + str(x) + " = w_" + str(x) + " " + opList[opListNum] + " w_" + str(x + 1) + ";\n")
                    opListNum = opListNum + 1
                    numx = numx + 2
                
                f.write("\n")


            else:
                #if(modNum != moduleNum - 1):
                    for x in range (VarInput):
                        #if (x != VarOutput -1):
                        f.write("\twire [" + str(varWidth) + ":0] w_" + str(x) + ";\n")
                    f.write("\n")
                    numx = 0
                    for x in range (VarOutput -1):
                        if(opListNum > 2):
                            opListNum = 0
                        f.write("\tassign out_" + str(x) + " = w_" + str(numx) + " " + opList[opListNum] + " w_" + str(numx + 1) + ";\n")
                        opListNum = opListNum + 1
                        numx = numx + 1
                    
                    f.write("\n")
            numx = 0
            numx2 = 0
            cntNum = 0
            for numM in range (moduleNum):
                if(modNum == 1):
                    f.write("   M" + str(modNum +1) + " m" + str(numM) + "(")
                    for i in range (VarInput):
                        f.write("in_" + str(i) + ",")
                    for i in range (VarOutput ):
                        f.write("w_" + str(cnt1) )
                        cnt1 = cnt1 + 1
                        if(i < (VarOutput - 1)):
                            f.write(",")
                    #inNum = inNum  + 1
                    #f.write(", in_" + str(inNum) )
                    #inNum = inNum +1
                    #f.write(", out_" + str(outNum) )
                    #outNum = outNum + 1
                    #f.write( ", out_" + str((outNum)) )
                    #outNum = outNum + 1
                    f.write(");\n")
                    #f.write("\n")
                    #if(numM == 2):
                    #    mux2to1(VarInput, VarOutput, varWidth, f)
                    #    f.write("\n")
                elif(modNum == 0):
                    
                    if(numM<1):
                        f.write("   M" + str(modNum +1) + " m" + str(cntNum) + "(")
                        for i in range (VarOutput):
                            f.write("in_" + str(i) + ",")
                            #if(i < (VarInput - 2)):
                            #    f.write(",")
                        
                        for i in range (VarInput -1):
                            f.write("w_" + str(cnt))
                            cnt = cnt + 1
                            if(i < (VarInput - 2)):
                                f.write(",")
                        f.write(");" + "\n")
                        cntNum = cntNum + 1
                        f.write("   M" + str(modNum +1) + " m" + str(cntNum) + "(")
                        for i in range (VarOutput):
                            f.write("in_" + str(i) + ",")
                            #if(i < (VarInput - 2)):
                            #    f.write(",")
                        
                        for i in range (VarInput -1):
                            f.write("w_" + str(cnt))
                            cnt = cnt + 1
                            if(i < (VarInput - 2)):
                                f.write(",")
                        f.write(");")
                        cntNum = cntNum + 1
                    else:
                        
                        f.write("   mux2to1 " + " m" + str(cntNum) + "(")
                        #f.write("in_" + str(VarOutput - 1) + ",")
                        if(i <= 2):
                            for i in range (2):
                                f.write("in_" + str(i) + ",")
                            f.write("w_0" + ",")
                            f.write("out_" + str(VarOutput - 2) )
                        else:
                            for i in range (VarInput - 1):
                                f.write("w_" + str(i) + ",")
                        #for i in range (VarOutput):
                        #f.write("out_" + str(VarOutput - 2) )
                        #if(i < (VarInput - 1)):
                        #f.write(",")
                        f.write(");\n")
                    f.write("\n")
                else:
                    f.write("   M" + str(modNum +1) + " m" + str(numM) + "(")
                    if(modNum == moduleNum - 1):
                        
                        for i in range (moduleNum - 1):
                            f.write("in_" + str(numx) + ",")
                            numx = numx + 1
                        for i in range (endVarOutput):
                            f.write("w_" + str(numx2))
                            numx2 = numx2 + 1
                            if(i < (endVarOutput - 1)):
                                f.write(",")
                        f.write(");\n")
                    else:    
                        for i in range (VarOutput):
                            f.write("out_" + str(i) + ",")
                        for i in range (VarInput):
                            if(i == VarInput - 2):
                                f.write("in_" + str(i + 1))
                            else:
                                f.write("in_" + str(i))
                            if(i < (VarInput - 1)):
                                f.write(",")
                        f.write(");\n")
                #if(mod == 0):
            f.write("\nendmodule\n")
        
        
        TestBench(VarInput, VarOutput -1, varWidth,fileName)
        Docu(VarInput, VarOutput, varWidth, ModuleLevel, f)
#f.write("\n endmodule")


f.close()

 