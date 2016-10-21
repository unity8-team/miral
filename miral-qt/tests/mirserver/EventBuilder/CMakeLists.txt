set(
  EVENT_BUILDER_TEST_SOURCES
  eventbuilder_test.cpp
)

include_directories(
  ${MIRAL_QT_SOURCE_DIR}/src/platforms/mirserver
  ${MIRAL_QT_SOURCE_DIR}/src/common
)

add_executable(EventBuilderTest ${EVENT_BUILDER_TEST_SOURCES})

target_link_libraries(
  EventBuilderTest
  qpa-mirserver
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(EventBuilder, EventBuilderTest)
