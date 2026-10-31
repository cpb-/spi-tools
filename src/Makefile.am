# SPDX-License-Identifier: GPL-2.0-only
AM_CPPFLAGS = -I$(builddir)
AM_CFLAGS = $(DEBUG_CFLAGS)
bin_PROGRAMS = spi-config spi-pipe

spi_config_SOURCES = spi-config.c spi-tools.c spi-tools.h
spi_config_LDADD =

spi_pipe_SOURCES = spi-pipe.c spi-tools.c spi-tools.h
spi_pipe_LDADD =

EXTRA_DIST = config.h.cmake.in
