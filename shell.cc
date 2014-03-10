#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>

class String {
	private:
		char *string;

	public:
		String() { this->string = NULL; }

		String(char *string) { 
			this->string = NULL;
			this->setString(string);
		}

		String(const char *string) { 
			this->string = NULL;
			this->setString(string);
		}

		String(String *string) { 
			this->string = NULL;
			this->setString(string);
		}

		String(const char *string, int start, int end) { 
			this->string = NULL;
			this->string = new char[end-start+1];
			strncpy(this->string, string + start, end - start);
			this->string[end - start] = '\0';
		}

		~String() {
			if (this->string != NULL)
				free(this->string);
		}

		void setString(char *string) {
			if (this->string != NULL) free(this->string);
			this->string = (char *) malloc(sizeof(char) *
					(strlen(string)+1));
			strcpy(this->string, string);
		}

		void setString(const char *string) {
			if (this->string != NULL) free(this->string);
			this->string = (char *) malloc(sizeof(char) *
					(strlen(string)+1));
			strcpy(this->string, string);
		}

		void setString(String *string) {
			if (this->string != NULL) free(this->string);
			this->string = (char *) malloc(sizeof(char) *
					(string->length()+1));
			strcpy(this->string, string->toCharArray());
		}

		char *toCharArray() {
			return this->string;
		}

		int length() {
			if (this->string != NULL)
				return strlen(this->string);
			return 0;
		}

		String *clone() {
			return new String(this);
		}
};

template <class T>
class Vector {
	private:
		T *vector;
		int counter;
	public:
		Vector() {
			vector = NULL;
			counter = 0;
		}

		~Vector() {
			if (vector != NULL) free(vector);
		}

		// T = Book
		void add(T elem) {
			vector = (T *) realloc(vector, sizeof(T) *
					(counter + 1));
			vector[counter++] = elem;
			//memcpy(&vector[counter++], &elem, sizeof(T));
		}

		void remove(int pos) {
			if (pos >= 0 && pos < counter) {
				for (int i = pos; i < counter-1; i++) {
					vector[i] = vector[i+1];
				}	
				vector = (T *) realloc(vector, sizeof(T) * --counter);
			}
		}

		T get(int pos) {
			if (pos >= 0 && pos < counter)
				return vector[pos];
			return NULL;
		}

		int size() {
			return counter;
		}
};

class Shell {

	private:
		int BUFFER_SIZE;
		char *buffer;
		int counter;

		Vector<String *> *match(const char *command, const char *pattern, 
				bool *matchResult) {

			pcre *reCompiled = NULL;
			pcre_extra *pcreExtra = NULL;
			int pcreExecRet;
			const char *pcreErrorStr;
			int pcreErrorOffset;
			const char *psubStrMatchStr;
			int j, len = strlen(command)+1;
			int *subStrVec = new int[len];

			Vector<String *> *myVector = new Vector<String *>();

			// no match for now...
			*matchResult = false;

			//printf("Pattern is: %s\n", pattern);

			// Compiling the pattern to be applied later...
			reCompiled = pcre_compile(pattern, 0, &pcreErrorStr, &pcreErrorOffset, NULL);

			// pcre_compile returns NULL on error, and sets pcreErrorOffset & pcreErrorStr
			if(reCompiled == NULL) {
			  printf("ERROR: Could not compile '%s': %s\n", pattern, pcreErrorStr);
			  return myVector;
			}

			// Optimize the regex
			pcreExtra = pcre_study(reCompiled, 0, &pcreErrorStr);

			/* pcre_study() returns NULL for both errors and when it can not optimize
			   the regex.  The last argument is how one checks for errors (it is NULL
			   if everything works, and points to an error string otherwise. */
			if (pcreErrorStr != NULL) {
			  printf("ERROR: Could not study '%s': %s\n", pattern, pcreErrorStr);
			  return myVector;
			}

			/* Try to find the regex in command, and report results. */
			pcreExecRet = pcre_exec(reCompiled,
			      		    pcreExtra,
			      		    command, 
			      		    strlen(command),  // length of string
			      		    0,                // Start looking at this point
			      		    0,                // OPTIONS
			      		    subStrVec,
			      		    len);              // Length of subStrVec
			// Something bad happened..
			if (pcreExecRet < 0) { 
			    /*
			    switch(pcreExecRet) {
				    case PCRE_ERROR_NOMATCH: 
					    printf("String did not match the pattern\n");
				    	    break;
				    case PCRE_ERROR_NULL: 
					    printf("Something was null\n");
		      			    break;
				    case PCRE_ERROR_BADOPTION: 
					    printf("A bad option was passed\n");
			   		    break;
				    case PCRE_ERROR_BADMAGIC: 
					    printf("Magic number bad (compiled re corrupt?)\n");
					    break;
				    case PCRE_ERROR_UNKNOWN_NODE: 
					    printf("Something kooky in the compiled re\n");      
					    break;
				    case PCRE_ERROR_NOMEMORY: 
					    printf("Ran out of memory\n");                       
					    break;
				    default: 
					    printf("Unknown error\n");                           
					    break;
			    }*/
			} else {
			    *matchResult = true;
			    printf("Result: We have a match!\n");
			      
			    // At this point, rc contains the number of substring matches found...
			    if(pcreExecRet == 0) {
			      printf("But too many substrings were found to fit in subStrVec!\n");
			      // Set rc to the max number of substring matches possible.
			      pcreExecRet = 30 / 3;
			    }

			    // PCRE contains a handy function to do the above for you:
			    for(j=0; j<pcreExecRet; j++) {
			      pcre_get_substring(command, subStrVec, pcreExecRet, j, 
					      &(psubStrMatchStr));
			      printf("Match(%2d/%2d): (%2d,%2d): '%s'\n", j, 
					      pcreExecRet-1, subStrVec[j*2], 
					      subStrVec[j*2+1], psubStrMatchStr);
			      myVector->add(new String(command, subStrVec[j*2], subStrVec[j*2+1]));
			    }

			    // Free up the substring
			    pcre_free_substring(psubStrMatchStr);
			} 

			//printf("\n");

			// Free up the regular expression.
			pcre_free(reCompiled);
			    
			// Free up the EXTRA PCRE value (may be NULL at this point)
			if(pcreExtra != NULL)
			  pcre_free(pcreExtra);

			return myVector;
		}

