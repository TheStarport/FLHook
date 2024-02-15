import subprocess
import os
from typing import Callable, Union, Tuple

is_windows = os.name == "nt"


def write_env(var: str, value: str):
    if is_windows:
        os.system("setx {0} {1}".format(var, value))
    else:
        # Assume linux
        with open(os.path.expanduser("~/.bashrc"), "a") as outfile:
            outfile.write("export {0}={1}".format(var, value))
    print("-- Set {0} to {1}".format(var, value))


def set_path(var: str):
    while True:
        path = input("Please specify a valid path:    ")
        if os.path.isdir(path):
            write_env(var, path)
            break
        else:
            print("ERR Invalid path provided")


def handle_yn(message: str, on_yes: Union[Callable, None] = None, on_no: Union[Callable, None] = None, **kwargs):
    yn = str.lower(input(message + "  Y/N:   "))

    while True:
        if yn == "y":
            if on_yes is not None:
                on_yes(**kwargs)
            break
        elif yn == "n":
            if on_no is not None:
                on_no(**kwargs)
            break
        else:
            yn = str.lower(input("ERR Invalid input. Y/N:   "))


def set_vcpkg_root():
    handle_yn("Would you like to set the VCPKG_ROOT now?", on_yes=set_path, var="VCPKG_ROOT")


def install_vcpkg_root():
    path = ""
    while True:
        path = input("Please specify a valid path to clone vcpkg into:    ")
        if (is_windows and not path.endswith('\\')) or (not is_windows and not path.endswith('/')):
            path += "\\" if is_windows else "/"

        if not os.path.exists(os.path.dirname(path)):
            break
        else:
            print("ERR Invalid path provided")

    print("Cloning into {0}".format(path))
    # noinspection PyBroadException
    try:
        proc = subprocess.Popen(['git', 'clone', "https://github.com/Microsoft/vcpkg", path])
        proc.wait()

        bootstrap = ("{0}/bootstrap-vcpkg.bat" if is_windows else "{0}/bootstrap-vcpkg.sh").format(path.strip("/\\"))

        print("Setting up vcpkg")
        proc = subprocess.Popen([bootstrap])
        proc.wait()

        print("vcpkg installed and setup")
        set_vcpkg_root()
    except Exception as ex:
        print("Failed to install vcpkg. err: {}".format(str(ex)))


def query_vcpkg_root():
    handle_yn("Would you like to download and setup vcpkg now?", on_yes=install_vcpkg_root)


def get_cmake_version() -> Tuple[int, int, int] | None:
    # noinspection PyBroadException
    try:
        output = subprocess.check_output(['cmake', '--version']).decode('utf-8')
        line = output.splitlines()[0]
        version_raw = line.split()[2]
        version = version_raw.split('.')
        return int(version[0]), int(version[1]), int(''.join(c for c in version[2] if c.isdigit()))
    except:
        return None


def download_cmake():
    try:
        print("Downloading CMake using PIP")
        proc = subprocess.Popen(['python', '-m', "pip", "install", "cmake"])
        proc.wait()

        print("Successfully downloaded cmake. Testing path")
        version = get_cmake_version()
        if version is None:
            print("ERR unable to find cmake on the path!! Manual fix required")
        else:
            print("-- Found CMake -- {0}.{1}.{2}".format(version[0], version[1], version[2]))
    except Exception as ex:
        print("Failed to install vcpkg. err: {}".format(str(ex)))


copy_path = os.environ.get("FLHOOK_COPY_PATH", None)
if copy_path is None:
    print("-- You do not have FLHOOK_COPY_PATH defined in your environment variables")
    print("-- Setting the FLHOOK_COPY_PATH specifies a destination directory to copy DLLs to after a successful build")
    handle_yn("-- Would you like to set the FLHOOK_COPY_PATH now?", on_yes=set_path, var="FLHOOK_COPY_PATH")
else:
    print("-- Found FLHOOK_COPY_PATH    " + copy_path)

vcpkg = os.environ.get("VCPKG_ROOT", None)
if vcpkg is None:
    print("You do not have VCPKG_ROOT defined.")
    print("vcpkg is required to download and manage FLHook's dependencies.")
    handle_yn("Is it currently installed on your system?", on_yes=set_vcpkg_root, on_no=query_vcpkg_root)
else:
    print("-- Found VCPKG_ROOT    " + vcpkg)

cmake = get_cmake_version()
if cmake is None:
    print("You do not have CMake installed or defined on the path.")
    handle_yn("Would you like to download it?", on_yes=download_cmake)
elif cmake[0] < 3 or cmake[1] < 20:
    print("-- Your CMake version is out of date. Please update it! -- https://cmake.org/download/")
else:
    print("-- Found CMake Install: {0}.{1}.{2}".format(cmake[0], cmake[1], cmake[2]))

