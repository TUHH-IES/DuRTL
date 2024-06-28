def TestBench(VarInput, VarOutput, varWidth,fileName):
# VarInput = 3
# VarOutput = 3
# varWidth = 4
# fileName = "ScalableModule"
    f = open(fileName + "_tb.v", "w")

    f.write("module ScalableModule_tb;\n")
    strInput = "reg [" + str(varWidth) + ":0]"
    for x in range(VarInput):
        strInput += " in_" + str(x)
        if(x < VarInput - 1):
            strInput += ","
    f.write(strInput)
    strOutput = ";\noutput [" + str(varWidth) + ":0]"
    for x in range(VarOutput):
        strOutput += "out_" + str(x)
        if(x < VarOutput - 1):
            strOutput +=","
        else:
            strOutput += ";\n"
    f.write(strOutput)

    strInput = "M0 test("
    for x in range(VarInput):
        strInput +=  "in_" + str(x)
        if(x < VarInput):
            strInput += ","
    for x in range(VarOutput):
        strInput += "out_" + str(x)
        if(x < VarOutput - 1):
            strInput +=","
        else:
            strInput += ");\n"
    f.write(strInput)
    f.write("initial begin\n")
    strInit = ""
    numTime = 2
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            strInit += "0"
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) + "\n")
    numTime = numTime + 2
    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            if(i > (varWidth - (x + 4 + numTime))):
                strInit += "1"
            elif(x == VarInput - 3):
                strInit += "0"
            else:
                strInit += "0"
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) +"\n")
    numTime = numTime + 2


    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            if(x == VarInput - 3):
                strInit += "1"
            else:
                strInit += "0"
            
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) +"\n")
    numTime = numTime + 2

    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            if(i < (varWidth - 8)):
                strInit += "1"
            elif(x == VarInput - 3):
                strInit += "1"
            else:
                strInit += "0"
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) +"\n")
    numTime = numTime + 2

    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            if(i > (varWidth - 8)):
                strInit += "1"
            elif(x == VarInput - 3):
                strInit += "1"
            else:
                strInit += "0"
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) +"\n")
    numTime = numTime + 2

    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            if(i < (varWidth - 8)):
                strInit += "0"
            elif(x == VarInput - 3):
                strInit += "1"
            else:
                strInit += "0"
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) +"\n")
    numTime = numTime + 2


    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            if(i < (varWidth - 8 + x)):
                strInit += "1"
            elif(x == VarInput - 3):
                strInit += "1"
            else:
                strInit += "0"
        strInit += ";\n"
        f.write(strInit)
    f.write("#" + str(numTime) +"\n")
    numTime = numTime + 2


    strInit = ""
    for x in range(VarInput):
        strInit = ""
        strInit += "in_" + str(x) + " = " + str(varWidth) + "'b";
        for i in range(varWidth):
            strInit += "1"
        strInit += ";\n"
        f.write(strInit)
    f.write("end" +"\n")

    f.write("initial\n")
    f.write("begin\n")
    f.write("\t" +	"$dumpfile(\"../../examples/Scalable/" + fileName + ".vcd\");\n");  
    f.write("\t$dumpvars();\n")
    f.write("end\n")
    f.write("endmodule\n")


