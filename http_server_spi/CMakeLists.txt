# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

if("${IDF_TARGET}" STREQUAL "linux")
    set(COMPONENTS main)
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    "Core"
    # This
    "../../HY_MCU_MOD/ESP32_MOD"
    "$ENV{IDF_PATH}/examples/common_components/protocol_examples_common"
)

project(http_server_spi)
