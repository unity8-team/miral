# TODO remove dependency on libmirserver-dev
# (Only needed by awkward painting code)
pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)
include_directories(include SYSTEM ${MIRSERVER_INCLUDE_DIRS})


add_library(miral-titlebar STATIC
        titlebar_user_data.cpp titlebar_user_data.h
)

target_link_libraries(miral-titlebar
    miral
)
