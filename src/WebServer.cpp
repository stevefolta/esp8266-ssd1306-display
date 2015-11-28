#include "WebServer.h"
#include "WebConnection.h"
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

	connections = (WebConnection**) os_zalloc(max_connections * sizeof(WebConnection*));

	accept_connection = (struct espconn*) os_zalloc(sizeof(struct espconn));
	accept_connection->type = ESPCONN_TCP;
	accept_connection->state = ESPCONN_NONE;
	accept_connection->proto.tcp = (esp_tcp*) os_zalloc(sizeof(esp_tcp));
	accept_connection->proto.tcp->local_port = 80;
	espconn_regist_connectcb(accept_connection, new_connection_fn);

	espconn_accept(accept_connection);
}


ICACHE_FLASH_ATTR WebServer::~WebServer()
{
	for (int i = 0; i < max_connections; ++i)
		delete connections[i];
	os_free(connections);
	os_free(accept_connection->proto.tcp);
	os_free(accept_connection);
}


void ICACHE_FLASH_ATTR WebServer::new_connection(struct espconn* connection)
{
	espconn_regist_recvcb(connection, receive_fn);
	espconn_regist_disconcb(connection, disconnected_fn);

	for (int i = 0; i < max_connections; ++i) {
		if (!connections[i]) {
			connections[i] = new WebConnection(connection);
			break;
			}
		}
}


void ICACHE_FLASH_ATTR WebServer::new_connection_fn(void* arg)
{
	the_server->new_connection((struct espconn*) arg);
}


void ICACHE_FLASH_ATTR WebServer::receive(struct espconn* connection, char* data, unsigned short length)
{
	for (int i = 0; i < max_connections; ++i) {
		if (connections[i] && connection == connections[i]->connection) {
			connections[i]->received_data(data, length);
			break;
			}
		}
}


void ICACHE_FLASH_ATTR WebServer::receive_fn(void* arg, char* data, unsigned short length)
{
	the_server->receive((struct espconn*) arg, data, length);
}


void ICACHE_FLASH_ATTR WebServer::disconnected(struct espconn* connection)
{
	for (int i = 0; i < max_connections; ++i) {
		if (connections[i] && connection == connections[i]->connection) {
			delete connections[i];
			connections[i] = 0;
			break;
			}
		}
}


void ICACHE_FLASH_ATTR WebServer::disconnected_fn(void* arg)
{
	the_server->disconnected((struct espconn*) arg);
}




