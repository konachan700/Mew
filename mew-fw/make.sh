#!/bin/bash
gcc -lhidapi-libusb -lhidapi-hidraw -lcrypto -O2 -o mew-fw *.c
