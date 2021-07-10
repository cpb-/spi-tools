/*
 * spidev tools.
 *
 * (c) 2014-2021 Christophe BLAESS <christophe.blaess@logilin.fr>
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

#ifndef SPI_TOOLS_H
#define SPI_TOOLS_H


typedef struct spi_config {

    int        spi_mode;      // [0-3]  (-1 when not configured).
    int        lsb_first;     // {0,1}  (-1 when not configured).
    int        bits_per_word; // [7...] (-1 when not configured).
    int        spi_speed;     // 0 when not configured.
    int        spi_ready;     // {0,1}  (-1 when not configured).

} spi_config_t;


int Read_spi_configuration(int fd, spi_config_t *config);

int Write_spi_configuration(int fd, spi_config_t *config);

int Parse_spi_mode(const char *optarg, spi_config_t *config);

int Parse_lsb_first(const char *optarg, spi_config_t *config);

int Parse_spi_speed(const char *optarg, spi_config_t *config);

int Parse_spi_ready(const char *optarg, spi_config_t *config);

int Parse_spi_bits_per_word(const char *optarg, spi_config_t *config);

int Transfer_spi_buffers(int fd, void *tx_buffer, void *rx_buffer, size_t length);


#endif
