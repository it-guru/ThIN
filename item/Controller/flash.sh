#!/bin/sh
esptool.py --port socket://10.123.100.41:2323 write_flash 0x00000 Controller.ino.generic.bin
#esptool.py write_flash --flash_size 4m 0x00000 Point1.ino.generic.bin
