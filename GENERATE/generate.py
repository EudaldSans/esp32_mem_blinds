#!/usr/bin/env python3

import os
import sys
import argparse
import subprocess
import shutil


FILE_ROMFS = "package.rfs"

DIR_BOOT = "boot/"
DIR_APPS = "apps/"
DIR_TMP = "tmp/"
DIR_BIN = "bin/"

APP_PYTHON = "python"
APP_CHECKSUM = "checksum.py"
APP_CRC32 = "crc32.py"
APP_ROMFS = "romfs.py"
APP_SETBYTE = "setbyte.py"


parser = argparse.ArgumentParser()
parser.add_argument("cfg_file", help="configuration file", nargs=1, metavar="config.txt")
parser.add_argument("zip_file", help="name of output zip file", nargs=1, metavar="output")
parser.add_argument("-c", help="checksum to use (default SUM)", nargs=1, metavar="SUM/CRC32", default=["SUM"], dest="chk")
parser.add_argument("-v", "--version", help="show program version and exit", action='version', version='%(prog)s 0.6')
args = parser.parse_args()


def remove_old_files():
    """
    Remove files created in other runs.
    """
    if not os.path.exists(DIR_TMP):
        os.mkdir(DIR_TMP)
    for file in os.listdir(DIR_TMP):
        os.remove(DIR_TMP + file)
    if os.path.exists(args.zip_file[0] + ".zip"):
        os.remove(args.zip_file[0] + ".zip")

def finish(value):
    """
    Ends the program with a exit code.

    Arguments:\n
    value -- int value for exit code (default 0: OK)
    """
    if value:
        remove_old_files()
    else:
        print(f"{args.zip_file[0]}.zip generated successfully")
    sys.exit(value)

version = "0"
esp_boot_file = ""
esp_app_file = ""

# try to open input file
try:
    text_file = open(args.cfg_file[0], "r")
except:
    print(f"Error opening file: '{args.cfg_file[0]}'")
    finish(1)

# remove posible zip extension
if str(args.zip_file[0]).endswith(".zip"):
    args.zip_file[0] = args.zip_file[0].replace(".zip", "")

remove_old_files()

# try to create sw file
try:
    sw_file = open(DIR_TMP + "sw.tmp", "wb")
except:
    print(f"Error creating file: '{DIR_TMP}sw.tmp'")
    finish(1)

# try to create hwconfig file
try:
    hwconfig_file = open(DIR_TMP + "hwconfig.tmp", "wb")
except:
    print(f"Error creating file: '{DIR_TMP}hwconfig.tmp'")
    finish(1)

# number of lines in input file
lines = sum(1 for _ in text_file)

text_file.seek(0)
apps = []

# read input file and create boot and hwconfig files
for i in range(lines):
    line = text_file.readline().replace(" ", "").replace("\n", "").split(",")

    # skip comments and empty lines
    if line[0].startswith("#") or not line[0]:
        continue

    # read version
    if line[0].startswith("version"):
        version = line[0].split("=")[1]
        # write temp sw file
        sw_file.write(version.encode())
        continue

    # read ESP32 files
    if line[0].startswith("ESP32_boot"):
        esp_boot_file = line[0].split("=")[1]
        continue
    if line[0].startswith("ESP32_app"):
        esp_app_file = line[0].split("=")[1]
        continue

    # check line parameters
    if len(line) != 8:
        print(f"Error reading config file '{args.cfg_file[0]}' at line {i + 1}")
        text_file.close()
        sw_file.close()
        hwconfig_file.close()
        finish(1)

    port = int(line[0])
    addr = int(line[1])
    name = bytes(line[2], "utf-8")
    name14 = bytearray(14 - len(name))
    name14[0:0] = name
    boot = line[3]
    address_offset = line[4]
    boot_delay = line[5]
    boot_delay_offset = hex(int(address_offset, 0) + 1)
    apps.append(name.decode())
    apps.append(line[6])
    checksum_offset = line[7]

    # copy boot file and set address and delay
    if os.path.exists(DIR_BOOT + boot):
        shutil.copy(DIR_BOOT + boot, DIR_TMP + "boot_" + name.decode() + ".bin")
    else:
        print(f"No such file: '{DIR_BOOT + boot}'")
        text_file.close()
        sw_file.close()
        hwconfig_file.close()
        finish(1)
    proc_args = [APP_PYTHON, DIR_BIN + APP_SETBYTE, DIR_TMP + "boot_" + name.decode() + ".bin", address_offset, str(addr)]
    if subprocess.run(proc_args).returncode:
        print(f"Error setting address {addr} in file {DIR_TMP}boot_{name.decode()}.bin with offset {address_offset}")
        text_file.close()
        sw_file.close()
        hwconfig_file.close()
        finish(1)
    proc_args = [APP_PYTHON, DIR_BIN + APP_SETBYTE, DIR_TMP + "boot_" + name.decode() + ".bin", boot_delay_offset, boot_delay]
    if subprocess.run(proc_args).returncode:
        print(f"Error setting delay {boot_delay} in file {DIR_TMP}boot_{name.decode()}.bin with offset {boot_delay_offset}")
        text_file.close()
        sw_file.close()
        hwconfig_file.close()
        finish(1)

    # write hwconfig file
    hwconfig_file.write(port.to_bytes(1, "little"))
    hwconfig_file.write(addr.to_bytes(1, "little"))
    hwconfig_file.write(name14)

    # set app checksum
    if os.path.exists(DIR_APPS + line[6]):
        shutil.copy(DIR_APPS + line[6], DIR_TMP + line[6])
    else:
        print(f"No such file: '{DIR_APPS + line[6]}'")
        text_file.close()
        hwconfig_file.close()
        finish(1)
    if args.chk[0] == "SUM":
        proc_args = [APP_PYTHON, DIR_BIN + APP_CHECKSUM, checksum_offset, DIR_TMP + line[6], DIR_TMP + line[6] + ".sum"]
    elif args.chk[0] == "CRC32":
        proc_args = [APP_PYTHON, DIR_BIN + APP_CRC32, checksum_offset, DIR_TMP + line[6], DIR_TMP + line[6] + ".sum"]
    else:
        print(f"{args.chk[0]} is not a valid checksum (CRC32 or SUM)")
        finish(1)
    if subprocess.run(proc_args).returncode:
        print(f"Error setting checksum in file {line[6]} (offset: {checksum_offset})")
        text_file.close()
        hwconfig_file.close()
        finish(1)


