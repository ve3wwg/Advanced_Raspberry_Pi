sdcc --std-sdcc99 -mmcs51 --stack-size 64 --model-small --xram-loc 0x0000 --xram-size 0x5000 --iram-size 0x0100 --code-loc 0x0000 -I../ezusb blink.c
