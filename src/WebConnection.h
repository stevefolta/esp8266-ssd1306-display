#ifndef WebConnection_h
#define WebConnection_h

struct espconn;
class WebRequest;

class WebConnection {
	public:
		WebConnection(struct espconn* connection_in)
			: connection(connection_in), request(0) {}
		~WebConnection();

		void	received_data(char* data, unsigned short length);

		struct espconn*	connection;

	protected:
		WebRequest*	request;

		void	reset();
	};



#endif	// !WebConnection_h

