#!/usr/bin/env python3

import os
import sys
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("-c", help="create a new romfs file", nargs=1, metavar="file.rfs", dest="C")
parser.add_argument("-a", help="add to romfs the file with a name", nargs=3, metavar=("file.rfs", "name", "file.bin"), dest="A", action='append')
parser.add_argument("-l", help="list the content of the romfs file", nargs=1, metavar="file.rfs", dest="L")
parser.add_argument("-p", help="print the content of a file inside the romfs file", nargs=2, metavar=("file.rfs", "name"), dest="P")
parser.add_argument("-v", "--version", help="show program version and exit", action='version', version='%(prog)s 0.2')
args = parser.parse_args()


OK = 1
ERROR_FILE_READ = 2
ERROR_FILE_CHECKSUM = 3
ERROR_FILE_UNKNOW = 4


def main():
    """
    Main function.
    """
    if not args.C and not args.A and not args.L and not args.P:
        parser.print_help()
        sys.exit(1)

    if args.C:
        res = create_file(args.C[0])
        if res == OK:
            update_header(args.C[0])
        else:
            print(f"Error creating file '{args.C[0]}'")
            sys.exit(1)

    if args.A:
        for i in range(len(args.A)):
            res = add_file(args.A[i][0], args.A[i][1], args.A[i][2])
            if res == OK:
                update_header(args.A[i][0])
            elif res == ERROR_FILE_CHECKSUM:
                print(f"Bad checksum for '{args.A[i][0]}'")
                sys.exit(1)
            elif res == ERROR_FILE_READ:
                print(f"Error opening file '{args.A[i][0]}'")
                sys.exit(1)
            elif res == ERROR_FILE_UNKNOW:
                print(f"Unknow file '{args.A[i][0]}'")
                sys.exit(1)
            else:
                print(f"Error updating '{args.A[i][0]}'")
                sys.exit(1)

    if args.L:
        res = list_files(args.L[0])
        if res == ERROR_FILE_CHECKSUM:
            print(f"Bad checksum for '{args.L[0]}'")
            sys.exit(1)
        elif res == ERROR_FILE_READ:
            print(f"Error opening file '{args.L[0]}'")
            sys.exit(1)
        elif res == ERROR_FILE_UNKNOW:
            print(f"Unknow file '{args.L[0]}'")
            sys.exit(1)

    if args.P:
        res = print_file(args.P[0], args.P[1])
        if res == ERROR_FILE_READ:
            print(f"Error opening file '{args.P[0]}'")
            sys.exit(1)
        elif res == ERROR_FILE_UNKNOW:
            print(f"Unknow file '{args.P[1]}'")
            sys.exit(1)


def create_file(romfile):
    """
    Creates a void romfs file.

    Arguments:\n
    romfile -- file to be created
    """
    if os.path.exists(romfile):
        print(f"File '{romfile}' already exist")
        while True:
            o = input("Do you want to overwrite it? [Y/n] ")
            if o in ("n", "N"):
                sys.exit(1)
            elif o in ("y", "Y", ""):
                break

    h_head = bytearray("rfs-0.00", "utf-8")
    h_size = bytearray(4)
    h_checksum = bytearray(4)
    h_reserved = bytearray(16)

    try:
        f = open(romfile, "wb")
        f.write(h_head + h_size + h_checksum + h_reserved)
        f.close()
    except:
        return ERROR_FILE_READ

    return OK


def add_file(romfile, addname, addfile):
    """
    Adds a file to a romfs file.

    Arguments:\n
    romfile -- already created romfs file\n
    addname -- name for the file to add\n
    addfile -- file to add
    """
    res = check_file(romfile)
    if res != OK:
        return res

    h_name = bytearray(12)
    count = 0
    for b in bytearray(addname, "utf-8"):
        h_name[count] = b
        count += 1
        if count == 12:
            break

    if os.path.exists(addfile):
        size = os.stat(addfile).st_size
    else:
        print(f"No such file: '{addfile}'")
        return -1

    try:
        romf = open(romfile, "ab")
        addf = open(addfile, "rb")
        romf.write(h_name)
        romf.write(size.to_bytes(4, "little"))
        romf.write(addf.read())
        addf.close()
        romf.close()
    except:
        return ERROR_FILE_READ

    return OK


def list_files(romfile):
    """
    Show the content of a romfs file.

    Arguments:\n
    romfile -- file to read
    """
    res = check_file(romfile)
    if res != OK:
        return res

    try:
        f = open(romfile, "rb")
        f.seek(8)
        total_size = int.from_bytes(f.read(4), "little")

        n = 0
        while n < total_size:
            f.seek(32 + n)
            name = f.read(12).decode().split("\x00")[0]
            size = int.from_bytes(f.read(4), "little")
            print(f"{name}: {size} bytes")
            n += size + 16

        f.close()

        if n == 0:
            print("Nothing found")

    except:
        return ERROR_FILE_READ

    return OK


def update_header(romfile):
    """
    Update the header of a romfs file.

    Arguments:\n
    romfile -- file to update
    """
    f = open(romfile, "r+b")

    f.seek(8)
    fsize = os.stat(romfile).st_size - 32
    f.write(fsize.to_bytes(4, "little"))

    sumc = 0
    f.seek(12)
    f.write(sumc.to_bytes(4, "little"))

    sumc = get_sum(f)
    f.seek(12)
    f.write(sumc.to_bytes(4, "little"))

    f.close()


def check_file(romfile):
    """
    Checks the 'checksum' of a romfs file.

    Arguments:\n
    romfile -- file to check
    """
    try:
        f = open(romfile, "r+b")
        head = f.read(8).decode("utf-8")
        if not head == "rfs-0.00":
            return ERROR_FILE_UNKNOW

        f.seek(12)
        file_sum = int.from_bytes(f.read(4), "little")

        sumc = 0
        f.seek(12)
        f.write(sumc.to_bytes(4, "little"))
        sumc = get_sum(f)

        f.seek(12)
        f.write(file_sum.to_bytes(4, "little"))

        f.close()

        if file_sum == sumc:
            return OK

    except:
        return ERROR_FILE_READ

    return ERROR_FILE_CHECKSUM


def get_sum(file):
    """
    Returns the checksum of a file.

    Arguments:\n
    file -- file to read
    """
    sumc = 0
    file.seek(0)
    for i in file.read():
        sumc += i
    return sumc


def print_file(romfile, filename):
    """
    Prints in console the content of a file inside the romfs file.

    Arguments:\n
    romfile -- romfs file to read
    filename -- filename inside romfs to read
    """
    try:
        f = open(romfile, "rb")
        f.seek(8)
        total_size = int.from_bytes(f.read(4), "little")

        n = 0
        while n < total_size:
            f.seek(32 + n)
            name = f.read(12).decode().split("\x00")[0]
            size = int.from_bytes(f.read(4), "little")
            if filename == name:
                print(f.read(size).decode("utf-8", "ignore"))
                return OK
            n += size + 16
        f.close()
        print(f"'{filename}' not found")
        return OK
    except:
        return ERROR_FILE_READ


main()
