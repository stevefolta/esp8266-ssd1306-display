#include "WebServer.h"
#include "WebRequest.h"
#include "HTMLFiles.h"
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
	WebRequest request(data, length);
	if (request.type == WebRequest::BAD) {
		static const char bad_request[] = "400 Bad Request\r\n";
		espconn_send(connection, (uint8*) bad_request, strlen(bad_request));
		return;
		}

	const char* url = request.url;
	if (strcmp(url, "/") == 0)
		url = "index.html";
	while (url[0] == '/')
		url += 1;

	if (strncmp(url, "api/", 4) == 0) {
		//***
		}

	else if (request.type == WebRequest::GET) {
		cur_file.load(url);
		if (cur_file.is_valid()) {
			log("Sending %s...\n", url);
			//*** TODO: Need the response code & the headers!
			espconn_send(
				connection,
				(uint8*) cur_file.contents, cur_file.size);
			}
		}
}


void ICACHE_FLASH_ATTR WebServer::receive_fn(void* arg, char* data, unsigned short length)
{
	the_server->receive((struct espconn*) arg, data, length);
}



