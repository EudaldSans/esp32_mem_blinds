#!/usr/bin/env python3

import os
import sys
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("offset", help="position offset", nargs=1, metavar="offset")
parser.add_argument("in_file", help="original file", nargs=1, metavar="input_file")
parser.add_argument("out_file", help="output file with checksum", nargs=1, metavar="output_file")
parser.add_argument("-v", "--version", help="show program version and exit", action='version', version='%(prog)s 0.1')
args = parser.parse_args()


try:
    ifile = open(args.in_file[0], "r+b")
except:
    print(f"Error opening file '{args.in_file[0]}'")
    sys.exit(1)

# check offset
pos = args.offset[0]
if pos.startswith("0x") or pos.isnumeric():
    pos = int(pos, 0)
max_pos = os.stat(args.in_file[0]).st_size - 1
if pos > max_pos:
    print(f"Max posible offset: {max_pos} [{hex(max_pos)}]")
    sys.exit(1)

if os.path.exists(args.out_file[0]):
    print(f"File '{args.out_file[0]}' already exist")
    while True:
        o = input("Do you want to overwrite it? [Y/n] ")
        if o in ("n", "N"):
            sys.exit(1)
        elif o in ("y", "Y", ""):
            break

try:
    ofile = open(args.out_file[0], "wb")
except:
    print(f"Error creating file '{args.out_file[0]}'")
    sys.exit(1)

checksum = 0
size = 0

# write 0 to size and checksum
ifile.seek(pos)
ifile.write(size.to_bytes(4, "little"))
ifile.write(checksum.to_bytes(4, "little"))
ifile.seek(0)

# write output file and calculate checksum and size
for b in ifile.read():
    ofile.write(b.to_bytes(1, "little"))
    checksum += b
    size += 1

# write size and checksum
ofile.seek(pos)
ofile.write(size.to_bytes(4, "little"))
ofile.write(checksum.to_bytes(4, "little"))

ifile.close()
ofile.close()
