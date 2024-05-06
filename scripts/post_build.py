from shutil import copy2, copytree, rmtree
import os
from argparse import ArgumentParser, ArgumentError

copy_path = os.environ.get("FLHOOK_COPY_PATH")

bin_dir = None

if __name__ != "__main__":
    raise NotImplementedError("This file should not be imported")

parser = ArgumentParser()
parser.add_argument("--bin-dir", "-o", dest="bin", type=str, required=True)
args = parser.parse_args()

if not os.path.isdir(args.bin):
    raise ArgumentError(argument=args.bin, message="Provided Bin Directory was not a valid directory")

print("Copying files from {0} to {1}".format(args.bin, copy_path))
dirs = [d for d in os.listdir(args.bin)]
for src in dirs:
    joined = os.path.join(args.bin, src)
    dest = os.path.join(copy_path, src)
    print("Copying {0} to {1}".format(src, dest))
    if os.path.isdir(joined):
        copytree(joined, dest, False, None, dirs_exist_ok=True)
    else:
        copy2(joined, copy_path)