text_file.close()
sw_file.close()
hwconfig_file.close()

# copy ESP32 boot file
if os.path.exists(DIR_BOOT + esp_boot_file):
    shutil.copy(DIR_BOOT + esp_boot_file, DIR_TMP + "boot_" + esp_boot_file)
else:
    print(f"No such file: '{DIR_BOOT + esp_boot_file}'")
    finish(1)

# create romfs file
proc_args = [APP_PYTHON, DIR_BIN + APP_ROMFS, "-c", DIR_TMP + FILE_ROMFS]
if subprocess.run(proc_args).returncode:
    finish(1)

# add sw file to romfs
proc_args = [APP_PYTHON, DIR_BIN + APP_ROMFS, "-a", DIR_TMP + FILE_ROMFS, "SW", DIR_TMP + "sw.tmp"]
if subprocess.run(proc_args).returncode:
    finish(1)
os.remove(DIR_TMP + "sw.tmp")

# add hwconfig file to romfs
proc_args = [APP_PYTHON, DIR_BIN + APP_ROMFS, "-a", DIR_TMP + FILE_ROMFS, "HWCONFIG", DIR_TMP + "hwconfig.tmp"]
if subprocess.run(proc_args).returncode:
    finish(1)
os.remove(DIR_TMP + "hwconfig.tmp")

# add ESP32 app file to romfs
if not os.path.exists(DIR_APPS + esp_app_file):
    print(f"No such file: '{DIR_APPS + esp_app_file}'")
    finish(1)
proc_args = [APP_PYTHON, DIR_BIN + APP_ROMFS, "-a", DIR_TMP + FILE_ROMFS, "ESP32", DIR_APPS + esp_app_file]
if subprocess.run(proc_args).returncode:
    finish(1)

# add apps files to romfs
for i in range(0, int(len(apps)), 2):
    proc_args = [APP_PYTHON, DIR_BIN + APP_ROMFS, "-a", DIR_TMP + FILE_ROMFS, apps[i], DIR_TMP + apps[i + 1] + ".sum"]
    if subprocess.run(proc_args).returncode:
        finish(1)

    # append app file to boot file
    boot_file = open(DIR_TMP + "boot_" + apps[i] + ".bin", "ab")
    app_file = open(DIR_TMP + apps[i + 1] + ".sum", "rb")
    # add 0xFF until 4KB
    for j in range(4096 - os.stat(DIR_TMP + "boot_" + apps[i] + ".bin").st_size):
        boot_file.write(bytes([0xFF]))
    boot_file.write(app_file.read())
    boot_file.close()
    app_file.close()

    os.remove(DIR_TMP + apps[i + 1])
    os.remove(DIR_TMP + apps[i + 1] + ".sum")

# create zip file
shutil.make_archive(args.zip_file[0], "zip", DIR_TMP)

# delete tmp dir
shutil.rmtree(DIR_TMP)

finish(0)
