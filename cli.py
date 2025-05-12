import platform
import subprocess
import click
import shutil
import os

from scripts.post_build import post_build


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


@cli.command(name='post_build',
             short_help='Create a build zip of all the build artefacts, and optionally copying them elsewhere')
@click.option("-r", "--release", is_flag=True, help="Build in release mode")
@click.option("-d", "--dest", default=None, type=str, help="Copy to another destination when done")
def _post_build(release: bool, dest: str | None):
    post_build(release, dest)


@cli.command(short_help='Install and build dependencies via conan')
@click.option("--lock", is_flag=True, help="Regenerate lockfile if needed")
def configure(lock: bool):
    default_profile = run("conan profile path default", allow_error=True)
    if default_profile:
        run("conan profile detect")

    host = 'windows' if is_windows() else 'linux'
    args = [
        ".",
        '--lockfile-out=conan.lock',
        '--build missing',
        '-pr:b=default',
        f'-pr:h=./profiles/{host}'
    ]

    if lock:
        args.append('--lockfile-partial')

    run(f"conan install {' '.join(args)} -s build_type=Debug")
    run(f"conan install {' '.join(args)} -s build_type=Release")


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
