
PROJECT_VERSION=\"$(shell git describe --tags)\"

CFLAGS=-Wall -DVERSION=$(PROJECT_VERSION)

INSTALL_DIR?=/usr/sbin/

EXEC_FILES=          \
        spi-config   \
        spi-pipe     \


.PHONY: all
all: ${EXEC_FILES}


.PHONY: clean
clean:
	rm -f ${EXEC_FILES} *.o *~


.PHONY: install
install: ${EXEC_FILES}
	install ${EXEC_FILES} "${INSTALL_DIR}"

.PHONY: uninstall
uninstall:
	@set -e;                          \
	for f in ${EXEC_FILES}; do        \
	    rm -f "${INSTALL_DIR}/$${f}"; \
	done                              \

