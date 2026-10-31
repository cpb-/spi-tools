/* SPDX-License-Identifier: GPL-2.0-only */
/* Test-only spidev emulator, loaded with LD_PRELOAD. */
#define _GNU_SOURCE
#include <errno.h>
#include <linux/spi/spidev.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static uint8_t mode;
static uint8_t lsb;
static uint8_t bits = 8;
static uint32_t speed = 500000;

static void log_value(const char *name, unsigned long value)
{
	const char *path = getenv("SPI_TEST_LOG");
	FILE *log = path ? fopen(path, "a") : NULL;
	if (log != NULL) {
		fprintf(log, "%s:%lu\n", name, value);
		fclose(log);
	}
}

int ioctl(int fd, unsigned long request, ...)
{
	va_list args;
	void *value;
	(void)fd;
	va_start(args, request);
	value = va_arg(args, void *);
	va_end(args);
	switch (request) {
	case SPI_IOC_RD_MODE: *(uint8_t *)value = mode; return 0;
	case SPI_IOC_RD_LSB_FIRST: *(uint8_t *)value = lsb; return 0;
	case SPI_IOC_RD_BITS_PER_WORD: *(uint8_t *)value = bits; return 0;
	case SPI_IOC_RD_MAX_SPEED_HZ: *(uint32_t *)value = speed; return 0;
	case SPI_IOC_WR_MODE:
		mode = *(uint8_t *)value;
		log_value("mode", mode);
		return 0;
	case SPI_IOC_WR_LSB_FIRST:
		lsb = *(uint8_t *)value;
		log_value("lsb", lsb);
		return 0;
	case SPI_IOC_WR_BITS_PER_WORD:
		bits = *(uint8_t *)value;
		log_value("bits", bits);
		return 0;
	case SPI_IOC_WR_MAX_SPEED_HZ:
		speed = *(uint32_t *)value;
		log_value("speed", speed);
		if (getenv("SPI_TEST_CONFIG_ERROR") && speed == 1000000) {
			errno = EIO;
			return -1;
		}
		return 0;
	case SPI_IOC_MESSAGE(1): {
		struct spi_ioc_transfer *transfer = value;
		log_value("transfer", transfer->len);
		if (getenv("SPI_TEST_TRANSFER_ERROR")) {
			errno = EIO;
			return -1;
		}
		memcpy((void *)(uintptr_t)transfer->rx_buf,
		       (const void *)(uintptr_t)transfer->tx_buf, transfer->len);
		return (int)transfer->len;
	}
	default:
		errno = ENOTTY;
		return -1;
	}
}

ssize_t read(int fd, void *buffer, size_t count)
{
	static int calls;
	if (fd == STDIN_FILENO) {
		int call = calls++;
		if (getenv("SPI_TEST_EINTR") && call == 0) {
			errno = EINTR;
			return -1;
		}
		if (getenv("SPI_TEST_READ_ERROR")) {
			if (call > 0) {
				errno = EIO;
				return -1;
			}
			if (count > 1)
				count = 1;
		}
	}
	return syscall(SYS_read, fd, buffer, count);
}

/* Optimized glibc builds may call the fortified entry point instead. */
ssize_t __read_chk(int fd, void *buffer, size_t count, size_t buffer_size)
{
	if (count > buffer_size)
		abort();
	return read(fd, buffer, count);
}
