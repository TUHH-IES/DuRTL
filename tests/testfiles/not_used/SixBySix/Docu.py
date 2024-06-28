def Docu(VarInput, VarOutput, varWidth, hierLevel, f):
    fileName = "SixBySixOutput.tex"
    f = open(fileName +".v", "w")
    f.write("\begin{table}[H]")

    f.write("    \begin{center}")
    f.write("    \begin{tabular}{l|c|r}")
    f.write("        \hline")
    f.write("        \multicolumn{2}{c}{Sample Circuit}\\")
    f.write("        \hline")
    f.write("        Functional Units Included& AND OR ADDER MUX\\")
    f.write("        Number of Inputs & " + str(VarInput) + "\\")
    f.write("        Number of Bits Per Input & " + str(varWidth) + "\\")
    f.write("        Number of Levels of Hierarchy & " + str(hierLevel) + "\\")
    f.write("        \hline")
    f.write("    \end{tabular}")
    f.write("    \end{center}")
    f.write("\label{tab:multicol}")
    f.write("\end{table}")