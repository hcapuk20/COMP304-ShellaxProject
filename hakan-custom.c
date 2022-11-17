#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
void red(){
    printf("\033[1;31m");
}
void bgred(){
    printf("\033[1;41m");
}
void yellow(){
    printf("\033[1;33m");
}
void bgyellow(){
    printf("\033[1;43m");
}
void black(){
    printf("\033[1;30m");
}
void bgblack(){
    printf("\033[1;40m");
}
void green(){
    printf("\033[1;32m");
}
void bggreen(){
    printf("\033[1;42m");
}
void purple(){
    printf("\033[1;35m");
}
void bgpurple(){
    printf("\033[1;45m");
}
void blue(){
    printf("\033[1;34m");
}
void bgblue(){
    printf("\033[1;44m");
}
void cyan(){
    printf("\033[1;36m");
}
void bgcyan(){
    printf("\033[1;46m");
}
void white(){
    printf("\033[1;37m");
}
void bgwhite(){
    printf("\033[1;47m");
}
void reset(){
    printf("\033[0m");
}
int main(int argc, char *argv[]) {

// custom command written by Hakan Çapuk
    char *sentence = argv[2];
    char *option = argv[1];

    if (strcmp(option,"--red")==0){
        red();
        printf("%s\n",sentence);
        reset();
    } else if (strcmp(option,"--yellow")==0){
        yellow();
        printf("%s\n",sentence);
        reset();
    } else if (strcmp(option,"--black")==0){
        black();
        printf("%s\n",sentence);
        reset();
    } else if (strcmp(option,"-green")==0){
        green();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--purple")==0){
        purple();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--blue")==0){
        blue();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--cyan")==0){
        cyan();
        printf("%s\n",sentence);
        reset();
    } else if (strcmp(option,"--white")==0){
        white();
        printf("%s\n",sentence);
        reset();
    } else if (strcmp(option,"--bgwhite")==0){
        bgwhite();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bgblack")==0){
        bgblack();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bggreen")==0){
        bggreen();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bgred")==0){
        bgred();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bgyellow")==0){
        bgyellow();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bgblue")==0){
        bgblue();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bgcyan")==0){
        bgcyan();
        printf("%s\n",sentence);
        reset();
    }else if (strcmp(option,"--bgpurple")==0){
        bgpurple();
        printf("%s\n",sentence);
        reset();
    }

    else if (strcmp(option,"--random")==0){
        char input[strlen(sentence)+1];
        strcpy(input,sentence);
        srand(time(NULL));
        for (int i = 0; i < strlen(sentence); i++){
            char color[10];
            int colorNum = rand() % 8;
            sprintf(color,"\033[1;3%dm",colorNum);
            printf("%s",color);
            printf("%c",input[i]);
        }
        reset();
        printf("\n");
    } else if (strcmp(option,"--randombg")==0){
        char input[strlen(sentence)+1];
        strcpy(input,sentence);
        srand(time(NULL));
        for (int i = 0; i < strlen(sentence); i++){
            char color[10];
            int colorNum = rand() % 8;
            sprintf(color,"\033[1;4%dm",colorNum);
            printf("%s",color);
            printf("%c",input[i]);
        }
        reset();
        printf("\n");
    } else if (strcmp(option,"--rainbow")==0){
        char input[strlen(sentence)+1];
        strcpy(input,sentence);
        srand(time(NULL));
        for (int i = 0; i < strlen(sentence); i++){
            char color[10];
            char colorbg[10];
            int colorNum = rand() % 8;
            int bgNum = rand() % 8;
            sprintf(color,"\033[1;3%dm",colorNum);
            sprintf(colorbg,"\033[1;4%dm",bgNum);

            printf("%s",color);
            printf("%s",colorbg);
            printf("%c",input[i]);
        }
        reset();
        printf("\n");
    }
    else if (strcmp(option,"--cow")==0){
        char syscall[100];
        sprintf(syscall,"./colorize --random %s | cowsay",sentence);
        system(syscall);
    } else if (strcmp(option, "--chcolor")==0){
        // printf("\r%s", your_string_with_a_new_color)
        printf("%s\n",sentence);
        for (int i = 0; i < 7; i++){
            char color[10];
            sprintf(color,"\033[1;3%dm",i);
            printf("%s",color);
            printf("%s", sentence);
            printf("\n");
            sleep(1);
        }
    }

    else {
        printf("Invalid argument given.\n");
        return 1;
    }
    reset();
    return 0;

}
