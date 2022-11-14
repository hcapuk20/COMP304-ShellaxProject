#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>


void printList(char *list[], int length){

        for(int i = 0; i<length; i++){
                printf("%d. %s\n", i+1 ,list[i]);
        }

}

char* adjustInput(char* input){
	for(int a = 0; a < strlen(input) - 1 ; a++){
                if(!isprint(input[a+1])){
                        input[a+1] = '\0';
			return input;
                }
        }
	return "";
}

char* printFileContent(char* path, char* text){
	int index = 0;
	char content;
	FILE *fp = fopen(path, "r");
	content = fgetc(fp);
        while (content != EOF){
        	printf ("%c", content);
		text[index++] = content;
                content = fgetc(fp);
	}
	fclose(fp);
	text[index] = '\0';
	return text;
}


void overWrite(char* path, char* text){
	FILE *fp;
        fp = fopen(path, "w");
        fputs(text, fp);
        fclose(fp);
}

int main(int argc, char *argv[]) {

        char *bookName = argv[1];
        char bookPath[strlen(bookName) + 1];
        sprintf(bookPath, "%s",bookName);
        printf("-- %s --\n", bookPath);

        struct stat s = {0};

        if (!stat(bookPath, &s)){
                //printf("'%s' is %sa directory.\n", roomName, (s.st_mode & S_IFDIR)  : "" ? "not ");
                //printf("is a dir\n");
        } else {
                //printf("Creating the dir\n");
                mkdir(bookPath, 0777);
        }


        int finish = 0;

        while(finish == 0){

        char *sections[1000];
        int sectionCount = 0;


	struct dirent *currentDir;
        DIR *dir = opendir(bookPath);
        if (dir) {
                while ((currentDir = readdir(dir)) != NULL) {
                        char *name = currentDir->d_name;

                        if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0){
                                sections[sectionCount++] = name;
                        }
                }
                closedir(dir);
        }


	char section[100];

	while(1){

	printf("\n");
	if(sectionCount > 0){
		printf("Sections:\n");
		printList(sections, sectionCount);
		printf("Name of the section you want to enter or create: ");
	} else {
		printf("This notebook is empty! Add a section!\n");
		printf("Name of the section you want to create: ");
	}

        fgets(section, sizeof(section), stdin);
	//scanf("%s", section);
	fflush(stdin);

	strcpy(section, adjustInput(section));

	char sure[100];
	printf("Is this the section you want to enter: %s ? (type yes or no)\n", section);
	fgets(sure, sizeof(sure), stdin);
	sure[2] = '\0';
	if(strcmp(sure, "no") != 0){
		break;
	}
	}


	if(strcmp(section,"") == 0){
		printf("Blank section is not accepted!\n");
		continue;
	}

	if(strcmp(section, "close") == 0){
		finish = 1;
		continue;
	}

	//printf("SECTION = %s\n", section);

        char sectionPath[strlen(bookPath) + strlen(section) + 2];
        sprintf(sectionPath,"%s/%s", bookPath, section);


	int exists = 0;
	for(int i = 0; i<sectionCount; i++){
		if(strcmp(sections[i], section) == 0){ exists = 1; }
	}

	char text[2000];

	if(exists){
        	printf("\n------%s------\n", section);
        	strcpy(text,printFileContent(sectionPath,text));
        	printf("------%s------\n\n", section);
	} else {
		strcpy(text, "");
		sections[sectionCount++] = section;
	}

	//printf("Path: %s\n", sectionPath);

	int exit = 0;
	while(exit == 0){
	FILE *fp;
	fp = fopen(sectionPath, "a");

	//char *newNotes[500];
	//int newNoteIndex;

	int write = 0;
	while(write == 0){
		printf("Your Note: ");
		char note[1000];
		fgets(note, sizeof(note), stdin);
		fflush(stdin);
		strcpy(note, adjustInput(note));
		//fprintf(fp, "%s", note);

		if(strcmp(note, "exit") == 0){
			//printf("%s\n", text);
			exit = 1;
			write = 1;
			continue;
		}

		if(strcmp(note, "delete") == 0 || strcmp(note, "del") == 0){
			int indexLast = 0;
			int indexSecond = 0;
			int index = 0;
			while(text[index] != '\0'){
				if(text[index] == '\n'){
					indexLast = indexSecond;
					indexSecond = index;
				}
				index++;
				//printf("Index %d, indexLast: %d,  indexSecond:%d\n", index, indexLast,indexSecond);
			}
			if(indexLast != 0){
				text[indexLast+1] = '\0';
			} else if(indexSecond != 0){
				strcpy(text, "");
			}
			write = 1;
			continue;
		}


		if(strcmp(note, "print") == 0){
			//printFileContent(sectionPath);
			printf("\n------%s------\n", section);
			printf("%s", text);
			printf("------%s------\n\n", section);
			continue;
		}

		strcat(text, note);
		strcat(text, "\n");
		fputs(note, fp);
		fputs("\n", fp);
	}

	fclose(fp);
	overWrite(sectionPath, text);
	}
	}


	/*
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



	*/

        return 0;
}
