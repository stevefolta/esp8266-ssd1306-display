#ifndef WebServer_h
#define WebServer_h

#include "HTMLFiles.h"

struct espconn;
class WebConnection;

class WebServer {
	public:
		WebServer();
		~WebServer();

	protected:
		enum {
			max_connections = 8,
			};

		struct espconn*	accept_connection;
		WebConnection**	connections;
		HTMLFile	cur_file;
		static WebServer*	the_server;

		void	new_connection(struct espconn* connection);
		static void	new_connection_fn(void* arg);
		void	receive(struct espconn* connection, char* data, unsigned short length);
		static void	receive_fn(void* arg, char* data, unsigned short length);
		void	disconnected(struct espconn* connection);
		static void	disconnected_fn(void* arg);
	};


#endif	// !WebServer_h

