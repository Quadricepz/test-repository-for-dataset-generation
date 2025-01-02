
cmake_minimum_required(VERSION 3.12)

project(hello_world C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)


# add_executable(hello_world_test "")

# target_include_directories(hello_world_test
#   PRIVATE
#   ${CMAKE_CURRENT_LIST_DIR}/.
# )

# set_target_properties(
#   hello_world_test
#   PROPERTIES
#   COMPILE_FLAGS -fno-rtti
#   COMPILE_FLAGS -fno-exceptions
#   COMPILE_FLAGS -fno-threadsafe-statics
#   COMPILE_FLAGS -nostdlib
# )

# target_sources(hello_world_test
#   PRIVATE
#   ${CMAKE_CURRENT_LIST_DIR}/hello_world_test.cpp
# )

# target_link_libraries(
#   hello_world_test
#   pico-tflmicro
#   pico-tflmicro_test
# )

# pico_enable_stdio_usb(hello_world_test 1)
# pico_enable_stdio_uart(hello_world_test 0)

# pico_add_extra_outputs(hello_world_test)


add_executable(fruit_classifier "")

target_include_directories(fruit_classifier
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/.
  ${PICO_SDK_PATH}/src/common/pico_stdlib/include
)

set_target_properties(
  fruit_classifier
  PROPERTIES
  COMPILE_FLAGS -fno-rtti
  COMPILE_FLAGS -fno-exceptions
  COMPILE_FLAGS -fno-threadsafe-statics
  COMPILE_FLAGS -nostdlib
)

target_sources(fruit_classifier
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/constants.cpp
  ${CMAKE_CURRENT_LIST_DIR}/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/main_functions.cpp
  ${CMAKE_CURRENT_LIST_DIR}/rgb_sensor/Adafruit_TCS34725.cpp
  ${CMAKE_CURRENT_LIST_DIR}/rgb_sensor/setup.cpp
  ${CMAKE_CURRENT_LIST_DIR}/rgb_sensor/TCS34725_utils.cpp

  ${CMAKE_CURRENT_LIST_DIR}/rgb_sensor/Adafruit_TCS34725.h
  ${CMAKE_CURRENT_LIST_DIR}/rgb_sensor/setup.h
  ${CMAKE_CURRENT_LIST_DIR}/rgb_sensor/TCS34725_utils.h
  ${CMAKE_CURRENT_LIST_DIR}/constants.h
  ${CMAKE_CURRENT_LIST_DIR}/main_functions.h
)

target_link_libraries(
  fruit_classifier
  pico-tflmicro
  hardware_pwm
  pico_stdlib
  hardware_i2c
)

pico_enable_stdio_usb(fruit_classifier 1)
pico_enable_stdio_uart(fruit_classifier 0)

pico_add_extra_outputs(fruit_classifier)
