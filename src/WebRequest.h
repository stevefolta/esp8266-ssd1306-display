#ifndef WebRequest_h
#define WebRequest_h

class WebRequest {
	public:
		enum {
			GET, HEAD, POST, PUT, DELETE,
			BAD
			};

		WebRequest();
		~WebRequest();

		void	received_data(char* data, unsigned short length);
		bool	is_complete() { return state == Complete; }

		int	type;
		char*	url;
		char*	body;
		int	body_length;

	protected:
		enum {
			Starting,
			ReadingHeaders,
			ReadingBody,
			Complete,
			};
		int state;
		char* body_out;

		void	read_first_line(char* data, char* end);
		void	read_headers(char* data, char* end);
		void	read_body(char* data, char* end);
	};


#endif	// !WebRequest_h

