# Simple bash
docker run --rm -i -e PATH=/usr/bin -v $PWD\var:c:/msys64/var/lib/pacman msys2 C:/msys64/usr/bin/bash.exe




# Start MYS tools:
docker run --rm -i -v $PWD\var:c:/var/lib/pacman msys2 C:/msys64/msys2_shell.cmd -defterm -no-start -msys2
