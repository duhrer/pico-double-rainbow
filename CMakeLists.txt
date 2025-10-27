cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

# Initialize the SDK
pico_sdk_init()

add_subdirectory(../../third-party/pi-pico-adafruit-neopixels/pico_neopixels build)

set(DOUBLE pico-double-rainbow)

project(${DOUBLE} C CXX ASM)

# Add your source files
add_executable(${DOUBLE}
    src/rainbow.cpp
)

# Link required libraries
target_link_libraries(${DOUBLE} LINK_PUBLIC
    pico_stdlib
    pico_neopixel
    pico_bootsel_via_double_reset
    hardware_pio
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${DOUBLE})

# Disable USB serial and enable UART serial
pico_enable_stdio_usb(${DOUBLE} 0)
pico_enable_stdio_uart(${DOUBLE} 1)

set(COMBINED pico-combined-rainbow)

project(${COMBINED} C CXX ASM)

# Add your source files
add_executable(${COMBINED}
    src/rainbow.cpp
)

target_compile_definitions(${COMBINED} PUBLIC PIXEL_STRINGS=1)

# Link required libraries
target_link_libraries(${COMBINED} LINK_PUBLIC
    pico_stdlib
    pico_neopixel
    pico_bootsel_via_double_reset
    hardware_pio
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${COMBINED})

# Disable USB serial and enable UART serial
pico_enable_stdio_usb(${COMBINED} 0)
pico_enable_stdio_uart(${COMBINED} 1)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${DOUBLE}.uf2
    ${CMAKE_CURRENT_BINARY_DIR}/${COMBINED}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)


