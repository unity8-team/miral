#!/bin/sh
if [ "$1" = "gnome-terminal" ]
then extras='--app-id com.canonical.miral.Terminal'
fi
unset QT_QPA_PLATFORMTHEME
GDK_BACKEND=mir QT_QPA_PLATFORM=ubuntumirclient SDL_VIDEODRIVER=mir "$@" ${extras}&
