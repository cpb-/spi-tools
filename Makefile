
.PHONY: all
all:
	\set -e;                          \
	cd src; make all; cd ..;          \
	cd man; make all; cd ..;          \


.PHONY: clean
clean:
	\set -e;                          \
	cd src; make clean; cd ..;        \
	cd man; make clean; cd ..;        \

.PHONY: install
install:
	\set -e;                          \
	cd src; make install; cd ..;      \
	cd man; make install; cd ..;      \

.PHONY: uninstall
uninstall:
	\set -e;                          \
	cd src; make uninstall; cd ..;    \
	cd man; make uninstall; cd ..;    \

