#ifndef WebServer_h
#define WebServer_h

#include "HTMLFiles.h"

struct WebConnection;

class WebServer {
	public:
		WebServer();
		~WebServer();

	protected:
		WebConnection*	connection;
		HTMLFile	cur_file;
		static WebServer*	the_server;

		static void	listen(void* arg);
		void	receive(char* data, unsigned short length);
		static void	receive_fn(void* arg, char* data, unsigned short length);
	};


#endif	// !WebServer_h

