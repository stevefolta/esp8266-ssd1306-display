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

struct WebConnection {
	struct espconn	connection;
	WebServer*	server;
	};

WebServer* WebServer::the_server = 0;


ICACHE_FLASH_ATTR WebServer::WebServer()
{
	the_server = this;

	connection = (WebConnection*) os_zalloc(sizeof(WebConnection));
	connection->connection.type = ESPCONN_TCP;
	connection->connection.state = ESPCONN_NONE;
	connection->connection.proto.tcp = (esp_tcp*) os_zalloc(sizeof(esp_tcp));
	connection->connection.proto.tcp->local_port = 80;
	espconn_regist_connectcb(&connection->connection, listen);
	connection->server = this;

	espconn_accept(&connection->connection);
}


ICACHE_FLASH_ATTR WebServer::~WebServer()
{
	os_free(connection->connection.proto.tcp);
	os_free(connection);
}


void ICACHE_FLASH_ATTR WebServer::listen(void* arg)
{
	struct espconn* connection = (struct espconn*) arg;
	espconn_regist_recvcb(connection, receive_fn);
}


void ICACHE_FLASH_ATTR WebServer::receive(char* data, unsigned short length)
{
	WebRequest request(data, length);
	if (request.type == WebRequest::BAD) {
		static const char bad_request[] = "400 Bad Request\r\n";
		espconn_send(&connection->connection, (uint8*) bad_request, strlen(bad_request));
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
				&connection->connection,
				(uint8*) cur_file.contents, cur_file.size);
			}
		}
}


void ICACHE_FLASH_ATTR WebServer::receive_fn(void* arg, char* data, unsigned short length)
{
	the_server->receive(data, length);
}



