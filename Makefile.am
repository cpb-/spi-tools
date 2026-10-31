# SPDX-License-Identifier: GPL-2.0-only
SUBDIRS=src man
CONFIGURE_DEPENDENCIES = $(srcdir)/VERSION

EXTRA_DIST = LICENSE VERSION CMakeLists.txt Android.bp tests/check.py tests/mock-spi.c \
             .github/workflows/build.yml

if HAVE_PYTHON
check-local:
	CC="$(CC)" $(PYTHON) $(srcdir)/tests/check.py --source $(srcdir) --bin-dir $(builddir)/src
endif
