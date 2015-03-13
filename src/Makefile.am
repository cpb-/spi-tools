bin_PROGRAMS = spi-config spi-pipe

GIT_VERSION:=$(shell git describe --abbrev=10 --dirty --always)

spi_config_SOURCES = spi-config.c
spi_config_CFLAGS = -DVERSION=\"$(GIT_VERSION)\"
spi_config_LDADD =

spi_pipe_SOURCES = spi-pipe.c
spi_pipe_CFLAGS = -DVERSION=\"$(GIT_VERSION)\"
spi_pipe_LDADD = 