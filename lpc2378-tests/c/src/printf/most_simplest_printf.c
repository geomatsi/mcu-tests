/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static void printchar(char **str, int c)
{
	**str = c;
	++(*str);
}

static int prints(char **out, const char *string)
{
	int pc = 0;

	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}

	return pc;
}

static int printi(char **out, int i, int b, int sign int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int t;

	unsigned int u = i;
	int neg = 0;
	int  pc = 0;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf);
	}

	if (sign && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg)
		*--s = '-';

	return pc + prints (out, s);
}

static int print(char **out, int *varg)
{
	char scr[2];

	char *format = (char *)(*varg++);
	int pc = 0;

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			if (*format == '\0') break;
			if (*format == '%') goto symbol;
			
			if( *format == 's' ) {
				char *s = *((char **)varg++);
				pc += prints (out, s ? s : "(null)");
				continue;
			}

			if( *format == 'd' ) {
				pc += printi (out, *varg++, 10, 1, 'a');
				continue;
			}

			if( *format == 'x' ) {
				pc += printi (out, *varg++, 16, 0, 'a');
				continue;
			}

			if( *format == 'X' ) {
				pc += printi (out, *varg++, 16, 0, 'A');
				continue;
			}

			if( *format == 'u' ) {
				pc += printi (out, *varg++, 10, 0, 'a');
				continue;
			}

			if( *format == 'c' ) {
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += prints (out, scr);
				continue;
			}
		} else {
		symbol:
			printchar (out, *format);
			++pc;
		}
	}

	if (out) 
		**out = '\0';

	return pc;
}

int sprintf(char *out, const char *format, ...)
{
	int *varg = (int *)(&format);
	return print(&out, varg);
}

int main(void)
{
	char *str = "Hello world!";
	char buf[100] = {0};
	int a = 0xABC;
	int b = 100;
	char c = '@';

	sprintf(buf, "str: %s\n", str);
	puts(buf);
	
	sprintf(buf, "int: %d\n", b);
	puts(buf);
	
	sprintf(buf, "hex: %x\n", a);
	puts(buf);
	
	sprintf(buf, "HEX: %X\n", a);
	puts(buf);
	
	sprintf(buf, "char: %c\n", c);
	puts(buf);
	
	sprintf(buf, "str: %s, int = %d, hex =0x%x\n", str, a, b);
	puts(buf);
	
	return 0;
}

