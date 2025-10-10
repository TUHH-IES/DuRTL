import json

f = open("latex_table.tex", "w")

designs = ["SPI_Master"]#, "y86"] #, "yadmc", "aes_128", "usb2uart", "openMSP430"]
results_name = "results_IFA_" # results_full_name + design + json_ending
json_ending = ".json"

results = open('../build/src/results/' + results_name + design + "_10" + json_ending)
results_data = json.load(results)

f.write("\\begin{table*}[t]\n") 
f.write("    \\caption{Minimum timesteps between input/output pairs (IFT/SIFA) for the SPI\_Primary design.}\n")
f.write("    \\label{tab:results_timesteps}\n")
f.write("    \\centering\n")
f.write("    \\setlength{\\tabcolsep}{8pt} \n")
f.write("    \\begin{tabularx}{0.8\\linewidth}{Xccc} \n")
f.write("        \\toprule\n")
f.write("    Design & IFT & IFA 5 & IFA 10 \\\\ \n") 
f.write("    \\midrule \n")

for design in designs:

    results = open('../build/src/results/' + results_name + design + "_10" + json_ending)
    results_data = json.load(results)
    f.write("    \\textit{" + design.replace("_", "\\_") + "} & " + str(results_data['IFs identified by IFT']) + " & " + str(results_data['UF:5']['IFA IFs identified']) + " & " + str(results_data['UF:10']['IFA IFs identified']) + " \\\\ \n")
    results.close()
    
f.write("    \\bottomrule\n")
f.write("    \\end{tabularx}\n")
f.write("\\end{table*}\n")

