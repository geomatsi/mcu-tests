void delay(unsigned int cycles)
{

	while (cycles--) {
		__asm__ volatile ("nop");
	}
}

void main(void)
{
    while (1) {
	    delay(200000);
    }
}
