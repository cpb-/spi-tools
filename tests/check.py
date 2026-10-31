#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Native Linux regression tests; no SPI hardware required."""
import argparse
import os
from pathlib import Path
import re
import shlex
import signal
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--bin-dir", type=Path, required=True)
    args = parser.parse_args()
    source = args.source.resolve()
    binaries = args.bin_dir.resolve()
    version = (source / "VERSION").read_text().strip()
    assert re.fullmatch(r"\d+\.\d+\.\d+", version), version
    assert "Version " + version in (source / "README.md").read_text()

    with tempfile.TemporaryDirectory(prefix="spi-tools-test-") as tmp:
        tmp = Path(tmp)
        mock = tmp / "mock-spi.so"
        subprocess.run(shlex.split(os.environ.get("CC", "cc")) + [
            "-Wall", "-Wextra", "-Werror", "-shared", "-fPIC",
            str(source / "tests/mock-spi.c"), "-o", str(mock)], check=True)
        log = tmp / "ioctl.log"
        env = dict(os.environ, LD_PRELOAD=str(mock), SPI_TEST_LOG=str(log))
        count = 0

        def run(prog, options, data=b"a", status=0, output=None, extra=None):
            nonlocal count
            log.write_text("")
            result = subprocess.run(
                [str(binaries / prog), "-d", "/dev/null"] + options,
                input=data, capture_output=True, env=dict(env, **(extra or {})),
                timeout=5)
            assert result.returncode == status, (prog, options, result)
            if output is not None:
                assert result.stdout == output, (prog, options, result.stdout)
            count += 1
            return result, log.read_text()

        for prog in ("spi-config", "spi-pipe"):
            result = subprocess.run([str(binaries / prog), "--version"],
                                    capture_output=True, check=True)
            assert (prog + " - " + version + "\n").encode() in result.stderr
            options = {
                "-m": (["0", "3"], ["-1", "4"]),
                "-l": (["0", "1"], ["-1", "2"]),
                "-r": (["0", "1"], ["-1", "2"]),
                "-s": (["1", "100000000"], ["-1", "100000001"]),
                "-b" if prog == "spi-config" else "-B":
                    (["7", "255", "0x08"], ["6", "256"]),
            }
            if prog == "spi-pipe":
                options.update({
                    "-b": (["1", "4"], ["-1", "0", "4294967296"]),
                    "-n": (["-1", "0", "1"], ["-2", "2147483648",
                           "4294967295", "4294967296", "-4294967296"]),
                })
            for opt, (valid, invalid) in options.items():
                for value in valid:
                    run(prog, [opt, value])
                for value in invalid + ["", "junk", "8junk", "1.5", "1 ",
                                        "999999999999999999999999999999"]:
                    _, trace = run(prog, [opt, value], status=1)
                    assert trace == "", (prog, opt, value, trace)
            for invalid in (["oops"], ["--", "oops"], ["--unknown"],
                            ["--mode"], ["--speed="], ["--bits=8junk"]):
                _, trace = run(prog, invalid, status=1)
                assert trace == "", (prog, invalid, trace)
            run(prog, ["--spirdy=1"])

        # EOF, finite counts and final partial blocks.
        _, trace = run("spi-pipe", [], b"abc", output=b"abc")
        assert "speed:" not in trace, trace
        run("spi-pipe", [], b"", output=b"")
        run("spi-pipe", ["-b", "4"], b"abc", output=b"abc")
        run("spi-pipe", ["-n", "2"], b"abc", output=b"ab")
        run("spi-pipe", ["-n", "4"], b"abc", status=1, output=b"abc")
        run("spi-pipe", ["-n", "0"], b"abc", output=b"")
        run("spi-pipe", ["-b", "4"], b"abc", output=b"abc",
            extra={"SPI_TEST_EINTR": "1"})

        # All error paths must restore the configuration, including a failure
        # midway through applying a new configuration.
        for failure in ("READ", "TRANSFER", "CONFIG"):
            result, trace = run("spi-pipe", ["-s", "1000000", "-b", "4"],
                                b"abc", status=1, output=b"",
                                extra={"SPI_TEST_" + failure + "_ERROR": "1"})
            assert "speed:1000000\n" in trace, trace
            assert trace.endswith("speed:500000\n"), trace
            assert result.stderr, failure
        log.write_text("")
        with open("/dev/full", "wb") as full:
            result = subprocess.run(
                [str(binaries / "spi-pipe"), "-d", "/dev/null", "-s", "1000000"],
                input=b"a", stdout=full, stderr=subprocess.PIPE, env=env, timeout=5)
        assert result.returncode == 1, result
        assert log.read_text().endswith("speed:500000\n")

        # Signal an idle process with stdin still open. A watchdog prevents a
        # regression from hanging make check.
        for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGPIPE):
            log.write_text("")
            process = subprocess.Popen(
                [str(binaries / "spi-pipe"), "-d", "/dev/null", "-s", "1000000"],
                stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env=env)
            try:
                deadline = time.monotonic() + 5
                while "speed:1000000\n" not in log.read_text():
                    assert process.poll() is None, process.returncode
                    assert time.monotonic() < deadline, "configuration timeout"
                    time.sleep(0.01)
                process.send_signal(signum)
                assert process.wait(timeout=5) == 128 + signum
                assert log.read_text().endswith("speed:500000\n")
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait()
                process.stdin.close()
                process.stdout.close()
                process.stderr.close()

        # A real closed output pipe must also reach restoration.
        log.write_text("")
        read_fd, write_fd = os.pipe()
        os.close(read_fd)
        try:
            result = subprocess.run(
                [str(binaries / "spi-pipe"), "-d", "/dev/null", "-s", "1000000"],
                input=b"a", stdout=write_fd, stderr=subprocess.PIPE, env=env, timeout=5)
        finally:
            os.close(write_fd)
        assert result.returncode == 128 + signal.SIGPIPE, result
        assert log.read_text().endswith("speed:500000\n")
        print("Passed {} parsing/transfer cases, error restoration and signal tests".format(count))


if __name__ == "__main__":
    main()
