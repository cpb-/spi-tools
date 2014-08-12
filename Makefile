
PROJECT_VERSION=\"$(shell git describe --tags)\"

CFLAGS=-Wall -DVERSION=$(PROJECT_VERSION)

EXEC_FILES=          \
        spi-config   \
        spi-pipe     \


.PHONY: all

all: ${EXEC_FILES}


.PHONY: clean

clean:
	rm -f ${EXEC_FILES} *.o *~


