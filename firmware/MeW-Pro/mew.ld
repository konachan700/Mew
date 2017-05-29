MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH =  512K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

INCLUDE libopencm3_stm32f2.ld
