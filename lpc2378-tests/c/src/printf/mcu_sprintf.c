#define PRINT_BUF_LEN 12
#define PAD_RIGHT 1
#define PAD_ZERO 2

static void printchar(char **str, int c)
{
	**str = c;
	++(*str);
}

static int prints(char **out, const char *string, int width, int pad)
{
	int pc = 0, padchar = ' ';

	if (width > 0) {
		const char *ptr;
		int len = 0;
		
		for (ptr = string; *ptr; ++ptr)
			++len;
		
		if (len >= width)
			width = 0;
		else
			width -= len;
	
		if (pad & PAD_ZERO)
			padchar = '0';
	}

	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int t, neg = 0, pc = 0;
	unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print(char **out, int *varg)
{
	int width, pad;
	int pc = 0;
	char *format = (char *)(*varg++);
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;

			if (*format == '\0')
				break;

			if (*format == '%')
				goto out;

			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}

			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}

			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}

			if( *format == 's' ) {
				char *s = *((char **)varg++);
				pc += prints (out, s ? s : "(null)", width, pad);
				continue;
			}

			if( *format == 'd' ) {
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}

			if( *format == 'x' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}

			if( *format == 'X' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}

			if( *format == 'u' ) {
				pc += printi (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}

			if( *format == 'c' ) {
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		} else {
		out:
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
	char buf[80];

	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;

	unsigned int bs = sizeof(int)*8;
	int mi = (1 << (bs-1)) + 1;

	sprintf(buf, "%s\n", ptr);
	puts(buf);
	sprintf(buf, "printf test\n");
	puts(buf);

	sprintf(buf, "%s is null pointer\n", np);
	puts(buf);

	sprintf(buf, "%d = 5\n", i);
	puts(buf);

	sprintf(buf, "%d = - max int\n", mi);
	puts(buf);

	sprintf(buf, "char %c = 'a'\n", 'a');
	puts(buf);

	sprintf(buf, "hex %x = ff\n", 0xff);
	puts(buf);

	sprintf(buf, "hex %02x = 00\n", 0);
	puts(buf);

	sprintf(buf, "signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	puts(buf);

	sprintf(buf, "%d %s(s)%", 0, "message");
	puts(buf);

	sprintf(buf, "\n");
	puts(buf);

	sprintf(buf, "%d %s(s) with %%\n", 0, "message");
	puts(buf);

	sprintf(buf, "justif: \"%-10s\"\n", "left");
	puts(buf);

	sprintf(buf, "justif: \"%10s\"\n", "right");
	puts(buf);

	sprintf(buf, " 3: %04d zero padded\n", 3);
	puts(buf);

	sprintf(buf, " 3: %-4d left justif.\n", 3);
	puts(buf);

	sprintf(buf, " 3: %4d right justif.\n", 3);
	puts(buf);

	sprintf(buf, "-3: %04d zero padded\n", -3);
	puts(buf);

	sprintf(buf, "-3: %-4d left justif.\n", -3);
	puts(buf);

	sprintf(buf, "-3: %4d right justif.\n", -3);
	puts(buf);

	return 0;
}
