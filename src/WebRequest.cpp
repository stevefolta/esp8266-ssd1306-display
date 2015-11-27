#include "WebRequest.h"
#include "log.h"
extern "C" {
#include "string.h"
#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "mem.h"
#include "sdk_missing.h"
}


ICACHE_FLASH_ATTR WebRequest::WebRequest()
{
	type = BAD;
	url = body = NULL;
	body_length = 0;
	state = Starting;
}


ICACHE_FLASH_ATTR WebRequest::~WebRequest()
{
	if (url)
		os_free(url);
	if (body)
		os_free(body);
}


ICACHE_FLASH_ATTR void WebRequest::received_data(
	char* data, unsigned short length)
{
	char* end = data + length;
	switch (state) {
		case Starting:
			read_first_line(data, end);
			break;
		case ReadingHeaders:
			read_headers(data, end);
			break;
		case ReadingBody:
			read_body(data, end);
			break;
		default:
			// ERROR!  We shouldn't get any more data at this point.
			break;
		}
}


ICACHE_FLASH_ATTR void WebRequest::read_first_line(char* data, char* end)
{
	// For now, we'll assume the first line won't arrive split among separate
	// transmissions.

	// Type.
	char* line_end = strstr(data, "\r\n");
	if (line_end == NULL)
		return;
	char* space = strchr(data, ' ');
	if (space == NULL || space > line_end)
		return;
	*space = 0;
	char* type_str = data;
	data = space + 1;
	if (strcmp(type_str, "GET") == 0)
		type = GET;
	else if (strcmp(type_str, "PUT") == 0)
		type = PUT;
	else if (strcmp(type_str, "POST") == 0)
		type = POST;
	else if (strcmp(type_str, "HEAD") == 0)
		type = HEAD;
	else if (strcmp(type_str, "DELETE") == 0)
		type = DELETE;
	else {
		type = BAD;
		return;
		}
	// Skip extra whitespace.
	char c;
	for (; data < line_end; ++data) {
		c = *data;
		if (c != ' ' && c != '\t')
			break;
		}
	// URL.
	space = strchr(data, ' ');
	if (space == NULL || space > line_end) {
		type = BAD;
		state = Complete;
		return;
		}
	const char* read_url = data;
	int url_length = space - data;
	// Skip the rest.
	data = line_end + 2;

	// Copy the URL.
	if (url)
		os_free(url);
	url = (char*) os_zalloc(url_length + 1);
	memcpy(url, read_url, url_length);
	url[url_length] = 0;

	state = ReadingHeaders;
	read_headers(data, end);
}


ICACHE_FLASH_ATTR void WebRequest::read_headers(char* data, char* end)
{
	char c;

	// We're going to assume that none of the header lines will be split between
	// two transmissions.

	body_length = 0;
	while (data < end) {
		// Get the line.
		char* line_end = strstr(data, "\r\n");
		if (line_end == NULL) {
			// Malformed request.
			type = BAD;
			state = Complete;
			return;
			}
		*line_end = 0;

		// Are we at the end of the headers?
		if (data[0] == 0) {
			data = line_end + 2;
			state = ReadingBody;
			break;
			}

		// Get the header name.
		char* header_name = data;
		char* colon = strchr(data, ':');
		if (colon == NULL) {
			// It's a header continuation line, which we don't handle, so just
			// skip it.
			}
		else {
			*colon = 0;
			data = colon + 1;
			// Skip whitespace.
			for (; ; ++data) {
				c = *data;
				if (c != ' ' && c != '\t')
					break;
				}
			char* value = data;
			if (strcmp(header_name, "Content-Length") == 0)
				body_length = atoi(value);
			else {
				// Other headers are ignored.
				}
			}

		// Next line.
		data = line_end + 2;
		}

	if (state == ReadingBody) {
		if (body)
			os_free(body);
		body = (char*) os_zalloc(body_length);
		body_out = body;
		read_body(data, end);
		}
}


ICACHE_FLASH_ATTR void WebRequest::read_body(char* data, char* end)
{
	int data_length = end - data;
	int body_left_to_read = body_length - (body_out - body);
	bool is_last_chunk = false;
	if (data_length > body_left_to_read) {
		log("Body overflow!\n");
		end = data + body_left_to_read;
		is_last_chunk = true;
		}
	else if (data_length == body_left_to_read)
		is_last_chunk = true;

	memcpy(body_out, data, data_length);
	body_out += data_length;
	if (is_last_chunk)
		state = Complete;
}



