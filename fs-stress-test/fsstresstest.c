/* 
 * filesystem stress test that simulates the activity
 * seen in a data logging applications
 *
 * Copyright (C) 2009 Cliff Brake <cbrake@bec-systems.com>
 *
 * we collect 528 bytes per second
 *
 * for 3500 hours, this is 3500 * 528 * 60 = 6GB data
 *
 * we plan to buffer this data and only write it once
 * every 60 seconds.
 *
 * This test runs the following loops:
 *
 * LOOP1 (repeat testing until max simhours is reached) 
 *   file_index = 0
 *   LOOP2 (write/verify files until we reach totaltest size)
 *     open new file for writing or verification
 *     LOOP3  
 *       LOOP4 write data to sim file
 *       if (maxfilesize) we create a new file when this is reached
 *      
 *     if (totaltestsize) break
 *
 *   if (reached maxsimhours) end test
 *   if (verify data failed, stop test) required so we can analyize data
 * 
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdarg.h>

void fslog(char * format, ...)
{
	char buffer[256];

	static FILE * logf;

	if (logf == NULL) {
		logf = fopen("fsstresstest.log", "w");
		if (logf == NULL) {
			printf("Error opening log file\n");
			exit(-1);
		}
	}

	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	fprintf(logf, "%s\n", buffer);
	printf("%s\n", buffer);
}

void display_help (void)
{
	printf("Usage: fsstresstest [OPTION]\n"
		"\n"
		"  -h, --maxsimhours    LOOP1: Maximum simulation time (hours), default 4000\n"
		"  -z, --totaltestsize  LOOP2: Total amount of data to write in test (MBytes), default 500\n"
		"  -m, --maxfilesize    LOOP3: Max file size (MBytes), default 10\n"
		"  -s, --size           LOOP4: Data size to write (bytes), default 528*60\n"
		"  -t, --delay          Delay time between writes (ms), default 0\n"
		"  -r, --noremount      Don't remount the SD card in sync mode\n"
		"  -d, --testdir        test directory, default SD card /media/mmcblk0p1\n"
		"  -v, --verifyfile     create verification file\n"
		"  -f, --stoponfail     Stop test on data verify failure\n"
		"  -g, --skipverify     Don't verify files\n");

	exit(0);
}
		

int maxsimhours = 4000;
int totaltestsize = 500;
int maxfilesize = 10;
int size = 526*60;
int delay = 0;
int noremount = 0;
char * testdir = "/media/mmcblk0p1";
int verifyfile = 0;
int stoponfail = 0;
int skipverify = 0;

void process_options(int argc, char * argv[])
{
	for (;;) {
		int option_index = 0;
		static const char *short_options = "h:z:m:s:t:rd:vfg";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 0},
			{"maxsimhours", required_argument, 0, 'h'},
			{"totaltestsize", required_argument, 0, 'z'},
			{"maxfilesize", required_argument, 0, 'm'},
			{"size", required_argument, 0, 's'},
			{"delay", required_argument, 0, 't'},
			{"noremount", no_argument, 0, 'r'},
			{"testdir", required_argument, 0, 'd'},
			{"verifyfile", no_argument, 0, 'v'},
			{"stoponfail", no_argument, 0, 'f'},
			{"skipverify", no_argument, 0, 'g'},
			{0,0,0,0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);

		if (c == EOF) {
			break;
		}

		switch (c) {
			case 0:
				display_help();
				break;
			
			case 'h':
				maxsimhours = atoi(optarg);
				break;
			case 'z':
				totaltestsize = atoi(optarg);
				break;
			case 'm':
				maxfilesize = atoi(optarg);
				break;
			case 's':
				size = atoi(optarg);
				break;
			case 't':
				delay = atoi(optarg);
				break;
			case 'r':
				noremount = 1;
				break;
			case 'd':
				testdir = optarg;
				break;
			case 'v':
				verifyfile = 1;
				break;
			case 'f':
				stoponfail = 1;
				break;
			case 'g':
				skipverify = 1;
				break;
		}

	}
}

/* returns array of bytes, that are formed by successive counting uints.
 * Must be free'd after use.  Counting starts at count and
 * count is updated.
 */
static char * counting_data(unsigned int * count, int len)
{
	char * r;
	int i;

	r = (char *) malloc(len);
	if (r == NULL) {
		fslog("error allocating buffer");
		exit(-1);
	};

	for (i=0; i < len/4; i++) {
		*(unsigned int *)(&r[i*4]) = *count;
		*count = *count + 1;
	}

	return r;
}

/* compares two arrays and returns index of failing byte, -1 if success */
static int byte_compare(char * a, char * b, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (a[i] != b[i]) {
			fslog("byte compare failed at offset 0x%x", i);
			fslog("Expected 0x%x, read 0x%x", a[i], b[i]);
			return i;
		}
	}

	return -1;
}

static int is_mmc_card_mounted(void)
{
	if (system("mount | grep \"mmcblk0p1\"") == 0) {
		return 1;
	} else return 0;
}

