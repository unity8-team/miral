include_directories(
    ${MIRAL_QT_SOURCE_DIR}/src/platforms/mirserver
)

include_directories(
    SYSTEM
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    )

set(SCREENSPLUGIN_SRC
    plugin.cpp
    screens.cpp
    qquickscreenwindow.cpp
    )

# miral-qt/src/platforms/mirserver/customscreenconfiguration.h references mir/graphics/display_configuration.h
set_source_files_properties(qquickscreenwindow.cpp  PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${FIXME_MIRSERVER_CXXFLAGS}")

add_library(unityscreensplugin SHARED
    ${SCREENSPLUGIN_SRC}
)

target_link_libraries(
    unityscreensplugin

    qpa-mirserver

    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
)

# install
add_qml_plugin(Unity.Screens 0.1 Unity/Screens TARGETS unityscreensplugin)
