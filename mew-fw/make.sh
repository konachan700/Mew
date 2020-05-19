#!/bin/bash
gcc -lhidapi-libusb -lhidapi-hidraw -lcrypto -O2 -o mew-flash mew-flash.c
gcc -lhidapi-libusb -lhidapi-hidraw -lcrypto -O2 -o mew-verify mew-verify.c
chmod +x ./mew-flash
chmod +x ./mew-verify
