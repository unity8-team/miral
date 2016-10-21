set(
  APPLICATION_MANAGER_TEST_SOURCES
  application_manager_test.cpp
  ${MIRAL_QT_SOURCE_DIR}/src/common/debughelpers.cpp
)
include_directories(
  ${MIRAL_QT_SOURCE_DIR}/src/platforms/mirserver
  ${MIRAL_QT_SOURCE_DIR}/src/modules
  ${MIRAL_QT_SOURCE_DIR}/tests/framework
)

include_directories(
  SYSTEM
  ${APPLICATION_API_INCLUDE_DIRS}
)

add_executable(applicationmanager_test ${APPLICATION_MANAGER_TEST_SOURCES})

add_dependencies(applicationmanager_test qtmir-test-framework-static)

target_link_libraries(
  applicationmanager_test

  Qt5::Test

  unityapplicationplugin

  -L${CMAKE_BINARY_DIR}/miral-qt/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(ApplicationManager applicationmanager_test)
