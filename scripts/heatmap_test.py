import numpy as np
import seaborn as sns
import matplotlib.pylab as plt
import json

# Load the data from the file
with open('../ducode/build/src/results_fullres.json') as f:
    data = json.load(f)

arr = []
nd_arr = []
inputs = []
outputs = []

for nr in data["cnt_map"]["co_add1_1"]:
    inputs.append(nr)

for obj in data["cnt_map"]:
    outputs.append(obj)
    for nr in data["cnt_map"][obj]:
        arr.append(data["cnt_map"][obj][nr])
    arr
    nd_arr.append(arr.copy())
    arr.clear()

np_data = np.array(nd_arr)

uniform_data = np.random.rand(10, 12)
ax = sns.heatmap(np_data, linewidth=0.5, xticklabels=inputs, yticklabels=outputs, annot=True, fmt="d", cmap="Purples")
plt.show()