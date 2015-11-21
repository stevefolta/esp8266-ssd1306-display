#ifndef WordParser_h
#define WordParser_h

class WordParser {
	public:
		WordParser(const char* text_in, int text_length = -1);

		void	get_next_word(char* word_out, int size);
		bool	at_sentence_end() { return is_at_sentence_end; }
		bool	at_paragraph_end() { return is_at_paragraph_end; }
		bool	at_text_end();

	protected:
		const char* text;
		const char*	end;
		bool	is_at_sentence_end, is_at_paragraph_end;
	};


#endif	// !WordParser_h