static int write_verify_test_file(int file_count, unsigned int * data_count, int verify,
		int * minutes, int * file_size)
{
	int ret = 0;
	*file_size = 0;

	FILE * fs;
	FILE * fs_b;

	char filename[100];
	char filename_b[100];
	char dir[100];

	sprintf(dir, "%s/fsstresstest_tmp", testdir);
	sprintf(filename, "%s/test_%i.dat", dir, file_count);
	sprintf(filename_b, "%s/test_%ib.dat", dir, file_count);

	char mkdir[100];
	sprintf(mkdir, "mkdir -p %s", dir);
	system(mkdir);

	if (!verify) {
		fslog("creating file %s", filename);
		fs = fopen(filename, "w");
		if (fs == NULL) {
			fslog("error opening %s", filename);
			exit(-1);
		}
		if (verifyfile) {
			fs_b = fopen(filename_b, "w");
			if (fs_b == NULL) {
				fslog("error opening %s", filename_b);
				exit(-1);
			}
		}
	} else {
		fslog("verifying file %s", filename);
		fs = fopen(filename, "r");
		if (fs == NULL) {
			fslog("error opening %s", filename);
		}
	}

	for (;;) {
		/* LOOP 3 */
		char * c = counting_data(data_count, size);
		char * read_data = malloc(size);
		if (read_data == NULL) {
			fslog("Error allocating read data");
			exit(-1);
		}

		if (!verify) {
			fwrite(c, size, 1, fs);
			if (verifyfile) fwrite(c, size, 1, fs_b);
		} else {
			int fail_index;
			fread(read_data, size, 1, fs);
			if ((fail_index = byte_compare(c, read_data, size)) >= 0) {
				fslog("file verification failed at %i hours, %s, index in file = %i",
						*minutes/60, filename, *file_size + fail_index);
				ret = -1;
			}
		}

		free(c);
		free(read_data);

		*minutes = *minutes + 1;

		*file_size += size;

		if (*file_size >= maxfilesize * 1024 * 1024) {
			fclose(fs);
			if (!verify && verifyfile) fclose(fs_b);
			fslog("%i minutes simulated, %f hours", *minutes, *minutes/60.0);
			break;
		}
	}

	if (delay > 0) {
		usleep(delay*1000);
	}

	return ret;
}

static int write_verify_test_files(int verify, int * total_sim_minutes)
{
	int total_write_size = 0;
	int file_count = 0;
	unsigned int data_count = 0;
	int minutes = *total_sim_minutes;

	int ret = 0;

	/* create files */
	for (;;) {
		/* LOOP 2 */
		int file_bytes_written;

		if (write_verify_test_file(file_count, &data_count, verify, &minutes, &file_bytes_written))
			ret = -1;

		file_count++;
		total_write_size += file_bytes_written;

		if (total_write_size >= totaltestsize * 1024 * 1024) {
			fslog("Maximum SD file usage reached");
			goto loop2_done;
		}

		if (minutes/(60.0) > maxsimhours) {
			fslog("Maximum hours reached, stop writing files");
			goto loop2_done;
		}

	}
loop2_done:
	if (verify || skipverify) *total_sim_minutes = minutes;
	return ret;
}


static int write_test_files(int * total_sim_minutes)
{
	return write_verify_test_files(0, total_sim_minutes);
}

static int verify_test_files(int * total_sim_minutes)
{
	return write_verify_test_files(1, total_sim_minutes);
}


int main(int argc, char **argv)
{
	process_options(argc, argv);

	if ((size % 4) != 0) {
		fslog("Error, size must be mutiple of 4");
		exit(-1);
	}

	fslog("------------------------------------------");
	fslog("Starting File System Stress Test");
	fslog("Options:");
	fslog("  - delay = %i ms", delay);
	fslog("  - size = %i bytes", size);
	fslog("  - maxfilesize = %i MBytes", maxfilesize);
	fslog("  - totaltestsize = %i MBytes", totaltestsize);
	fslog("  - maxsimhours = %i hours", maxsimhours);
	fslog("\n");

	if (!noremount) {
		fslog("remounting SD card in sync mode");
		if (is_mmc_card_mounted()) system("umount /media/mmcblk0p1");

		if (system("mount -o sync /dev/mmcblk0p1 /media/mmcblk0p1") != 0) {
			fslog("Error mounting SD card in sync mode");
			exit(-1);
		}
	}

	int failed = 0;

	int total_sim_minutes = 0;

	for (;;) {

		if (total_sim_minutes/(60.0) > maxsimhours) {
			fslog("Maximum hours reached, test is finished!");
			break;
		}

		fslog("Starting write data loop");

		write_test_files(&total_sim_minutes);

		fslog("Starting verify loop");

		if (!skipverify) {
			if (verify_test_files(&total_sim_minutes)) {
				failed = -1;
				fslog("ERROR: Data verification failed");
				if (stoponfail) break;
			}
		}
	}

	fslog("Simulated %f hours", total_sim_minutes/(60.0));

	if (failed) {
		fslog("ERROR: There were verification failures");
	}
}


