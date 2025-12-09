#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>

enum {
	TOKEN_LEFT_ROTATE = 128,
	TOKEN_RIGHT_ROTATE,
	TOKEN_INT_LITERAL,
};

static char *map;

static int tk; /* token */
static int tk_value; /* token value */
static char *p; /* character iterator */

static void die(int status, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	exit(status);
}

static void next(void)
{
	while((tk = *p) != 0) {
		p++;
		if (tk == 'L') {
			tk = TOKEN_LEFT_ROTATE;
			return;
		}

		if (tk == 'R') {
			tk = TOKEN_RIGHT_ROTATE;
			return;
		}

		if (tk >= '0' && tk <= '9') {
			tk_value = tk - '0';
			for (; *p >= '0' && *p <= '9'; p++) {
				tk_value = (tk_value*10) + (*p - '0');
			}
			tk = TOKEN_INT_LITERAL;
			return;
		}
		switch(tk) {
		case '\n':
			return;
		}
	}
	return;
}

int main(void)
{
	int fd;
	struct stat st;
	int dial = 50;
	int password = 0;
	if ((fd = open("input", O_RDONLY)) == -1) {
		die(-1, "input file not found\n");
	}
	fstat(fd, &st);
	if ((map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		close(fd);
		die(-2, "cannot map input file\n");
	}
	p = map;

	/* we can iterate the file line by line and sscanf() each line with a
 	 * format string of "%d %d". also, utilizing mmap() and other stuff 
 	 * will tie this code to linux only. but whatever. */

	tk = '\n';
	while(tk) {
		bool left_rotate = true;

		/* first token is either an L or R rotation */
		next();
		if (tk == 0)
			break;

		if (tk == '\n')
			continue;

		if (tk != TOKEN_LEFT_ROTATE && tk != TOKEN_RIGHT_ROTATE) {
			die(-1, "illegal rotation format encountered.\n");
		}

		if (tk == TOKEN_RIGHT_ROTATE)
			left_rotate = false;

		/* second token should be an int literal */
		next();

		if (tk != TOKEN_INT_LITERAL) {
			die(-1, "illegal rotation value.\n");
		}

		if (left_rotate) 
			dial = (dial - tk_value) + 100;
		else
			dial = (dial + tk_value);
		dial = dial % 100;
		password += (dial == 0) ? 1 : 0;
	}
	
	printf("password: %d\n", password);

	munmap(map, st.st_size);
	close(fd);
	return 0;
}

