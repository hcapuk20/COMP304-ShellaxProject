#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>

void printUsers(char *users[], int userCount){

        for(int i = 0; i<userCount; i++){
                printf("User %d : %s\n", i+1, users[i]);
        }

}


int main(int argc, char *argv[]){


	char *initial = "chatroom-";
	char *roomStr = argv[1];
	char roomName[strlen(roomStr) + strlen(initial) + 2];
	sprintf(roomName, "%s%s/",initial,roomStr);
	printf("Welcome to %s!\n", roomStr);

	struct stat s = {0};

	if (!stat(roomName, &s)){
  		//printf("'%s' is %sa directory.\n", roomName, (s.st_mode & S_IFDIR)  : "" ? "not ");
		//printf("is a dir\n");
	} else {
  		//printf("Creating the dir\n");
		mkdir(roomName, 0777);
	}

	char *username = argv[2];
	char userPath[strlen(roomName) + strlen(username) + 1];
	sprintf(userPath,"%s%s", roomName, username);


	char *users[1000];
	int userCount = 0;
	int userExists = 0;

 	struct dirent *currentDir;
  	DIR *dir = opendir(roomName);
  	if (dir) {
    		while ((currentDir = readdir(dir)) != NULL) {
			char *name = currentDir->d_name;
			if(strcmp(name, username) == 0) {
				userExists = 1;
			}
			if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0){
				users[userCount++] = name;
			}
    		}
    		closedir(dir);
	}

	if(!userExists){
		mkfifo(userPath, 0666);
		printf("User Pipe Created\n");
		users[userCount++] == username;
	}

	//printUsers(users, userCount);

	int fd;

	if(fork() == 0){
	 printf("[%s] %s > ", roomStr, username);
    	while (1)
    	{
		char strInput[100];
   		fgets(strInput, sizeof(strInput), stdin);
		char str[150];
		sprintf(str, "[%s] %s : %s", roomStr, username, strInput);

		for(int i = 0; i<strlen(str); i++){
			if(str[i] == '\n'){
				str[i] = '\0';
			}
		}

		for(int i = 0; i < userCount; i++){
			int pid = fork();
			if(pid == 0){
				char *pipeName = users[i];
				char pipePath[strlen(roomName) + strlen(pipeName) + 1];
                		sprintf(pipePath,"%s%s", roomName, pipeName);

				//printf("Current User Path: %s\n", pipePath);
				fd = open(pipePath, O_WRONLY);
				write(fd, str, strlen(str)+1);
				close(fd);

				exit(0);
			}
			//printf("Current Process Pid = %d\n", getpid());
		}

    	}
	} else {
		while(1){
			char text[100];
			fd = open(userPath, O_RDONLY);
                	read(fd, text, sizeof(text));
			if(text != NULL){
				if(strstr(text, username) != NULL){
					//printf("In contains\n\n");
					printf("\033[A\r%s\n[%s] %s > ", text, roomStr, username);
					fflush(stdout);
					//printf("\r[%s] %s > ", roomStr, username);
				} else {
					printf("\r%s\n[%s] %s > ", text, roomStr, username);
					fflush(stdout);
					//printf("[%s] %s > ", roomStr, username);
					//printf("[%s] %s > ", roomStr, username);
				}
			}
                	close(fd);
		}
	}



	return 0;
}


