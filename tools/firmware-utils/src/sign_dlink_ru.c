/*
 * This program is designed to sign firmware images so they are accepted
 * by D-Link DIR-882 R1 WebUIs.
 *
 * Copyright (C) 2020 Andrew Pikler
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "md5.h"

#define BUF_SIZE 4096
#define MD5_HASH_LEN 16


typedef struct _md5_digest_t {
	uint8_t digest[MD5_HASH_LEN];
} md5_digest_t;

typedef struct _salt_t {
	char* salt_ascii;
	uint8_t* salt_bin;
	size_t salt_bin_len;
} salt_t;

void read_file_bytes(FILE* f, MD5_CTX* md5_ctx) {
	uint8_t buf[BUF_SIZE];
	size_t bytes_read;
	rewind(f);
	
	while (0 != (bytes_read = fread(buf, sizeof(uint8_t), BUF_SIZE, f))) {
		MD5_Update(md5_ctx, buf, bytes_read);
	}

	if (!feof(f)) {
		printf("Error: expected to be at EOF\n");
		exit(-1);
	}
}

void add_magic_bytes(FILE* f) {
	char magic_bytes[] = { 0x00, 0xc0, 0xff, 0xee };
	size_t magic_bytes_len = 4;
	fwrite(magic_bytes, magic_bytes_len, 1, f);
}

/**
 * Add the signature produced by this salt to the file
 * The signature consists by creating an MD5 digest wht the salt bytes plus
 * all of the bytes in the firmware file, then adding the magic bytes to the
 * file
 */
void add_signature(FILE* f, salt_t* salt) {
	md5_digest_t digest;
	MD5_CTX md5_context;

	MD5_Init(&md5_context);
	MD5_Update(&md5_context, salt->salt_bin, salt->salt_bin_len);
	read_file_bytes(f, &md5_context);
	MD5_Final(digest.digest, &md5_context);

	fwrite(&digest.digest, sizeof(uint8_t), MD5_HASH_LEN, f);
	add_magic_bytes(f);
}

void add_version_suffix(FILE* f) {
	char* version_suffix = "c0ffeef0rge";
	fseek(f, 0, SEEK_END);
	fwrite(version_suffix, sizeof(char), strlen(version_suffix), f);
}

int asciihex_to_int(char c) {
	if(c >= '0' && c <= 'F')
		return c - '0';

	if(c >= 'a' && c <= 'f')
		return 10 + c - 'a';
	return -1;
}

/**
 * Verify this is a valid hex string to convert
 */
void verify_valid_hex_str(char* s) {
	int i;
	int s_len = strlen(s);
	if (s_len == 0) {
		printf("invalid empty salt: %s\n", s);
		exit(-1);
	}

	if (s_len % 2 != 0) {
		printf("invalid odd len salt: %s\n", s);
		exit(-1);
	}

	for (i = 0; i < s_len; ++i) {
		if (asciihex_to_int(s[i]) < 0) {
			printf("invalid salt (invalid hex char): %s\n", s);
			exit(-1);
		}
	}
}

/**
 * Convert a hex ascii string to an allocated binary array. This array must be free'd
 */
uint8_t* convert_hex_to_bin(char * s) {
	int i;
	int s_len = strlen(s);

	uint8_t* ret = malloc(s_len / 2);
	for (i = 0; i < s_len; i += 2) {
		ret[i / 2] = (asciihex_to_int(s[i]) << 4) | asciihex_to_int(s[i + 1]);
	}

	return ret;
}

void init_salt(salt_t* salt, char * salt_ascii) {
	salt->salt_ascii = salt_ascii;
	salt->salt_bin = convert_hex_to_bin(salt_ascii);
	salt->salt_bin_len = strlen(salt_ascii) / 2;
}

void free_salt(salt_t* salt) {
	free(salt->salt_bin);
}

/**
 * Verify that the arguments are valid, or exit with failure
 */
void verify_args(int argc, char** argv) {
	int i;

	if (argc < 3) {
		printf("Usage: %s <firmware file> <signing hash1> <signing hash2> ... <signing hash n>\n", argv[0]);
		exit(1);
	}

	for (i = 2; i < argc; i++) {
		verify_valid_hex_str(argv[i]);
	}
}

FILE* make_out_file(char* filename) {
	uint8_t buf[BUF_SIZE];
	int bytes_read;
	char* suffix = ".new";
	int new_filename_len = strlen(filename) + strlen(suffix) + 1;
	char* new_filename = malloc(new_filename_len);
	strcpy(new_filename, filename);
	strcat(new_filename, suffix);

	FILE* f = fopen(filename, "r+");
	if (!f) {
		printf("cannot open file %s\n", filename);
		exit(2);
	}

	FILE* out = fopen(new_filename, "w+");
	free(new_filename);
	if (!out) {
		printf("cannot open file %s\n", filename);
		exit(2);
	}

	while (0 != (bytes_read = fread(buf, sizeof(uint8_t), BUF_SIZE, f))) {
		fwrite(buf, sizeof(uint8_t), bytes_read, out);
	}
	fclose(f);
	return out;
}

/**
 * Sign the firmware file after all of our checks have completed
 */
void sign_firmware(char* filename, char** salts, int num_salts) {
	int i;
	salt_t salt;
	FILE* f = make_out_file(filename);

	// add a version suffix string - dlink versions do something similar before the first signature
	add_version_suffix(f);

	//for each of the salts we are supplied with
	for (i = 0; i < num_salts; i++) {
		char* salt_str = salts[i];
		// convert this str to binary
		init_salt(&salt, salt_str);

		// add the signature to the firmware file produced from this salt
		add_signature(f, &salt);
		free_salt(&salt);
		printf("Signed with salt: %s\n", salt_str);
	}

	fclose(f);
}


int main(int argc, char ** argv) {
	verify_args(argc, argv);
	sign_firmware(argv[1], argv+2, argc-2);
	return 0;
}
