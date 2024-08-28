import platform
import subprocess
import click


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
    preset = 'release' if release else 'debug'
    if (examples and plugins) or all:
        preset += '-all'
    elif examples:
        preset += '-examples'
    elif plugins:
        preset += '-plugins'

    default_profile = run("conan profile path default")
    if default_profile:
        run("conan profile detect")

    run(f"conan install . --build missing -pr:b=default -pr:h=./profiles/"
        f"{'windows' if platform.system() == "Windows" else 'linux'}-{('rel' if release else 'dbg')}")
    run(f'cmake --preset="{preset}"')
    run(f"cmake --build build/{('Release' if release else 'Debug')}")


if __name__ == '__main__':
    cli()

