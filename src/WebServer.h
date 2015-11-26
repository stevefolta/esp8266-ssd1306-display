#ifndef WebServer_h
#define WebServer_h

#include "HTMLFiles.h"

struct espconn;

class WebServer {
	public:
		WebServer();
		~WebServer();

	protected:
		struct espconn*	accept_connection;
		HTMLFile	cur_file;
		static WebServer*	the_server;

		static void	listen(void* arg);
		void	receive(struct espconn* connection, char* data, unsigned short length);
		static void	receive_fn(void* arg, char* data, unsigned short length);
	};


#endif	// !WebServer_h

