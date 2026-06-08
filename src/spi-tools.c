/*
 * spidev tools.
 *
 * (c) 2014-2026 Christophe BLAESS <christophe.blaess@logilin.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */


// ---------------------- System headers.

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>


// ---------------------- Project headers.

#include "spi-tools.h"


// ---------------------- Private macros declarations.

// ---------------------- Private types and structures.

// ---------------------- Private method declarations.

static int parse_int_with_range(const char *s, int min, int max, int *value);

// ---------------------- Private variables.

// ---------------------- Public methods

int Read_spi_configuration(int fd, spi_config_t *config)
{
	uint8_t  u8;
	uint32_t u32;

	if (config == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_MODE, &u8) < 0) {
		perror("SPI_IOC_RD_MODE");
		return -1;
	}
	config->spi_mode = u8 & 0x03;
	config->spi_ready = ((u8 & SPI_READY) ? 1 : 0);

	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &u8) < 0) {
		perror("SPI_IOC_RD_LSB_FIRST");
		return -1;
	}
	config->lsb_first = (u8 ? 1 : 0);

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &u8) < 0) {
		perror("SPI_IOC_RD_BITS_PER_WORD");
		return -1;
	}
	config->bits_per_word = (u8 == 0 ? 8 : u8);

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &u32) < 0) {
		perror("SPI_IOC_RD_MAX_SPEED_HZ");
		return -1;
	}
	config->spi_speed = u32;

	return 0;
}



int Write_spi_configuration(int fd, const spi_config_t *config)
{
	uint8_t  u8;
	uint32_t u32;

	if (config == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (config->spi_mode >= 0 || config->spi_ready >= 0) {
		if (ioctl(fd, SPI_IOC_RD_MODE, &u8) < 0) {
			perror("SPI_IOC_RD_MODE");
			return -1;
		}
		if (config->spi_mode >= 0) {
			u8 &= ~0x03;
			u8 |= config->spi_mode;
		}

		if (config->spi_ready >= 0) {
			if (config->spi_ready)
				u8 |= SPI_READY;
			else
				u8 &= ~SPI_READY;
		}

		if (ioctl(fd, SPI_IOC_WR_MODE, &u8) < 0) {
			perror("SPI_IOC_WR_MODE");
			return -1;
		}
	}

	if (config->lsb_first >= 0) {
		u8 = (config->lsb_first ? 1 : 0);
		if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &u8) < 0) {
			perror("SPI_IOC_WR_LSB_FIRST");
			return -1;
		}
	}

	if (config->bits_per_word >= 0) {
		u8 = config->bits_per_word;
		if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &u8) < 0) {
			perror("SPI_IOC_WR_BITS_PER_WORD");
			return -1;
		}
	}

	if (config->spi_speed > 0) {
		u32 = config->spi_speed;
		if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &u32) < 0) {
			perror("SPI_IOC_WR_MAX_SPEED_HZ");
			return -1;
		}
	}

	return 0;
}


int Parse_spi_mode(const char *optarg, spi_config_t *config)
{
	if (config == NULL)
		return -1;

	if (parse_int_with_range(optarg, 0, 3, &(config->spi_mode)) != 0) {
		fprintf(stderr, "Error: wrong SPI mode ([0-3]): %s\n", optarg ? optarg : "(null)");
		return -1;
	}
	return 0;
}



int Parse_lsb_first(const char *optarg, spi_config_t *config)
{
	if (config == NULL)
		return -1;

	if (parse_int_with_range(optarg, 0, 1, &(config->lsb_first)) != 0) {
		fprintf(stderr, "Error: wrong LSB value ([0,1]): %s\n", optarg ? optarg : "(null)");
		return -1;
	}
	return 0;
}



int Parse_spi_speed(const char *optarg, spi_config_t *config)
{
	if (config == NULL)
		return -1;

	if (parse_int_with_range(optarg, 0, 100000000, &(config->spi_speed)) != 0) {
		fprintf(stderr, "Error: invalid SPI speed ([0-100000000]): %s\n", optarg ? optarg : "(null)");
		return -1;
	}
	return 0;
}



int Parse_spi_ready(const char *optarg, spi_config_t *config)
{
	if (config == NULL)
		return -1;

	if (parse_int_with_range(optarg, 0, 1, &(config->spi_ready)) != 0) {
		fprintf(stderr, "Error: wrong SPI-ready value ([0, 1]): %s\n", optarg ? optarg : "(null)");
		return -1;
	}
	return 0;
}



int Parse_spi_bits_per_word(const char *optarg, spi_config_t *config)
{
	if (config == NULL)
		return -1;

	if (parse_int_with_range(optarg, 7, 255, &(config->bits_per_word)) != 0) {
		fprintf(stderr, "Error: wrong bits-per-word value ([7, 255]): %s\n", optarg ? optarg : "(null)");
		return -1;
	}
	return 0;
}



int Transfer_spi_buffers(int fd, const void *tx_buffer, void *rx_buffer, size_t length)
{
	struct spi_ioc_transfer transfer = {
		.tx_buf        = 0,
		.rx_buf        = 0,
		.len           = 0,
		.delay_usecs   = 0,
		.speed_hz      = 0,
		.bits_per_word = 0,
	};


	if (length > UINT32_MAX) {
		errno = EINVAL;
		return -1;
	}

	if ((length > 0) && (tx_buffer == NULL) && (rx_buffer == NULL)) {
		errno = EINVAL;
		return -1;
	}

	transfer.rx_buf = (unsigned long)rx_buffer;
	transfer.tx_buf = (unsigned long)tx_buffer;
	transfer.len = length;

	if (ioctl(fd, SPI_IOC_MESSAGE(1), & transfer) < 0)
		return -1;

	return 0;
}


// ---------------------- Private methods

static int parse_int_with_range(const char *str, int min, int max, int *value)
{
	char *end = NULL;
	long tmpvalue;

	if ((str == NULL) || (value == NULL))
		return -1;

	errno = 0;
	tmpvalue = strtol(str, &end, 0);
	if ((errno != 0) || (end == str) || (*end != '\0'))
		return -1;

	if ((tmpvalue < min) || (tmpvalue > max))
		return -1;

	*value = (int)tmpvalue;

	return 0;
}

