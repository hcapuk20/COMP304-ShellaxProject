#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if (argc != 2){
        printf("You should only enter 'minutes' as a parameter. Example usage: wiseman 3\n");
        return 1;
    }

    char command[100];

    int minute = atoi(argv[1]);

    sprintf(command,"*/%d * * * * fortune | espeak --stdout | aplay",minute); 

    char systemCall[200];

    sprintf(systemCall, "echo '%s' | crontab -",command);

    system(systemCall);

    return 0;
}

