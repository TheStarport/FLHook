from shutil import copy2
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
    if not src.endswith(".dll") or not os.path.isfile(joined):
        continue

    print("Copying {0} to {1}".format(src, joined))
    copy2(joined, copy_path)
