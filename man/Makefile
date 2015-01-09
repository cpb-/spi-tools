
MAN_INSTALL_DIR  ?= $(DESTDIR)/usr/share/man
MAN1_INSTALL_DIR ?= ${MAN_INSTALL_DIR}/man1

MAN1_GZ_FILES=           \
	spi-config.1.gz  \
	spi-pipe.1.gz    \

.PHONY: all
all: ${MAN1_GZ_FILES}


.PHONY: clean
clean:
	rm -f ${MAN1_GZ_FILES} *~


.PHONY: install
install: ${MAN1_GZ_FILES}
	mkdir -p "${MAN1_INSTALL_DIR}"
	install ${MAN1_GZ_FILES} "${MAN1_INSTALL_DIR}"

.PHONY: uninstall
uninstall:
	@set -e;                               \
	for f in ${MAN1_GZ_FILES}; do          \
	    rm -f "${MAN1_INSTALL_DIR}/$${f}"; \
	done;                                  \

%.1.gz: %.1
	gzip -c $< > $@

