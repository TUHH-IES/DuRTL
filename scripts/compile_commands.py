import json
import argparse
import re

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='compile_commands.json')
    parser.add_argument('filename')
    args = parser.parse_args()

    elements = []
    with open(args.filename, 'r') as file:
        data = json.load(file)
        for element in data:
            match = re.search("vcpkg_installed", element['file'])
            if not match:
                elements.append(element)

    with open(args.filename, 'w') as file:
        file.write(json.dumps(elements, indent=2))
