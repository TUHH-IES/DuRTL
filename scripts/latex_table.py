import json

f = open("latex_table.tex", "w")

designs = ["SPI_Master", "SPI_Slave", "y86", "yadmc", "aes_128", "usb2uart", "openMSP430"]
results_full_name = "results_full_"
results_rand_name = "results_random_"
results_rand25_ending = "_25.json"
results_rand50_ending = "_50.json"
results_rand75_ending = "_75.json"
json_ending = ".json"

f.write("\\begin{table} \caption{The numbers of cells and nets for the designs.} \n")
f.write("\centering \n")
f.write("\setlength{\\tabcolsep}{8pt} \n")
f.write("\\begin{tabularx}{\linewidth}{Xccccc} \n")
f.write("\\toprule\n")
f.write("Design & \# cells & \# nets & \# inputs & \# outputs \\\\ \n")
f.write("\midrule \n")
for design in designs:

    full_results = open('../build/src/results/' + results_full_name + design + json_ending)
    full_results_data = json.load(full_results)
    f.write("    \\textit{" + design.replace("_", "\_") + "} & " + str(full_results_data[design]['cells']) + " & " + str(full_results_data[design]['nets']) + " & " + str(full_results_data[design]['inputs'])  + " & " + str(full_results_data[design]['outputs']) +" \\\\ \n")
    full_results.close()
f.write("    \\bottomrule \n")
f.write("\end{tabularx} \n")
f.write("\label{tab:design_info} \n")
f.write("\end{table} \n\n")


f.write("\\begin{table*}[t]\n") 
f.write("    \caption{IFT results for the lengths of identified paths}\n")
f.write("    \label{tab:results}\n")
f.write("    \centering\n")
f.write("    \setlength{\\tabcolsep}{8pt} \n")
f.write("    \\begin{tabularx}{0.8\linewidth}{Xccccccccccc} \n")
f.write("        \\toprule\n")
f.write("         & & \multicolumn{2}{c}{25\% tags}  & \multicolumn{2}{c}{50\% tags}  & \multicolumn{2}{c}{75\% tags} & \multicolumn{2}{c}{100\% tags} & \multicolumn{2}{c}{toggle} \\\\ \n")
f.write("     \cmidrule(lr){3-4}\n")
f.write("     \cmidrule(lr){5-6}\n")
f.write("     \cmidrule(lr){7-8}\n")
f.write("     \cmidrule(lr){9-10}\n")
f.write("     \cmidrule(lr){11-12}\n")

f.write("    Design & \#tags & most & least & most & least & most & least & most & least & most & least \\\\ \n") 
f.write("    \midrule \n")

for design in designs:

    full_results = open('../build/src/results/' + results_full_name + design + json_ending)
    full_results_data = json.load(full_results)
    rand_25_results = open('../build/src/results/' + results_rand_name + design + results_rand25_ending)
    rand_25_results_data = json.load(rand_25_results)
    rand_50_results = open('../build/src/results/' + results_rand_name + design + results_rand50_ending)
    rand_50_results_data = json.load(rand_50_results)
    rand_75_results = open('../build/src/results/' + results_rand_name + design + results_rand75_ending)
    rand_75_results_data = json.load(rand_75_results)
    f.write("    \\textit{" + design.replace("_", "\_") + "} & " + str(full_results_data[design]['number_of_injected_tags']) + " & " + str(rand_25_results_data[design]['path(most)_size']) + " & " + str(rand_25_results_data[design]['path(least)_size']) + " & " + str(rand_50_results_data[design]['path(most)_size']) + " & " + str(rand_50_results_data[design]['path(least)_size']) + " & " + str(rand_75_results_data[design]['path(most)_size']) + " & " + str(rand_75_results_data[design]['path(least)_size']) + " & " + str(full_results_data[design]['path(most)_size']) + " & " + str(full_results_data[design]['path(least)_size']) + " & " + str(full_results_data[design]['path(most_toggled)_size']) + " & " + str(full_results_data[design]['path(least_toggled)_size']) +" \\\\ \n")
    full_results.close()
    rand_25_results.close()
    rand_50_results.close()
    rand_75_results.close()
    
f.write("    \\bottomrule\n")
f.write("    \end{tabularx}\n")
f.write("\end{table*}\n")

# second table
f.write("\n")

