#include "WordParser.h"
#include <string.h>
extern "C" {
#include "ets_sys.h"
}


ICACHE_FLASH_ATTR WordParser::WordParser(const char* text_in, int text_length)
	: text(text_in), is_at_sentence_end(false), is_at_paragraph_end(false)
{
	if (text_length < 0)
		end = text + strlen(text);
	else
		end = text + text_length;
}


ICACHE_FLASH_ATTR void WordParser::get_next_word(char* word_out, int size)
{
	char c;
	char* out_end = word_out + size - 1;

	// Skip whitespace.
	while (true) {
		c = *text;
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			break;
		}

	// Get the word.
	is_at_sentence_end = is_at_paragraph_end = false;
	bool done = false;
	while (!done && text < end) {
		c = *text++;
		switch (c) {
			case ' ':
			case '\t':
				done = true;
				break;

			case '\n':
			case '\r':
				is_at_paragraph_end = true;
				done = true;
				break;

			case '.':
			case '!':
			case '?':
				is_at_sentence_end = true;
				if (word_out < out_end)
					*word_out++ = c;
				break;

			default:
				is_at_sentence_end = false;
				if (word_out < out_end)
					*word_out++ = c;
				break;
			}
		}

	// Skip trailing whitespace.
	done = false;
	while (!done && text < end) {
		c = *text;
		if (c == '\n' || c == '\r') {
			is_at_paragraph_end = true;
			text++;
			}
		else if (c == ' ' || c == '\t')
			text++;
		else
			done = true;
		}

	// Finish the output.
	*word_out = 0;
}


ICACHE_FLASH_ATTR bool WordParser::at_text_end()
{
	return text >= end;
}