		char* nextCommand(FILE *stream) {
			char *buffer = NULL;
			int counter = 1;
			char c;
			bool matchResult;

			while (true) {
				c = fgetc(stream);

				buffer = (char *) realloc(buffer, sizeof(char) * (counter+1));
				buffer[counter-1] = c;
				buffer[counter] = '\0';
				counter++;

				printf("%c (%d)\n", c, c);

				// Veja mais sobre esta expressão regular em: 
				// 	http://regex101.com/r/vG3mX9
				Vector <String *> *result = 
					this->match(buffer, "\\n*\\s*(.*[^\\\\];)\\s*\\n*", &matchResult);

				if (matchResult) {
					char *aux = new char[counter];
					strcpy(aux, result->get(1)->toCharArray());
					if (buffer != NULL) free(buffer);
					if (result != NULL) delete result;

					return aux;
				}

				if (result != NULL) delete result;
			}
		}

		bool isCreateTable(const char *command) {
			bool matchResult;

			printf("DEBUG: Running isCreateTable\n");

			// Ver mais sobre esta expressão regular em:
			// 	http://regex101.com/r/nR4vR1
			Vector<String *> *result = 
				this->match(command, "^(?i)create\\s+table\\s+([a-zA-Z][a-zA-Z0-9_-]*)\\s+\\(\\s*([^\\)]+)\\s*\\)\\s*;\\s*$", &matchResult);


			if (matchResult) {
				printf("your code goes here...\n");
				String *tableName = result->get(1);
				String *fields = result->get(2);
				printf("Tablename = %s (length: %d)\n", tableName->toCharArray(), tableName->length());
				printf("All Fields = %s (length: %d)\n", fields->toCharArray(), fields->length());

				char *buffer = new char[BUFFER_SIZE];
				bool matchField;
				int counter = 1;
				do {
					matchField = false;
					sprintf(buffer, "(?:([^,]+)(,|$)){%d}", counter++);
					printf("Using buffer as: %s\n", buffer);
					Vector<String *> *field = 
						this->match(fields->toCharArray(), buffer, &matchField);

					if (matchField) {
						String *fieldInfo = field->get(1);
						printf("\t\t--> Field: %s\n", fieldInfo->toCharArray());
					}

					delete field;
				} while (matchField);
				delete[] buffer;

			} else {
				printf("no match...\n");
			}
		}

		bool isCreateDatabase(const char *command) {
			return true;
		}

		bool isInsert(const char *command) {
			return true;
		}

	public:
		Shell() {
			this->buffer = NULL;
			this->counter = 0;
			this->BUFFER_SIZE = 1024;
		}

		void commandLine() {
			bool matchResult;

			while (true) {
				printf("> ");
				char *command = nextCommand(stdin);
				printf("DEBUG: %s\n", command);
				
				//isCreateDatabase(command);
				isCreateTable(command);
				//isInsert(command);
			}
		}
};


int main(int argc, char *argv[]){
	
	Shell *s = new Shell();
	s->commandLine();
	delete s;
	
	
	return 0;
}
