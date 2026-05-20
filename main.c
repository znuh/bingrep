#define _GNU_SOURCE

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mmap.h"

int hex2nibble(char c) {
	if((c>='0') && (c<='9'))
		return c-'0';
	else if((c>='a') && (c<='f'))
		return c-'a'+10;
	else if((c>='A') && (c<='F'))
		return c-'A'+10;
	else
		return -1;
}

size_t hexparse(void *d, size_t bufsize, const char *hex) {
	uint8_t *dst = d;
	uint8_t *dstart = dst, byte=0;
	size_t i=0;

	for(;bufsize && *hex;hex++) {
		int v = hex2nibble(*hex);
		if(v<0)
			continue;
		byte<<=4;
		byte|=v;
		if(++i<2)
			continue;
		*dst++ = byte;
		i = 0;
		bufsize--;
	}

	return dst - dstart;
}

int scan_file(const char *fname, const void *pat, size_t pat_len) {
	mf_t mf;
	int res = map_file(&mf,fname,0,0);
	if(res)
		return res;

	const void *end = mf.mem + mf.len;
	const void *p = mf.mem;
	for(size_t left; (left=end-p);p++) {
		p = memmem(p,left,pat,pat_len);
		if(!p)
			break;
		printf("%s: found @%lx\n", fname, p-mf.mem);
	}

	unmap_file(&mf);
	return 0;
}

int main(int argc, char **argv) {
	assert(argc>2);
	size_t hexlen = strlen(argv[1]);
	void *pat = alloca(hexlen/2);
	size_t patlen = hexparse(pat,hexlen/2,argv[1]);
	assert(patlen == (hexlen/2));
	int res = scan_file(argv[2], pat, patlen);
	assert(!res);
	return 0;
}
