#!/usr/bin/env python3

import os
import sys
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("file", help="file to modify", nargs=1, metavar="file")
parser.add_argument("position", help="position to write", nargs=1, metavar="position")
parser.add_argument("value", help="value to write (0-255 [0x0-0xff])", nargs=1, metavar="value")
parser.add_argument("-v", "--version", help="show program version and exit", action='version', version='%(prog)s 0.1')
args = parser.parse_args()


if not args.file and not args.position and not args.value:
    parser.print_help()
    sys.exit(1)

# check position
pos = args.position[0]
if pos.startswith("0x") or pos.isnumeric():
    pos = int(pos, 0)
max_pos = os.stat(args.file[0]).st_size - 1
if pos > max_pos:
    print(f"Max posible position: {max_pos} [{hex(max_pos)}]")
    sys.exit(1)

# check value
val = args.value[0]
if val.startswith("0x") or val.isnumeric():
    val = int(val, 0)
    if val > 255:
        print("Max posible value is 255 [0xff]")
        sys.exit(1)
else:
    parser.print_help()
    sys.exit(1)

try:
    f = open(args.file[0], "r+b")
    f.seek(pos)
    old_val = int.from_bytes(f.read(1), "little")
    f.seek(pos)
    f.write(val.to_bytes(1, "little"))
    f.close()
except:
    print(f"Error updating file '{args.file[0]}'")
    sys.exit(1)
