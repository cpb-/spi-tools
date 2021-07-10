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
#include "config.h"
#include "spi-tools.h"

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
	fprintf(stderr, "    -l --lsb={0,1}     LSB first (1) or MSB first (0).\n");
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
		{"lsb",       required_argument, NULL,  'l' },
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
	spi_config_t prev_config;
	spi_config_t new_config;

	int block_size    = -1;
	int blocks_count  = -1;
	int offset        =  0;
	int nb            =  0;

	new_config.spi_mode = -1;
	new_config.lsb_first = -1;
	new_config.bits_per_word = -1;
	new_config.spi_speed = -1;
	new_config.spi_ready = -1;


	while ((opt = getopt_long(argc, argv, "d:s:b:l:m:n:B:rhv", options, &long_index)) >= 0) {
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
				if (Parse_spi_mode(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'l':
				if (Parse_lsb_first(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 's':
				if (Parse_spi_speed(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'r':
				if (Parse_spi_ready(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'B':
				if (Parse_spi_bits_per_word(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'b':
				if ((sscanf(optarg, "%d", &block_size) != 1)
				 || (block_size <= 0)) {
					fprintf(stderr, "%s: wrong blocksize\n", argv[0]);
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

	if (device == NULL) {
		fprintf(stderr, "%s: no device specified (use option -h for help).\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(device, O_RDONLY);
	if (fd < 0) {
		perror(device);
		exit(EXIT_FAILURE);
	}

	if (Read_spi_configuration(fd, &prev_config) != 0)
		exit(EXIT_FAILURE);

	if (new_config.spi_mode == -1)
		new_config.spi_mode = prev_config.spi_mode;

	if (new_config.lsb_first == -1)
		new_config.lsb_first = prev_config.lsb_first;

	if (new_config.bits_per_word == -1)
		new_config.bits_per_word = prev_config.bits_per_word;

	if (new_config.spi_speed == -1)
		new_config.spi_speed = prev_config.spi_speed;

	if (new_config.spi_ready == -1)
		new_config.spi_ready = prev_config.spi_ready;

	if (Write_spi_configuration(fd, &new_config) != 0)
		exit(EXIT_FAILURE);

	while ((blocks_count > 0) || (blocks_count == -1)) {
		for (offset = 0; offset < block_size; offset += nb) {
			nb = read(STDIN_FILENO, & (tx_buffer[offset]), block_size - offset);
			if (nb <= 0)
				break;
		}
		if ((nb <= 0) && (offset == 0))
			break;

		if (Transfer_spi_buffers(fd, tx_buffer, rx_buffer, offset) != 0) {
			perror("SPI_IOC_MESSAGE");
			break;
		}
		if (write(STDOUT_FILENO, rx_buffer, offset) <= 0)
			break;
		if (blocks_count > 0)
			blocks_count --;
	}

	free(rx_buffer);
	free(tx_buffer);

	if (Write_spi_configuration(fd, &prev_config) != 0)
		exit(EXIT_FAILURE);

	if (blocks_count != 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
