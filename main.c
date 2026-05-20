#define _GNU_SOURCE
#define _XOPEN_SOURCE 500 // Required for nftw

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ftw.h>
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

static void *pat = NULL;
static size_t pat_len = 0;

int scan_file(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
	if (tflag != FTW_F && tflag != FTW_SL)
		return 0;

	mf_t mf;
	int res = map_file(&mf,fpath,0,0);
	if(res)
		return res;

	if(mf.len < pat_len)
		goto skip;

	const void *end = mf.mem + mf.len;
	const void *p = mf.mem;
	for(size_t left; (left=end-p);p++) {
		p = memmem(p,left,pat,pat_len);
		if(!p)
			break;
		printf("%s: found @%lx\n", fpath, p-mf.mem);
	}

skip:
	unmap_file(&mf);
	return 0;
}

int main(int argc, char **argv) {
	assert(argc>2);
	size_t hexlen = strlen(argv[1]);
	pat = alloca(hexlen/2);
	pat_len = hexparse(pat,hexlen/2,argv[1]);
	assert(pat_len == (hexlen/2));
	if (nftw(argv[2], scan_file, 20, FTW_PHYS) == -1) {
		perror("nftw");
		return 1;
	}
	return 0;
}
