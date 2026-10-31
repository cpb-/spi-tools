#######################################################################
# CMakeLists.txt for the spi-tools project.
#
# Copyright 2024  Christophe BLAESS (https://www.blaess.fr/christophe).
#
# SPDX-License-Identifier: GPL-2.0-only
# License  GPLv2:  This is free software:  you are free to  change and
# redistribute it. There is NO WARRANTY, to the extent permitted by law.
#
#######################################################################

cmake_minimum_required(VERSION 3.10)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" SPI_TOOLS_VERSION LIMIT_COUNT 1)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
project(SPI_TOOLS VERSION ${SPI_TOOLS_VERSION} LANGUAGES C)

include(GNUInstallDirs)
configure_file(src/config.h.cmake.in generated/config.h @ONLY)

add_executable(spi-config src/spi-config.c src/spi-tools.c src/spi-tools.h)
add_executable(spi-pipe src/spi-pipe.c src/spi-tools.c src/spi-tools.h)

foreach(target spi-config spi-pipe)
    target_include_directories(${target} BEFORE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
    target_compile_definitions(${target} PRIVATE _POSIX_C_SOURCE=200809L)
    set_target_properties(${target} PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES)
endforeach()

install(TARGETS spi-config spi-pipe RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES man/spi-config.1 man/spi-pipe.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

include(CTest)
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    find_program(SPI_TOOLS_PYTHON NAMES python3)
    if(SPI_TOOLS_PYTHON)
        add_test(NAME regression COMMAND "${SPI_TOOLS_PYTHON}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/check.py"
            --source "${CMAKE_CURRENT_SOURCE_DIR}"
            --bin-dir "$<TARGET_FILE_DIR:spi-pipe>")
    endif()
endif()
