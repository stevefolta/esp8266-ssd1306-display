#include "WebRequest.h"
extern "C" {
#include "string.h"
#include "stdlib.h"
}


WebRequest::WebRequest(char* data, unsigned short length)
{
	type = BAD;
	url = body = NULL;
	body_length = 0;

	const char* end = data + length;

	// Parse the first line.
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
		return;
		}
	*space = 0;
	url = data;
	// Skip the rest.
	data = line_end + 2;

	// Parse the headers.
	body_length = 0;
	while (data < end) {
		// Get the line.
		line_end = strstr(data, "\r\n");
		if (line_end == NULL) {
			// Malformed request.
			type = BAD;
			url = NULL;
			return;
			}
		*line_end = 0;

		// Are we at the end of the headers?
		if (data[0] == 0) {
			data = line_end + 2;
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

	// Body.
	body = data;
	if (end - body < body_length) {
		// We didn't get the whole body.  But what do we do about it?
		body = NULL;
		body_length = 0;
		}
}



