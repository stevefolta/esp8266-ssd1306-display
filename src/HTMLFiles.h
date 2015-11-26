#ifndef HTMLFiles_h
#define HTMLFiles_h

struct HTMLFile {
	HTMLFile();
	~HTMLFile();

	void	load(const char* path);
	void	free();
	bool is_valid() { return contents != 0; }

	char*	contents;
	int	size;
	};



#endif	// !HTMLFiles_h

