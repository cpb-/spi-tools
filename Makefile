CFLAGS=-Wall

EXEC_FILES=          \
        spi-config   \
        spi-pipe     \


.PHONY: all

all: ${EXEC_FILES}


.PHONY: clean

clean:
	rm -f ${EXEC_FILES} *.o *~


