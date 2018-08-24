#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

	FILE* fp;

	while(1) {

		fp = fopen("/tmp/test-put", "w");
		fwrite("test", 4, 1, fp);
		fclose(fp);
		sleep(1);
	}

	return 0;
}
