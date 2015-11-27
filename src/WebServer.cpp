#include "WebServer.h"
#include "WebRequest.h"
#include "HTMLFiles.h"
#include "user_main.h"
#include "log.h"
extern "C" {
#include "ets_sys.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "string.h"
#include "osapi.h"
#include "sdk_missing.h"
}

WebServer* WebServer::the_server = 0;


ICACHE_FLASH_ATTR WebServer::WebServer()
{
	the_server = this;

	accept_connection = (struct espconn*) os_zalloc(sizeof(struct espconn));
	accept_connection->type = ESPCONN_TCP;
	accept_connection->state = ESPCONN_NONE;
	accept_connection->proto.tcp = (esp_tcp*) os_zalloc(sizeof(esp_tcp));
	accept_connection->proto.tcp->local_port = 80;
	espconn_regist_connectcb(accept_connection, listen);

	espconn_accept(accept_connection);
}


ICACHE_FLASH_ATTR WebServer::~WebServer()
{
	os_free(accept_connection->proto.tcp);
	os_free(accept_connection);
}


void ICACHE_FLASH_ATTR WebServer::listen(void* arg)
{
	struct espconn* connection = (struct espconn*) arg;
	espconn_regist_recvcb(connection, receive_fn);
}


void ICACHE_FLASH_ATTR WebServer::receive(struct espconn* connection, char* data, unsigned short length)
{
	WebRequest request;
	request.received_data(data, length);
	if (request.type == WebRequest::BAD) {
		static const char bad_request[] = "400 Bad Request\r\n\r\n";
		espconn_send(connection, (uint8*) bad_request, strlen(bad_request));
		return;
		}

	const char* url = request.url;
	if (strcmp(url, "/") == 0)
		url = "index.html";
	while (url[0] == '/')
		url += 1;

	static const char not_found[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";

	if (request.type == WebRequest::GET) {
		cur_file.load(url);
		if (cur_file.is_valid()) {
			log("Sending %s...\n", url);
			const char* content_type = "text/plain";
			const char* dot = strrchr(url, '.');
			if (dot) {
				const char* suffix = dot + 1;
				if (strcmp(suffix, "html") == 0)
					content_type = "text/html";
				else if (strcmp(suffix, "css") == 0)
					content_type = "text/css";
				else if (strcmp(suffix, "js") == 0)
					content_type = "application/javascript";
				}
			char headers[256];
			static const char* headers_fmt =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: %s\r\n"
				"Content-Length: %d\r\n"
				"\r\n";
			os_sprintf(headers, headers_fmt, content_type, cur_file.size);
			int headers_length = strlen(headers);
			int message_length = headers_length + cur_file.size;
			char* message = (char*) os_zalloc(message_length);
			strcpy(message, headers);
			os_memcpy(message + headers_length, cur_file.contents, cur_file.size);
			espconn_send(connection, (uint8*) message, message_length);
			os_free(message);
			}
		else
			espconn_send(connection, (uint8*) not_found, strlen(not_found));
		}

	else if (request.type == WebRequest::POST) {
		if (strcmp(url, "message") == 0) {
			display_message(request.body, request.body_length);
			static const char* generic_ok =
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: 0\r\n"
				"\r\n";
			espconn_send(connection, (uint8*) generic_ok, strlen(generic_ok));
			}
		else
			espconn_send(connection, (uint8*) not_found, strlen(not_found));
		}

	else {
		static const char method_not_allowed[] = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
		espconn_send(
			connection, (uint8*) method_not_allowed, strlen(method_not_allowed));
		}
}


void ICACHE_FLASH_ATTR WebServer::receive_fn(void* arg, char* data, unsigned short length)
{
	the_server->receive((struct espconn*) arg, data, length);
}



