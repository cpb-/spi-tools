spi-tools
=========

This package contains some simple command line tools to help using Linux spidev devices.

Content
-------

### `spi-config`
Query or set the SPI configuration (mode, speed, bits per word, etc.)

### `spi-pipe`
Send and receive data simultaneously to and from a SPI device.

License
-------
The tools are released under the GPLv2 license. See `LICENSE` file for details.

Author
------
Christophe Blaess
http://www.blaess.fr/christophe

Installation
------------
First, get the latest version on https://github.com/cpb-/spi-tools.git

    # autoreconf -fim
    # ./configure

Simply do a `make` to build the tools, then `make install` to install them and the man pages.
If you have to use a cross-compilation toolchain, simply fill the `CROSS_COMPILE` environment variable with the cross-compiler prefix.
You can use `make uninstall` to remove the installed files.

Usage
-----
### spi-config usage
#### options
* `-d --device=<dev>`  use the given spi-dev character device.
* `-q --query`         print the current configuration.
* `-m --mode=[0-3]`    use the selected spi mode.
* `-l --lsb={0,1}`     LSB first (1) or MSB first (0).
* `-b --bits=[7...]`   bits per word.
* `-r --spirdy={0,1}`   set the SPI_READY spi mode flag.
* `-s --speed=<int>`   set the speed in Hz.
* `-h --help`          help screen.
* `-v --version`       display the version number.

#### Read the current configuration

```
$ spi-config -d /dev/spidev0.0 -q
/dev/spidev0.0: mode=0, lsb=0, bits=8, speed=500000
$
```

#### Change the clock frequency and read it again

```
$ spi-config -d /dev/spidev0.0 -s 10000000
$ spi-config -d /dev/spidev0.0 -q
/dev/spidev0.0: mode=0, lsb=0, bits=8, speed=10000000
$
```

### spi-pipe usage
#### Options
* `-d --device=<dev>`    use the given spi-dev character device.
* `-b --blocksize=<int>` transfer block size in byte.
* `-n --number=<int>`    number of blocks to transfer.
* `-h --help`            help screen.
* `-v --version`         display the version number.

#### Send and receive simultaneously
Sending data from `command-1` to SPI link and receiving data from SPI link to `command-2`

```
$ command_1 | spi-pipe -d /dev/spidev0.0 | command_2
```

Note that `command_1`, `command_2` and `spi-pipe` run simultaneously in three parallel processes.

#### Send data to the SPI link

```
$ command_1 | spi-pipe -d /dev/spidev0.0
```

#### Receive data from the SPI link

```
$ spi-pipe -d /dev/spidev0.0 < /dev/zero | command_2
```

You can also use `command_2 < /dev/spidev0.0` but with `spi-pipe` you control what is sent to the device (always `0` in this case).

#### Read 40 blocks of 4 bytes from the SPI link
```
$ spi-pipe -d /dev/spidev0.0 -b 4 -n 40 < /dev/zero | command_2
```


