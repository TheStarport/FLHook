import platform
import subprocess
import click
import shutil
import os


def is_windows():
    return platform.system() == "Windows" or os.getenv("GITHUB_ACTIONS") is not None


def log(s: str):
    click.echo(">>> " + s)


def run(cmd: str, no_log: bool = False, allow_error: bool = False) -> int:
    log("Running Command: " + cmd)
    proc = subprocess.Popen(cmd, stdout=(subprocess.DEVNULL if no_log else subprocess.PIPE),
                            stderr=subprocess.STDOUT, shell=True, text=True, encoding='UTF-8')

    if no_log:
        proc.wait()
    else:
        empty_line = False
        while proc.poll() is None:
            line = proc.stdout.readline()

            # if it ends with a new line character, remove it, so we don't print two lines
            if line.endswith('\n'):
                if line.endswith('\r\n'):
                    line = line[:-2]
                else:
                    line = line[:-1]

            if line == '':
                if empty_line:
                    continue
                else:
                    empty_line = True
            else:
                empty_line = False

            print(line)

        print(proc.stdout.read())
        if not allow_error and proc.returncode != 0:
            raise ValueError(f"Execution of {cmd} failed")

    return proc.returncode


@click.group()
def cli():
    pass


@cli.command(short_help='Install and build dependencies via conan')
def configure():
    default_profile = run("conan profile path default", allow_error=True)
    if default_profile:
        run("conan profile detect")

    host = 'windows' if is_windows() else 'linux'
    run(f"conan install . -s build_type=Debug --build missing -pr:b=default -pr:h=./profiles/{host}")
    run(f"conan install . -s build_type=Release --build missing -pr:b=default -pr:h=./profiles/{host}")


@cli.command(short_help='Runs a first-time build, downloading any needed dependencies, and generating preset files.')
@click.option("-r", "--release", is_flag=True, help="Build in release mode")
@click.option("--no-post-build", is_flag=True, help="Configure in release mode")
@click.pass_context
def build(ctx: click.Context, release: bool, no_post_build: bool):
    preset = 'release' if release else 'debug'

    # noinspection PyTypeChecker
    ctx.invoke(configure)

    run(f'cmake --preset="{preset}" {'-DNO_POST_BUILD=TRUE' if no_post_build else ''}')
    run(f"cmake --build build/{preset.title()}", allow_error=True)


if __name__ == '__main__':
    cli()
