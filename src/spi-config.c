/*
 * spidev configuration tool.
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
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include "config.h"

static char * project = "spi-config";


typedef struct spi_config {
	int        mode;  // [0-3]  (-1 when not configured).
	int        lsb;   // {0,1}  (-1 when not configured).
	int        bits;  // [7...] (-1 when not configured).
	uint32_t   speed; // 0 when not configured.
	int        spiready;   // {0,1}  (-1 when not configured).
} spi_config_t;



static void display_usage(const char * name)
{
	fprintf(stderr, "usage: %s options...\n", name);
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "    -d --device=<dev>  use the given spi-dev character device.\n");
	fprintf(stderr, "    -q --query         print the current configuration.\n");
	fprintf(stderr, "    -m --mode=[0-3]    use the selected spi mode:\n");
	fprintf(stderr, "             0: low idle level, sample on leading edge,\n");
	fprintf(stderr, "             1: low idle level, sample on trailing edge,\n");
	fprintf(stderr, "             2: high idle level, sample on leading edge,\n");
	fprintf(stderr, "             3: high idle level, sample on trailing edge.\n");
	fprintf(stderr, "    -l --lsb={0,1}     LSB first (1) or MSB first (0).\n");
	fprintf(stderr, "    -b --bits=[7...]   bits per word.\n");
	fprintf(stderr, "    -s --speed=<int>   set the speed in Hz.\n");
	fprintf(stderr, "    -r --spirdy={0,1}  consider SPI_RDY signal (1) or ignore it (0).\n");
	fprintf(stderr, "    -h --help          this screen.\n");
	fprintf(stderr, "    -v --version       display the version number.\n");

}

int main (int argc, char * argv[])
{
	int opt;
	int long_index = 0;

	static struct option options[] = {
		{"device",   required_argument, NULL,  'd' },
		{"query",    no_argument,       NULL,  'q' },
		{"help",     no_argument,       NULL,  'h' },
		{"version",  no_argument,       NULL,  'v' },
		{"mode",     required_argument, NULL,  'm' },
		{"lsb",      required_argument, NULL,  'l' },
		{"bits",     required_argument, NULL,  'b' },
		{"speed",    required_argument, NULL,  's' },
		{"spirdy",   required_argument, NULL,  'r' },
		{0,         0,                 0,  0 }
	};

	spi_config_t  new_config = { -1, -1, -1, 0, -1 };
	spi_config_t  config;
	char *        device = NULL;
	int           fd;
	int           val;
	uint8_t       byte;
	uint32_t      u32;
	int           query_only = 0;

	while ((opt = getopt_long(argc, argv, "d:qhvm:l:b:s:r:", options, &long_index)) >= 0) {
		switch(opt) {
			case 'q':
				query_only = 1;
				break;
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
				if ((sscanf(optarg, "%d", & val) != 1)
				 || (val < 0) || (val > 3)) {
					fprintf(stderr, "%s: wrong SPI mode ([0-3])\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				new_config.mode = val;
				break;

			case 'l':
				if ((sscanf(optarg, "%d", & val) != 1)
				 || (val < 0) || (val > 1)) {
					fprintf(stderr, "%s: wrong LSB first value ([0,1])\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				new_config.lsb = val;
				break;

			case 'b':
				if ((sscanf(optarg, "%d", & val) != 1)
				 || (val < 7)) {
					fprintf(stderr, "%s: wrong bits per word value [7...]\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				new_config.bits = val;
				break;

			case 's':
				if ((sscanf(optarg, "%d", & val) != 1)
				 || (val < 10)
				 || (val > 100000000)) {
					fprintf(stderr, "%s: wrong speed value (Hz)\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				new_config.speed = val;
				break;

			case 'r':
				if ((sscanf(optarg, "%d", & val) != 1)
				 || (val < 0) || (val > 1)) {
					fprintf(stderr, "%s: wrong SPI_RDY value ([0,1])\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				new_config.spiready = val;
				break;

			default:
				fprintf(stderr, "%s: wrong option. Use -h for help.\n", argv[0]);
				exit(EXIT_FAILURE);
		}				
	}

	if (device == NULL) {
		fprintf(stderr, "%s: no device specified (use option -h for help).\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(device, O_RDONLY);
	if (fd < 0) {
		perror(device);
		exit(EXIT_FAILURE);
	}

	// Read the previous configuration.
	if (ioctl(fd, SPI_IOC_RD_MODE, & byte) < 0) {
		perror("SPI_IOC_RD_MODE");
		exit(EXIT_FAILURE);
	}
	config.mode = byte;
	config.spiready = ((config.mode & SPI_READY) ? 1 : 0);
	// clear the upper flag bits to be left with mode number
	config.mode &= 0x3;
	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, & byte) < 0) {
		perror("SPI_IOC_RD_LSB_FIRST");
		exit(EXIT_FAILURE);
	}
	config.lsb = (byte == SPI_LSB_FIRST ? 1 : 0);
	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, & byte) < 0) {
		perror("SPI_IOC_RD_BITS_PER_WORD");
		exit(EXIT_FAILURE);
	}
	config.bits = (byte == 0 ? 8 : byte);
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, & u32) < 0) {
		perror("SPI_IOC_RD_MAX_SPEED_HZ");
		exit(EXIT_FAILURE);
	}
	config.speed = u32;

	if (query_only) {
		fprintf(stdout, "%s: mode=%d, lsb=%d, bits=%d, speed=%d, spiready=%d\n",
		        device, config.mode, config.lsb, config.bits, config.speed, config.spiready);
		exit(EXIT_SUCCESS);
	}

	// Set the new configuration.
	if ((config.spiready != new_config.spiready) && (new_config.spiready == 1)) {
		new_config.mode |= SPI_READY;
	}
	if ((config.mode != new_config.mode) || (config.spiready != new_config.spiready)) {
		// In case only the spiready flag was changed
		if (new_config.mode == -1) {
			new_config.mode = config.mode;
		}
		if (new_config.spiready == 1) {
			new_config.mode |= SPI_READY;
		}
		byte = new_config.mode;
		if (ioctl(fd, SPI_IOC_WR_MODE, & byte) < 0) {
			perror("SPI_IOC_WR_MODE");
			exit(EXIT_FAILURE);
		}
	}
	if ((config.lsb != new_config.lsb) && (new_config.lsb != -1)) {
		byte = (new_config.lsb ? SPI_LSB_FIRST : 0);
		if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, & byte) < 0) {
			perror("SPI_IOC_WR_LSB_FIRST");
			exit(EXIT_FAILURE);
		}
	}
	if ((config.bits != new_config.bits) && (new_config.bits != -1)) {
		byte = new_config.bits;
		if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, & byte) < 0) {
			fprintf(stderr, "Unable to set bits to %d\n", byte);
			perror("SPI_IOC_WR_BITS_PER_WORD");
			exit(EXIT_FAILURE);
		}
	}
	if ((config.speed != new_config.speed) && (new_config.speed != 0)) {
		u32 = new_config.speed;
		if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, & u32) < 0) {
			perror("SPI_IOC_WR_MAX_SPEED_HZ");
			fprintf(stderr, "Failed to set speed to %d\n", u32);
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

