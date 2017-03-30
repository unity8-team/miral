#!/bin/sh
if [ "$1" = "gnome-terminal" ]
then extras='--app-id com.canonical.miral.Terminal'
fi
unset QT_QPA_PLATFORMTHEME
MIR_SOCKET=${XDG_RUNTIME_DIR}/miral_socket GDK_BACKEND=mir QT_QPA_PLATFORM=ubuntumirclient SDL_VIDEODRIVER=mir "$@" ${extras}&
