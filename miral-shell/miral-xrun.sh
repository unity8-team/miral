#!/bin/bash
port=0

while [ -e "/tmp/.X11-unix/X${port}" ]; do
    let port+=1
done

unset QT_QPA_PLATFORMTHEME
unset GDK_BACKEND
unset QT_QPA_PLATFORM
unset SDL_VIDEODRIVER
MIR_SOCKET=${XDG_RUNTIME_DIR}/miral_socket Xmir -rootless :${port} & pid=$!
DISPLAY=:${port} "$@"
kill ${pid}
