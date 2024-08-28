import platform
import subprocess
import click
import shutil
import os


def run(cmd: str) -> int:
    click.echo(">>> Running Command: " + cmd)
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True, text=True)
    while proc.poll() is None:
        line = proc.stdout.readline()

        # if it ends with a new line character, remove it, so we don't print two lines
        if line.endswith('\n'):
            if line.endswith('\r\n'):
                line = line[:-2]
            else:
                line = line[:-1]

        print(line)

    print(proc.stdout.read())
    return proc.returncode


@click.group()
def cli():
    pass


@cli.command()
@click.option("-r", "--release", is_flag=True, help="Build in release mode")
@click.option("-e", "--examples", is_flag=True, help="Build the examples")
@click.option("-p", "--plugins", is_flag=True, help="Build the plugins")
@click.option("-a", "--all", is_flag=True, help="Build both the examples and the plugins")
def build(release: bool, examples: bool, plugins: bool, all: bool):
    is_windows = platform.system() == "Windows"
    preset = 'build' if is_windows else ('release' if release else 'debug')
    if (examples and plugins) or all:
        preset += '-all'
    elif examples:
        preset += '-examples'
    elif plugins:
        preset += '-plugins'

    default_profile = run("conan profile path default")
    if default_profile:
        run("conan profile detect")

    presets = "CMakePresets.json"
    if (os.path.exists(presets)):
        os.remove(presets)

    if is_windows:
        shutil.copy2("CMakePresetsWindows.json", presets)
    else:
        shutil.copy2("CMakePresetsLinux.json", presets)

    run(f"conan install . --build missing -pr:b=default -pr:h=./profiles/"
        f"{'windows' if is_windows else 'linux'}-{('rel' if release else 'dbg')}")
    run(f'cmake --preset="{preset}"')
    run(f"cmake --build build/{('' if is_windows else ('Release' if release else 'Debug'))}")


if __name__ == '__main__':
    cli()

