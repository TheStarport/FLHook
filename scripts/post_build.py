from glob import glob
from shutil import copy2, copytree, rmtree, make_archive
import os
from argparse import ArgumentParser, ArgumentError


def post_build(release: bool, dest: str | None):
    # noinspection PyBroadException
    try:
        if not os.path.isdir('./build'):
            print('Build directory not present. Run the build first.')
            return

        if os.path.isdir('./dist'):
            rmtree('./dist')

        os.makedirs('./dist/plugins', exist_ok=True)

        # Dll Files
        result = [y for x in os.walk('./build') for y in glob(os.path.join(x[0], '*.dll'))]
        for dll in result:
            if (release and 'Debug' in dll) or (not release and 'Release' in dll):
                continue

            dist = './dist/' if 'plugin' not in dll else './dist/plugins/'
            copy2(dll, dist)
            print(f'copied {dll} to {dist}')

        print('Zipping up dist folder to build.zip')
        make_archive('build', 'zip', './dist')

        if dest:
            print('Copying to end dest: ' + dest)
            if not dest.endswith(os.path.sep):
                dest += os.path.sep

            copytree('./dist', dest, dirs_exist_ok=True)
    except Exception as e:
        print(e)


if __name__ == "__main__":
    cwd = os.getcwd()
    if 'build' in cwd:
        cwd, _, _ = cwd.partition('build')
        os.chdir(cwd)

    parser = ArgumentParser()
    parser.add_argument("--dest", "-d", dest="dest", type=str, required=False, nargs='?', const='',
                        help="Copy to another destination when done")
    parser.add_argument("--release", "-r", dest="release", default=False, action="store_true",
                        help="Include/Exclude files depending on release mode")
    args = parser.parse_args()

    post_build(args.release, args.dest)
