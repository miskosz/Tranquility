#include <cstdio>
#include <cstring>
#include <cstdlib>

/**
 * Unsafe version of uniq.
 * Deletes duplicate consecutive rows on input. Rows are considered duplicate
 * if the integer value of the first field is the same.
 */

int main() {
	char line[1000], *tail;
	int num, oldnum = -1;
	
	while (gets(line)) {
		tail = strchr(line, ' ');
		tail[0] = '\0';
		tail++;
		num = atoi(line);
		if (num != oldnum) {
			printf("%d %s\n", num, tail);
			oldnum = num;
		}
	}
}
