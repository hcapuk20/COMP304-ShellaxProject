#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {

	if (argc == 2){
		if(strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "--count") != 0){
			return 1;
		}
	}
	int i = 0;
	char input[5000];

	if (!isatty(fileno(stdin))) {
    		while(-1 != (input[i++] = getchar()));
		input[i-1] = '\0';
    		//printf("%s\n", input);
  	}

	if(strlen(input) == 0){ return 1; }

	char *words[1000];
	int wordCount[1000];
	int length = 1;

	char *tokenizer = " \n\t";
	char *word = strtok(input, tokenizer);

	i = 0;
	words[i] = word;
	while (word) {
		if(strcmp(word, words[i]) == 0){
			wordCount[i] += 1;
		} else {
			words[++i] = word;
			wordCount[i] = 1;
			length += 1;
		}
    		word = strtok(NULL, tokenizer);
	}

	if(argc == 1){
		for(int a = 0; a<length; a++){
			printf("%s\n", words[a]);
		}
	} else {

		for(int a = 0; a<length; a++){
                        printf("\t%d %s\n", wordCount[a], words[a]);
                }

	}





	return 0;
}
