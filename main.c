/*
 *  bin2array
 *  Copyright (C) 2020 Anthony Lee 
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. *  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA *  02110-1301, USA.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#define STYLE_C_UINT8 0
#define STYLE_C_UINT8_IDENT "--c-uint8"
#define STYLE_C_UINT16 1
#define STYLE_C_UINT16_IDENT "--c-uint16"
#define STYLE_C_UINT32 2 
#define STYLE_C_UINT32_IDENT "--c-uint32"
#define STYLE_C_UINT64 4
#define STYLE_C_UINT64_IDENT "--c-uint64"
#define STYLE_C_INT8 8
#define STYLE_C_INT8_IDENT "--c-int8"
#define STYLE_C_INT16 16
#define STYLE_C_INT16_IDENT "--c-int16"
#define STYLE_C_INT32 32
#define STYLE_C_INT32_IDENT "--c-int32"
#define STYLE_C_INT64 64
#define STYLE_C_INT64_IDENT "--c-int64"
#define STYLE_JAVA_BYTE 16384
#define STYLE_JAVA_BYTE_IDENT "--java-byte"
#define STYLE_JAVA_CHAR 32768
#define STYLE_JAVA_CHAR_IDENT "--java-char"

#define INT_TYPE_HEX 0
#define INT_TYPE_DEC 1

struct argvconf {
	int style;
	int canonical;
	int inttype;
	int length_of_line;
	int length_of_item;
	char* filepath;
};

void do_array_printing(size_t current_size, size_t thissize, struct argvconf* conf, uint8_t* buffer, int last_buffer);
void do_print_head(struct argvconf* conf);
void do_print_tail(struct argvconf* conf);


void* request_shared_buffer(size_t size) {
	static void* buffer = NULL;
	static int current_size = 0;
	if (! buffer) {
		buffer = malloc(size);
		current_size = size;
	} else if (size > current_size) {
		buffer = realloc(buffer, size);
		current_size = size;
	}

	return buffer;
}

void print_help(const char* argv0) {
	fprintf(stderr, "Usage: %s [-f file] [args]\n", argv0);
	fputs("Options:\n", stderr);
	fputs("  -f\t\tInput file (default: stdin)\n", stderr);
	fputs("  -h, --help\tShow this help\n", stderr);
	fputs("  --c-uint8\tUses C style with unsigned 8-bit integer format.\n", stderr);
	fputs("  --c-uint16\tUses C style with unsigned 16-bit integer format.\n", stderr);
	fputs("  --c-uint32\tUses C style with unsigned 32-bit integer format.\n", stderr);
	fputs("  --c-uint64\tUses C style with unsigned 64-bit integer format.\n", stderr);
	fputs("  --c-int8\tUses C style with signed 8-bit integer format.\n", stderr);
	fputs("  --c-int16\tUses C style with signed 16-bit integer format.\n", stderr);
	fputs("  --c-int32\tUses C style with signed 32-bit integer format.\n", stderr);
	fputs("  --c-int64\tUses C style with signed 64-bit integer format.\n", stderr);
	fputs("  --java-byte\tUses Java style with byte format.\n", stderr);
	fputs("  --java-char\tUses Java style with char format.\n", stderr);
}

void resolv_argv(int argc, char* argv[], struct argvconf* conf) {
	conf->length_of_line = 4;	// default as 4
	conf->length_of_item = 1;

	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			print_help(argv[0]);
			exit(0);
		} else if (!strcmp(argv[i], "--file") || !strcmp(argv[i], "-f")) {
			if (i + 1 < argc && argv[i + 1]) {
				conf->filepath = argv[i + 1];
			}
		} else if (!strcmp(argv[i], "--dec")) {
			conf->inttype = INT_TYPE_DEC;
		} else if (!strcmp(argv[i], "--len") || !strcmp(argv[i], "-l")) {
			if (i + 1 < argc && argv[i + 1]) {
				conf->length_of_line = atoi(argv[i + 1]);
			}
		} else if (!strcmp(argv[i], STYLE_C_UINT8_IDENT)) {
			conf->style = STYLE_C_UINT8;
		} else if (!strcmp(argv[i], STYLE_C_UINT16_IDENT)) {
			conf->style = STYLE_C_UINT16;
			conf->length_of_item = 2;
		} else if (!strcmp(argv[i], STYLE_C_UINT32_IDENT)) {
			conf->style = STYLE_C_UINT32;
			conf->length_of_item = 4;
		} else if (!strcmp(argv[i], STYLE_C_UINT64_IDENT)) {
			conf->style = STYLE_C_UINT64;
			conf->length_of_item = 8;
		} else if (!strcmp(argv[i], STYLE_C_INT8_IDENT)) {
			conf->style = STYLE_C_INT8;
		} else if (!strcmp(argv[i], STYLE_C_INT16_IDENT)) {
			conf->style = STYLE_C_INT16;
			conf->length_of_item = 2;
		} else if (!strcmp(argv[i], STYLE_C_INT32_IDENT)) {
			conf->style = STYLE_C_INT32;
			conf->length_of_item = 4;
		} else if (!strcmp(argv[i], STYLE_C_INT64_IDENT)) {
			conf->style = STYLE_C_INT64;
			conf->length_of_item = 8;
		} else if (!strcmp(argv[i], STYLE_JAVA_BYTE_IDENT)) {
			conf->style = STYLE_JAVA_BYTE;
		} else if (!strcmp(argv[i], STYLE_JAVA_CHAR_IDENT)) {
			conf->style = STYLE_JAVA_CHAR;
		}
	}
}

void do_work(struct argvconf* conf) {
	uint8_t* shared_buf = (uint8_t *) malloc(4096);
	FILE* input_handle;
	if (conf->filepath) {
		input_handle = fopen(conf->filepath, "rb");
		if (! input_handle) {
			fprintf(stderr, "Error while opening file %s: %s\n", conf->filepath, strerror(errno));
			exit(1);
		}
	} else {
		// if user didn't gives specific input file, uses stdin for reading
		input_handle = stdin;
	}
	size_t totalwrite_size = 0;
	do_print_head(conf);
	while (1) {
		size_t readsize = fread(shared_buf, 1, 4096, input_handle);
		int last_buffer = (readsize < 4096);
		do_array_printing(totalwrite_size, readsize, conf, shared_buf, last_buffer);
		totalwrite_size += readsize;
		if (last_buffer) {
			do_print_tail(conf);
			break;
		}
	}
}

void do_print_tail(struct argvconf* conf) {
	fputs("};\n", stdout);
}

void do_print_head(struct argvconf* conf) {
	char* print_head = NULL;
	switch (conf->style) {
		case STYLE_C_UINT8:
			print_head = "const uint8_t array[] = {\n";
			break;
		case STYLE_C_UINT16:
			print_head = "const uint16_t array[] = {\n";
			break;
		case STYLE_C_UINT32:
			print_head = "const uint32_t array[] = {\n";
			break;
		case STYLE_C_UINT64:
			print_head = "const uint64_t array[] = {\n";
			break;
		case STYLE_C_INT8:
			print_head = "const int8_t array[] = {\n";
			break;
		case STYLE_C_INT16:
			print_head = "const int16_t array[] = {\n";
			break;
		case STYLE_C_INT32:
			print_head = "const int32_t array[] = {\n";
			break;
		case STYLE_C_INT64:
			print_head = "const int64_t array[] = {\n";
			break;
		case STYLE_JAVA_BYTE:
			print_head = "final byte[] array = new byte[] {\n";
			break;
		case STYLE_JAVA_CHAR:
			print_head = "final char[] array = new char[] {\n";
			break;
	}
	fputs(print_head, stdout);
}

static inline void get_value_safety(void* out, void* in, size_t copy_size, size_t variable_size) {
	// Read `copy_size` from in, then write them to out
	// Then fill (variable_size - copy_size) count zero byte to least space of out
	uint8_t* inu8 = (uint8_t *) in;
	uint8_t* outu8 = (uint8_t *) out;
	for (size_t i = 0; i < variable_size; i++) {
		if (i < copy_size) {
			outu8[i] = inu8[i];
		} else {
			outu8[i] = 0;
		}
	}
}

/*static inline*/ size_t do_array_print_onechar(char* writebuf, struct argvconf* conf, uint8_t* bufferin, size_t get_value_size) {
	char wbuf[32];
	char* prefix = "";
	if (conf->style == STYLE_JAVA_BYTE) 
		prefix = "(byte) ";
	else if (conf->style == STYLE_JAVA_CHAR)
		prefix = "(char) ";

	int linelen = 0;

	switch (conf->length_of_item) {
		case 1:
			if (conf->canonical || conf->style == STYLE_JAVA_CHAR) {
				uint8_t char_at_pos = *bufferin;
				if (char_at_pos == '\\' || char_at_pos == '\'')
					linelen = sprintf(wbuf, "%s\'\\%c\'", prefix, char_at_pos);
				else if (char_at_pos > 31 && char_at_pos < 127)
					linelen = sprintf(wbuf, "%s\'%c\'", prefix, char_at_pos);
				else
					linelen = sprintf(wbuf, "%s0x%x", prefix, char_at_pos);
				break;
			}
		case 2:
		case 4: {
			long value_mask = (1L << (8 * conf->length_of_item)) - 1;
			uint32_t value;
			if (get_value_size) {
				get_value_safety(&value, bufferin, get_value_size, conf->length_of_item);
			} else {
				value = *((uint32_t *) bufferin);	// uses memcpy(3) later
			}
			if (conf->inttype == INT_TYPE_DEC) {
				if (conf->style > 4)
					linelen = sprintf(wbuf, "%s%d", prefix, value & value_mask);
				else
					linelen = sprintf(wbuf, "%s%u", prefix, value & value_mask);
			} else {
				linelen = sprintf(wbuf, "%s0x%x", prefix, value & value_mask);
			}
			break;
		}
		case 8: {
			long value_mask = 0xFFFFFFFFFFFFFFFFL;
			uint64_t value;
			if (get_value_size) {
				get_value_safety(&value, bufferin, get_value_size, 8);
			} else {
				value = *((uint64_t *) bufferin);
			}
			if (conf->inttype == INT_TYPE_DEC) {
				if (conf->style > 4)
					linelen = sprintf(wbuf, "%s%ld", prefix, value & value_mask);
				else
					linelen = sprintf(wbuf, "%s%lu", prefix, value & value_mask);
			} else {
				linelen = sprintf(wbuf, "%s0x%lx", prefix, value & value_mask);
			}
			break;
		}
	}
	if (linelen > 0) {
		memcpy(writebuf, wbuf, linelen);
	}

	return linelen;
}

