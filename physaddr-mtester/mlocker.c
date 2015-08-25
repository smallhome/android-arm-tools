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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

int main (int argc, char** argv) {
    unsigned long start, end;

    if (argc == 1) {
        printf("Usage: %s range [ranges]\n"
               "  mmap() the specified physical address ranges.\n"
               "  The ranges are specified as colon-separated start/end hex "
               "addresses (inclusive)\n."
               "  For example 0x07df0000:0x07e0ffff.\n");
        exit(1);
    }
    
    int devmem = open("/dev/mem", O_RDONLY);
    if (devmem < 0) {
        perror("open /dev/mem");
        exit(1);
    }

    int sum = 0;
    for (int i = 1; i < argc; ++i) {
        if (sscanf(argv[i], "0x%lx:0x%lx", &start, &end) != 2 ||
            start >= end) {
            fprintf(stderr, "invalid spec %s\n", argv[i]);
            exit(1);
        }

        int len = 1 + (end - start);
        void* buf = mmap(0, len, PROT_READ, MAP_SHARED,
                         devmem, start);
        if (!buf) {
            perror("mmap");
            exit(1);
        }

        // I thought that at a minimum we'd need to mlock() buf, and keep
        // this process running forever. But empirically just the act of
        // mmap() on /dev/mem locks the memory permanently, even after the
        // process exits.
        
        sum += len;
    }

    printf("locked %d bytes\n", sum);
}
