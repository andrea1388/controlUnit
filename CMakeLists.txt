# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
# set(EXTRA_COMPONENT_DIRS /Users/ac/esp/components/)
# set(srcs ${CORE_SRCS} ${LIBRARY_SRCS} ${BLE_SRCS})
# idf_component_register(INCLUDE_DIRS ${includedirs} PRIV_INCLUDE_DIRS ${priv_includes} SRCS ${srcs} REQUIRES ${requires} PRIV_REQUIRES ${priv_requires})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ControlUnit)
# target_add_binary_data(ControlUnit.elf "certs/ca.crt" TEXT)


idf_build_set_property(COMPILE_OPTIONS "-Wno-unknown-pragmas" APPEND)