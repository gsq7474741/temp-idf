# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(IDF_TARGET esp32s3)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3_cpp_pthread)

#list(APPEND EXTRA_COMPONENT_DIRS "esp_lte_modem/components/modem")
#set(IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

#add_custom_target(pio_monitor
#    COMMAND pio device monitor -b 115200
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    USES_TERMINAL )