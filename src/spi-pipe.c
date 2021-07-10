/*
 * spidev data transfer tool.
 *
 * (c) 2014 Christophe BLAESS <christophe.blaess@logilin.fr>
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

#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include "config.h"

static char *project = "spi-pipe";


static void display_usage(const char *name)
{
	fprintf(stderr, "usage: %s options...\n", name);
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "    -d --device=<dev>    Use the given spi-dev character device.\n");
	fprintf(stderr, "    -m --mode=[0-3]      Use the selected spi mode:\n");
	fprintf(stderr, "             0: low idle level, sample on leading edge,\n");
	fprintf(stderr, "             1: low idle level, sample on trailing edge,\n");
	fprintf(stderr, "             2: high idle level, sample on leading edge,\n");
	fprintf(stderr, "             3: high idle level, sample on trailing edge.\n");
	fprintf(stderr, "    -s --speed=<speed>   Maximum SPI clock rate (in Hz).\n");
	fprintf(stderr, "    -B --bits=[7...]     Bits per word.\n");
	fprintf(stderr, "    -b --blocksize=<int> transfer block size in byte.\n");
	fprintf(stderr, "    -n --number=<int>    number of blocks to transfer (-1 = infinite).\n");
	fprintf(stderr, "    -h --help            this screen.\n");
	fprintf(stderr, "    -v --version         display the version number.\n");
}

int main (int argc, char *argv[])
{
	int opt;
	int long_index = 0;

	static struct option options[] = {
		{"device",    required_argument, NULL,  'd' },
		{"speed",     required_argument, NULL,  's' },
		{"blocksize", required_argument, NULL,  'b' },
		{"number",    required_argument, NULL,  'n' },
		{"mode",      required_argument, NULL,  'm' },
		{"bits",      required_argument, NULL,  'B' },
		{"help",      no_argument,       NULL,  'h' },
		{"version",   no_argument,       NULL,  'v' },
		{0,           0,                 0,  0 }
	};

	int fd;
	char *device = NULL;
	uint8_t *rx_buffer = NULL;
	uint8_t *tx_buffer = NULL;
	int spi_mode      = -1;
	int block_size    =  1;
	int blocks_count  = -1;
	int bits_per_word = -1;
	int offset        =  0;
	int nb            =  0;
	int speed         = -1;
	int orig_speed    = -1;

	struct spi_ioc_transfer transfer = {
		.tx_buf        = 0,
		.rx_buf        = 0,
		.len           = 0,
		.delay_usecs   = 0,
		.speed_hz      = 0,
		.bits_per_word = 0,
	};

	while ((opt = getopt_long(argc, argv, "d:s:b:m:n:B:rhv", options, &long_index)) >= 0) {
		switch(opt) {
			case 'h':
				display_usage(argv[0]);
				exit(EXIT_SUCCESS);

			case 'v':
				fprintf(stderr, "%s - %s\n", project, VERSION);
				exit(EXIT_SUCCESS);

			case 'd':
				device = optarg;
				break;

			case 'm':
				if ((sscanf(optarg, "%d", &spi_mode) != 1)
				 || (spi_mode < 0) || (spi_mode > 3)) {
					fprintf(stderr, "%s: wrong SPI mode ([0-3])\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				break;

			case 'b':
				if ((sscanf(optarg, "%d", &block_size) != 1)
				 || (block_size <= 0)) {
					fprintf(stderr, "%s: wrong blocksize\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				break;

			case 's':
				if ((sscanf(optarg, "%d", &speed) != 1)
				 || (speed < 10) || (speed > 100000000)) {
					fprintf(stderr, "%s: Invalid speed\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				break;

			case 'B':
				if ((sscanf(optarg, "%d", &bits_per_word) != 1)
				 || (bits_per_word < 7)) {
					fprintf(stderr, "%s: wrong bits per word value [7...]\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				break;

			case 'n':
				if ((sscanf(optarg, "%d", &blocks_count) != 1)
				 || (blocks_count < -1)) {
					fprintf(stderr, "%s: wrong block number\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				break;

			default:
				fprintf(stderr, "%s: wrong option. Use -h for help.\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (((rx_buffer = malloc(block_size)) == NULL)
	 || ((tx_buffer = malloc(block_size)) == NULL)) {
		fprintf(stderr, "%s: not enough memory to allocate two %d bytes buffers\n",
		                argv[0], block_size);
		exit(EXIT_FAILURE);
	}

	memset(rx_buffer, 0, block_size);
	memset(tx_buffer, 0, block_size);

	transfer.rx_buf = (unsigned long)rx_buffer;
	transfer.tx_buf = (unsigned long)tx_buffer;
	transfer.len = block_size;

	if (device == NULL) {
		fprintf(stderr, "%s: no device specified (use option -h for help).\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(device, O_RDONLY);
	if (fd < 0) {
		perror(device);
		exit(EXIT_FAILURE);
	}

	if (speed != -1) {
		if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &orig_speed) < 0) {
			perror("SPI_IOC_RD_MAX_SPEED_HZ");
			exit(EXIT_FAILURE);
		}

		if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
			perror("SPI_IOC_WR_MAX_SPEED_HZ");
			exit(EXIT_FAILURE);
		}
	}

	if (spi_mode != -1) {
		if (ioctl(fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
			perror("SPI_IOC_WR_MODE");
			exit(EXIT_FAILURE);
		}
	}

	if (bits_per_word != -1) {
		if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
			fprintf(stderr, "Unable to set bits to %d\n", bits_per_word);
			perror("SPI_IOC_WR_BITS_PER_WORD");
			exit(EXIT_FAILURE);
		}
	}

	while ((blocks_count > 0) || (blocks_count == -1)) {
		for (offset = 0; offset < block_size; offset += nb) {
			nb = read(STDIN_FILENO, & (tx_buffer[offset]), block_size - offset);
			if (nb <= 0)
				break;
		}
		if ((nb <= 0) && (offset == 0))
			break;

		transfer.len = offset;
		if (ioctl(fd, SPI_IOC_MESSAGE(1), & transfer) < 0) {
			perror("SPI_IOC_MESSAGE");
			break;
		}
		if (write(STDOUT_FILENO, rx_buffer, offset) <= 0)
			break;
		if (blocks_count > 0)
			blocks_count --;
	}

	if (orig_speed != -1) {
		if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &orig_speed) < 0) {
			perror("SPI_IOC_WR_MAX_SPEED_HZ");
			exit(EXIT_FAILURE);
		}
	}

	free(rx_buffer);
	free(tx_buffer);

	if (blocks_count != 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

