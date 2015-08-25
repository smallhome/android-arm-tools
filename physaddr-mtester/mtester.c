// Copyright (C) 2011 by Juho Snellman. Standard MIT license:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <alloca.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int devmem;

// Find offset in /dev/mem which contains exactly the same data as @page.
void find_physaddr(char* page, int page_bytes) {
    char* read_buf = alloca(page_bytes);
    lseek(devmem, 0, SEEK_SET);
    
    int offset = 0;
    int count = 0;
    
    while (read(devmem, read_buf, page_bytes) == page_bytes) {
        if (memcmp(page, read_buf, page_bytes) == 0) {
            printf("0x%08x:0x%08x\n", offset, offset + (page_bytes - 1));
            count++;
        }

        offset += page_bytes;
    }

    if (!count) {
        printf("Physical page not found\n");
    }
    if (count > 1) {
        printf(";; Found multiple physical pages, results unreliable\n");
    }
}

int main (int argc, char** argv) {
    size_t buf_mbytes;
    size_t buf_bytes;
    
    int page_bytes = getpagesize();
    
    if (argc != 2 ||
        sscanf(argv[1], "%ld", &buf_mbytes) != 1) {
        printf("Usage: %s mbytes\n");
        exit(1);
    }
    buf_bytes = buf_mbytes * 1024 * 1024;

    devmem = open("/dev/mem", O_RDONLY);
    if (devmem < 0) {
        perror("open /dev/mem");
        exit(1);
    }

    printf(";; Allocating locked memory\n");
    
    char* buf = NULL;
    posix_memalign((void**) &buf, page_bytes, buf_bytes);

    if (!buf) {
        perror("posix_memalign");
        exit(1);
    }
    if (mlock(buf, buf_bytes) < 0) {
        perror("mlock");
        exit(1);
    }

    char* rand_data = malloc(page_bytes);
    for (int j = 0; j < page_bytes; ++j) { 
        rand_data[j] = rand();
    }

    printf(";; Filling memory with random pattern\n");
    for (int i = 0; i < buf_bytes / page_bytes; ++i) {
        char* page = buf + page_bytes * i;
        memcpy(page, rand_data + 1, page_bytes);
    }
    
    printf(";; Reading back memory\n");

    int ret = 0;
    for (int i = 0; i < buf_bytes / page_bytes; ++i) {
        char* page = buf + page_bytes * i;
        if (memcmp(page, rand_data + 1, page_bytes) != 0) {
            find_physaddr(page, page_bytes);
            ret = 1;
        }
    }

    exit(ret);
}