static inline size_t write_middle_symbol(char* string_in, struct argvconf* conf) {
	const uint16_t comma_and_space = ',' + (' ' << 8);
	((uint16_t *) string_in)[0] = comma_and_space;
	return 2;
}

void do_array_printing(size_t current_size, size_t thissize, struct argvconf* conf, uint8_t* buffer, int last_buffer) {
	size_t string_buffer_size = conf->length_of_line * 25 + (conf->length_of_line - 1) * 2 + 1;
	size_t line_read_step_size = conf->length_of_line * conf->length_of_item;
	size_t last_size = thissize % line_read_step_size;
	int count_of_line = thissize / line_read_step_size;

	char* string_buffer = (char *) request_shared_buffer(string_buffer_size);
	size_t data_writeoff = 0;
	size_t string_writeoff_init = 0;

	int i;
	int j;

	// write four tabspace align before
	if (conf->style > 64) {
		((uint32_t *) string_buffer)[0] = 0x20202020;
		string_writeoff_init += sizeof(uint32_t);
	} else {
		string_buffer[0] = '\t';
		string_writeoff_init ++;
	}
	
	for (i = 0; i < count_of_line; i++) {
		size_t string_writeoff = string_writeoff_init;
		for (j = 0; j < conf->length_of_line; j++) {
			if (j > 0) {
				string_writeoff += write_middle_symbol(&string_buffer[string_writeoff], conf);
			}
			string_writeoff += do_array_print_onechar(&string_buffer[string_writeoff], conf, &buffer[data_writeoff], 0);
			data_writeoff += conf->length_of_item;
		}
		if (last_size > 0 || !last_buffer || count_of_line - i > 1) {
			string_writeoff += write_middle_symbol(&string_buffer[string_writeoff], conf);
		}
		string_buffer[string_writeoff++] = '\n';
		fwrite(string_buffer, string_writeoff, 1, stdout);
		string_writeoff = 0;
	}
	if (last_size > 0) {
		int last_count_of_item = last_size / conf->length_of_item;
		size_t least_item = last_size % conf->length_of_item;
		size_t string_writeoff = 0;
		for (i = 0; i < last_count_of_item; i++) {
			if (i > 0) {
				string_writeoff += write_middle_symbol(&string_buffer[string_writeoff], conf);
			}
			string_writeoff += do_array_print_onechar(&string_buffer[string_writeoff], conf, &buffer[data_writeoff], 0);
			data_writeoff += conf->length_of_item;
		}
		if (least_item) {
			// add one more middle char
			if (i > 0) {
				string_writeoff += write_middle_symbol(&string_buffer[string_writeoff], conf);
			}
			string_writeoff += do_array_print_onechar(&string_buffer[string_writeoff], conf, &buffer[data_writeoff], least_item);
		}
		fwrite(string_buffer, string_writeoff, 1, stdout);
	}
}

int main(int argc, char* argv[]) {
	struct argvconf conf;
	memset(&conf, 0x00, sizeof(struct argvconf));
	resolv_argv(argc, argv, &conf);
	do_work(&conf);
	return 0;
}
