# spi-tools

This package contains some simple command line tools to help using Linux spidev devices.

Version 1.2.0

## Content

### `spi-config`

Query or set the SPI configuration (mode, speed, bits per word, etc.)

### `spi-pipe`

Send and receive data simultaneously to and from a SPI device.

## License

All project files are released under the GNU General Public License, version 2 only (GPL-2.0-only). See `LICENSE` file for details.

## Author

Christophe Blaess
https://www.blaess.fr/christophe

## Installation

There's two ways to install `spi-tools`: using Autotools or using CMake.

### Autotools

First, get the latest version on https://github.com/cpb-/spi-tools.git.
Then enter the directory and execute:

```
$ autoreconf -fim
$ ./configure
$ make
```

Then you can run `make install` (probably with `sudo`) to install them and the man pages.

If you have to use a cross-compilation toolchain, add the `--host` option to
the `./configure` command, as in `./configure --host=arm-linux`. This is the
prefix to be inserted before all the toolchain commands (giving for example
`arm-linux-gcc`).

You can use `make uninstall` (with `sudo`) to remove the installed files.

### CMake

Build outside the source directory (CMake 3.10 or later):

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
cmake --build .
ctest --output-on-failure
```

Install the tools and manual pages with `cmake --build . --target install`.
For a staged package installation, use
`DESTDIR=/path/to/stage cmake --build . --target install`.
The `CMAKE_INSTALL_BINDIR` and `CMAKE_INSTALL_MANDIR` options select the
installation directories relative to the prefix. Cross compilation uses
`-DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake`.

For example, with the Android NDK, configure a separate build directory:

```sh
cmake -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
      -DANDROID_PLATFORM=android-21 -DANDROID_ABI=armeabi-v7a ..
cmake --build .
```

### Android platform (Soong)

The `Android.bp` file builds the two device binaries directly, without a shared
library. From an initialized Android platform build environment, run
`m spi_tools`. Device permissions must allow access to the spidev device.

### Development and releases

`VERSION` is the authoritative version for Autotools, CMake and Soong. Update it
and the version shown in this README when preparing a release, then run
`autoreconf -fi` to regenerate the release's `configure` script.
Run `make check` and `make distcheck` for Autotools, and `ctest` for CMake.
For an unoptimized debug build, configure with
`CFLAGS='-g -O0' ./configure --enable-debug`.
The regression tests require Python 3 and a native C compiler; they simulate
spidev without requiring hardware. Cross builds skip these native tests.

## Usage

### spi-config usage

#### options

* `-d --device=<dev>`  use the given spi-dev character device.
* `-q --query`         print the current configuration.
* `-m --mode=[0-3]`    use the selected spi mode.
* `-l --lsb={0,1}`     LSB first (1) or MSB first (0).
* `-b --bits=[7-255]`   bits per word.
* `-s --speed=<int>`   set the speed in Hz.
* `-r --spirdy={0,1}`  set the SPI_READY spi mode flag.
* `-w --wait`          block, keeping the file descriptor open.
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

Note: on some platforms, the speed is reset to a default value when the file descriptor is closed.
To avoid this, one can use the `-w` option that keep the file descriptor open. For example:

```
$ spi-config -d /dev/spidev0.0 -s 10000000 -w &
$ PID=$!
```

And when you don't need the SPI device anymore:

```
$ kill $PID
```

### spi-pipe usage

#### Options

* `-d --device=<dev>`    use the given spi-dev character device.
* `-m --mode=[0-3]`      use the selected spi mode.
* `-s --speed=<speed>`   Maximum SPI clock rate (in Hz).
* `-l --lsb={0,1}`       LSB first (1) or MSB first (0).
* `-B --bits=[7-255]`     bits per word.
* `-r --spirdy={0,1}`    set the SPI_READY spi mode flag.
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

#### Send data through the SPI link

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

#### Send binary commands through the SPI link

You can use the shell `printf` command to format binary data to send.

For example, to send the bytes sequence 0x01-0x82-0xF3 and see the reply, use:

```
$ printf '\x01\x82\xF3' | spi-pipe -d /dev/spidev0.0 | hexdump -C
```

`spi-pipe` restores the previous configuration on normal exit, I/O errors,
and handled SIGINT, SIGTERM or SIGPIPE signals. Signal handlers only request
termination; restoration happens in the main program. On a handled signal,
the exit status is 128 plus the signal number. SIGKILL and uninterruptible
kernel operations cannot be handled this way.

A normal end of input is successful unless `-n` requested more blocks than
were available. A final partial block is transferred and counts as one block.
