#!/usr/bin/env python3
import re
import urllib.request

url = "https://www.unicode.org/Public/UCD/latest/ucd/DerivedCoreProperties.txt"
source_txt = urllib.request.urlopen(url).read().decode("utf-8")

id_start = []
id_continue = []

for line in source_txt.splitlines():
    if '; ID_Start' in line:
        prog = re.compile(r'([0-9A-F]*)\.\.([0-9A-F]*)')
        match = prog.match(line)
        if match:
            c1 = int(match.group(1), 16)
            c2 = int(match.group(2), 16)
            if c2 > 128:  # Non-ASCII
                id_start += [[c1, c2]]
        else:
            prog = re.compile(r'([0-9A-F]*)')
            match = prog.match(line)
            if match:
                c1 = int(match.group(1), 16)
                if c1 > 128:  # Non-ASCII
                    id_start += [[c1, c1]]
            else:
                print("Error!!!")
    
    elif '; ID_Continue' in line:
        prog = re.compile(r'([0-9A-F]*)\.\.([0-9A-F]*)')
        match = prog.match(line)
        if match:
            c1 = int(match.group(1), 16)
            c2 = int(match.group(2), 16)
            if c2 > 128:  # Non-ASCII
                id_continue += [[c1, c2]]
        else:
            prog = re.compile(r'([0-9A-F]*)')
            match = prog.match(line)
            if match:
                c1 = int(match.group(1), 16)
                if c1 > 128:  # Non-ASCII
                    id_continue += [[c1, c1]]
            else:
                print("Error!!!")

destination = "#pragma once\n"
destination += "// Ths is a generated file. DO NOT CHANGE!\n\n"
destination += "#include <cstdint>\n\n"


destination += "uint32_t id_start_table[][2] = {\n"
for (c1, c2) in id_start:
    destination += "    {}, {},  //".format(c1, c2)
    for i in range(c1, c2 + 1):
        destination += chr(i)
    destination += "\n"
destination += "};\n"
destination += "size_t id_start_table_len = sizeof(id_start_table) / sizeof(id_start_table[0]);\n\n"

destination += "uint32_t id_continue_table[][2] = {\n"
for (c1, c2) in  id_continue:
    destination += "    {}, {},  //".format(c1, c2)
    for i in range(c1, c2 + 1):
        destination += chr(i)
    destination += "\n"
destination += "};\n"
destination += "size_t id_continue_table_len = sizeof(id_continue_table) / sizeof(id_continue_table[0]);\n\n"

with open('unicode_tables.h', 'w') as file:
    file.write(destination)
