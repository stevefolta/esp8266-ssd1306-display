#include "HTMLFiles.h"
#include "FlashData.h"
#include "log.h"
extern "C" {
#include "mem.h"
#include "string.h"
#include "osapi.h"
#include "sdk_missing.h"
}

struct HTMLFileContents {
	const char*	path;
	const char*	contents;
	uint32	size;
	};

#include "../objects/html.h"


HTMLFile::HTMLFile()
	: contents(0), size(0)
{
}


HTMLFile::~HTMLFile()
{
	free();
}


void HTMLFile::load(const char* path)
{
	uint32* in;
	uint32* in_end;
	uint32* out;
	int num_words;

	free();

	const HTMLFileContents* found_file = 0;
	num_words = (strlen(path) + 1 + 3) / 4;
	char file_path[num_words * 4];
	int num_files = sizeof(files) / sizeof(files[0]);
	for (int which_file = 0; which_file < num_files; ++which_file) {
		// Copy the file's path so we can strcmp it.
		in = (uint32*) files[which_file].path;
		in_end = in + num_words;
		out = (uint32*) file_path;
		while (in < in_end)
			*out++ = *in++;
		if (strcmp(path, file_path) == 0) {
			found_file = &files[which_file];
			break;
			}
		}
	if (found_file) {
		size = found_file->size;
		// Since we're copying from flash, we need to copy 4 bytes at a time.
		num_words = (found_file->size + 3) / 4;
		contents = (char*) os_zalloc(num_words * 4);
		in = (uint32*) found_file->contents;
		in_end = in + num_words;
		out = (uint32*) contents;
		while (in < in_end)
			*out++ = *in++;
		}
}


void HTMLFile::free()
{
	if (contents)
		os_free(contents);
	contents = 0;
}



