#include "mheads.h"
#include "lheads.h"


int main()
{
	unsigned char result[16];

	memset(result, 0x0, 16);

	const unsigned char *tt = "test";

	md5_signature(tt, (unsigned int)strlen((char*)tt), result);

	printf("%s\n", result);

	return 0;
}
