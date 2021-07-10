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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#include "spi-tools.h"

static char *project = "spi-config";



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
	fprintf(stderr, "    -w --wait          block keeping the file descriptor open to avoid speed reset.\n");
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
		{"wait",     no_argument,       NULL,  'w' },
		{"mode",     required_argument, NULL,  'm' },
		{"lsb",      required_argument, NULL,  'l' },
		{"bits",     required_argument, NULL,  'b' },
		{"speed",    required_argument, NULL,  's' },
		{"spirdy",   required_argument, NULL,  'r' },
		{0,         0,                 0,  0 }
	};

	char *        device = NULL;
	spi_config_t  new_config;
	spi_config_t  config;
	int           fd;
	int           val;
	int           query_only = 0;
	int           wait = 0;

	new_config.spi_mode = -1;
	new_config.lsb_first = -1;
	new_config.bits_per_word = -1;
	new_config.spi_speed = -1;
	new_config.spi_ready = -1;

	while ((opt = getopt_long(argc, argv, "d:qhvwm:l:b:s:r:", options, &long_index)) >= 0) {
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
			case 'w':
				wait = 1;
				break;
			case 'm':
				if (Parse_spi_mode(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'l':
				if (Parse_lsb_first(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'b':
				if ((sscanf(optarg, "%d", &val) != 1)
				 || (val < 7)) {
					fprintf(stderr, "%s: wrong bits per word value [7...]\n", argv[0]);
					exit(EXIT_FAILURE);
				}
				new_config.bits_per_word = val;
				break;

			case 's':
				if (Parse_spi_speed(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
				break;

			case 'r':
				if (Parse_spi_ready(optarg, &new_config) != 0)
					exit(EXIT_FAILURE);
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

	if (Read_spi_configuration(fd, &config) != 0)
		exit(EXIT_FAILURE);

	if (query_only) {
		fprintf(stdout, "%s: mode=%d, lsb=%d, bits=%d, speed=%d, spiready=%d\n",
		        device, config.spi_mode, config.lsb_first, config.bits_per_word,
		        config.spi_speed, config.spi_ready);
		exit(EXIT_SUCCESS);
	}

	if (new_config.spi_mode == -1)
		new_config.spi_mode = config.spi_mode;

	if (new_config.lsb_first == -1)
		new_config.lsb_first = config.lsb_first;

	if (new_config.bits_per_word == -1)
		new_config.bits_per_word = config.bits_per_word;

	if (new_config.spi_speed == -1)
		new_config.spi_speed = config.spi_speed;

	if (new_config.spi_ready == -1)
		new_config.spi_ready = config.spi_ready;

	if (Write_spi_configuration(fd, &new_config) != 0)
		exit(EXIT_FAILURE);

	if (wait)
		for (;;)
			pause();

	return EXIT_SUCCESS;
}

