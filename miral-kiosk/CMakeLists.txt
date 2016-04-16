add_executable(miral-kiosk
    kiosk_main.cpp
    kiosk_window_manager.cpp    kiosk_window_manager.h
    sw_splash.cpp               sw_splash.h
)

target_link_libraries(miral-kiosk miral)

install(TARGETS miral-kiosk
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