f.write("\\begin{table*}[t] \n")
f.write("\caption{IFT results for the number of tags reaching 1 or all outputs} \n")
f.write("\label{tab:results2} \n")
f.write("\centering \n")
f.write("\setlength{\\tabcolsep}{8pt} \n")
f.write("\\begin{tabularx}{0.8\linewidth}{Xcccccccc} \n")
f.write("\\toprule \n")
f.write("& \multicolumn{2}{c}{25\% tags}  & \multicolumn{2}{c}{50\% tags}  & \multicolumn{2}{c}{75\% tags} & \multicolumn{2}{c}{100\% tags} \\\\ \n")
f.write("    \cmidrule(lr){2-3} \n")
f.write("\cmidrule(lr){4-5} \n")
f.write("\cmidrule(lr){6-7} \n")
f.write("\cmidrule(lr){8-9} \n")

f.write("Design & 1o & No & 1o & No & 1o & No & 1o & No \ \\\\ \n")
f.write("    \midrule \n")

def get_tag_count_range(data):
    result = ""
    min_value = 10000000
    max_value = 0
    for attribute, value in data.items():
        if attribute == "total_tag_count":
            continue
        if value < min_value:
            min_value = value
        if value > max_value:
            max_value = value
    return str(min_value) + "-" + str(max_value)


for design in designs:
    full_results = open('../build/src/results/' + results_full_name + design + json_ending)
    full_results_data = json.load(full_results)
    rand_25_results = open('../build/src/results/' + results_rand_name + design + results_rand25_ending)
    rand_25_results_data = json.load(rand_25_results)
    rand_50_results = open('../build/src/results/' + results_rand_name + design + results_rand50_ending)
    rand_50_results_data = json.load(rand_50_results)
    rand_75_results = open('../build/src/results/' + results_rand_name + design + results_rand75_ending)
    rand_75_results_data = json.load(rand_75_results)
    f.write("    \\textit{" + design.replace("_", "\_") + "} & " + str(get_tag_count_range(rand_25_results_data[design]['output_tag_count'])) + " & " + str(rand_25_results_data[design]['output_tag_count']['total_tag_count']) + " & " + str(get_tag_count_range(rand_50_results_data[design]['output_tag_count'])) + " & " + str(rand_50_results_data[design]['output_tag_count']['total_tag_count']) + " & " + str(get_tag_count_range(rand_75_results_data[design]['output_tag_count'])) + " & " + str(rand_75_results_data[design]['output_tag_count']['total_tag_count']) + " & " + str(get_tag_count_range(full_results_data[design]['output_tag_count'])) + " & " + str(full_results_data[design]['output_tag_count']['total_tag_count']) + " \\\\ \n")
    full_results.close()
    rand_25_results.close()
    rand_50_results.close()
    rand_75_results.close()

f.write("    \\bottomrule \n")
f.write("\end{tabularx} \n")
f.write("\end{table*} \n\n")

f.write("\\begin{table} \n")
f.write("\caption{Runtimes for each design (total \#tags in \%) [\si{\second}]} \n")
f.write("\label{tab:runtimes} \n")
f.write("\centering \n")
f.write("\\begin{tabularx}{\linewidth}{Xrrrr} \n")
f.write("\\toprule \n")
f.write("Design & \SI{25}{\percent} tags & \SI{50}{\percent} tags & \SI{75}{\percent} tags & \SI{100}{\percent} tags \ \\\\ \n")
f.write("    \midrule \n")

for design in designs:

    full_results = open('../build/src/results/' + results_full_name + design + json_ending)
    full_results_data = json.load(full_results)
    rand_25_results = open('../build/src/results/' + results_rand_name + design + results_rand25_ending)
    rand_25_results_data = json.load(rand_25_results)
    rand_50_results = open('../build/src/results/' + results_rand_name + design + results_rand50_ending)
    rand_50_results_data = json.load(rand_50_results)
    rand_75_results = open('../build/src/results/' + results_rand_name + design + results_rand75_ending)
    rand_75_results_data = json.load(rand_75_results)
    f.write("    \\textit{" + design.replace("_", "\_") + "} & " + str(rand_25_results_data['overall_runtime']) + " & " + str(rand_50_results_data['overall_runtime']) + " & " + str(rand_75_results_data['overall_runtime']) + " & " + str(full_results_data['overall_runtime']) +" \\\\ \n")
    full_results.close()
    rand_25_results.close()
    rand_50_results.close()
    rand_75_results.close()

f.write("    \\bottomrule \n")
f.write("\end{tabularx} \n")
f.write("\\vspace{-5pt} \n")
f.write("\end{table} \n")

f.close()
