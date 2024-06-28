import argparse


def main():
    parser = argparse.ArgumentParser(
        prog='ProgramName',
        description='What the program does',
        epilog='Text at the bottom of help')

    parser.add_argument('filename')  # positional argument
    parser.add_argument('-i', '--inplace', action='store_true')  # option that takes a value
    args = parser.parse_args()

    with open(args.filename) as file:
        lines = file.read().splitlines()
        for i, line in enumerate(lines):
            if "<<" in line and "format" not in line:
                lines[i] = parse_line(line.strip())
            else:
                lines[i] = line.strip()

    if args.inplace:
        with open(args.filename, "w") as file:
            for line in lines:
                file.write(f"{line}\n")
    else:
        for line in lines:
            print(f"{line}")


def parse_line(line):
    end_line = ""
    segments = line.split("<<")
    if len(segments) == 2:
        return line
    if line[-1] == ";":
        end_line = ";"
        line = line[:-1]
    segments = line.split("<<")
    new_line = segments[0] + "<< fmt::format(" + process_segnments(segments[1:]) + ")" + end_line
    return new_line


def process_segnments(segments):
    new_line = ""
    variables = []
    for i, segment in enumerate(segments):
        if "\"" in segment:
            new_line += segment.strip().replace('"', '')
        else:
            new_line += "{}"
            variables.append(segment.strip())
    return '"' + new_line + '", ' + ", ".join(variables)


if __name__ == '__main__':
    main()
