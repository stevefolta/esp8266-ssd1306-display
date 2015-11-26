#ifndef WebRequest_h
#define WebRequest_h

class WebRequest {
	public:
		enum {
			GET, HEAD, POST, PUT, DELETE,
			BAD
			};

		WebRequest(char* data, unsigned short length);

		int	type;
		const char*	url;
		const char*	body;
		int	body_length;

	protected:
	};


#endif	// !WebRequest_h

