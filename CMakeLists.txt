#######################################################################
# CMakeLists.txt for the spi-tools project.
#
# Copyright 2024  Christophe BLAESS (https://www.blaess.fr/christophe).
#
# License  GPLv3+:  This is free software:  you are free to  change and
# redistribute it.There is NO WARRANTY, to the extent permitted by law.
#
#######################################################################

cmake_minimum_required(VERSION 3.0)
project(SPI_TOOLS VERSION 1.0.2)

configure_file(src/config.h.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

add_executable(spi-config src/spi-config.c src/spi-tools.c src/spi-tools.h)
add_executable(spi-pipe src/spi-pipe.c src/spi-tools.c src/spi-tools.h)

install(TARGETS spi-config DESTINATION /usr/bin)
install(TARGETS spi-pipe DESTINATION /usr/bin)
