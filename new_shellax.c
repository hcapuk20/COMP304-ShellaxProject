#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h> // termios, TCSANOW, ECHO, ICANON
#include <unistd.h>
#include <fcntl.h>

const char *sysname = "shellax";

enum return_codes {
    SUCCESS = 0,
    EXIT = 1,
    UNKNOWN = 2,
};

struct command_t {
    char *name;
    bool background;
    bool auto_complete;
    int arg_count;
    char **args;
    char *redirects[3];     // in/out redirection
    struct command_t *next; // for piping
};

/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t *command) {
    int i = 0;
    printf("Command: <%s>\n", command->name);
    printf("\tIs Background: %s\n", command->background ? "yes" : "no");
    printf("\tNeeds Auto-complete: %s\n", command->auto_complete ? "yes" : "no");
    printf("\tRedirects:\n");
    for (i = 0; i < 3; i++)
        printf("\t\t%d: %s\n", i,
               command->redirects[i] ? command->redirects[i] : "N/A");
    printf("\tArguments (%d):\n", command->arg_count);
    for (i = 0; i < command->arg_count; ++i)
        printf("\t\tArg %d: %s\n", i, command->args[i]);
    if (command->next) {
        printf("\tPiped to:\n");
        print_command(command->next);
    }
}
/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command) {
    if (command->arg_count) {
        for (int i = 0; i < command->arg_count; ++i)
            free(command->args[i]);
        free(command->args);
    }
    for (int i = 0; i < 3; ++i)
        if (command->redirects[i])
            free(command->redirects[i]);
    if (command->next) {
        free_command(command->next);
        command->next = NULL;
    }
    free(command->name);
    free(command);
    return 0;
}
/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt() {
    char cwd[1024], hostname[1024];
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
    printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
    return 0;
}
/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command) {
    const char *splitters = " \t"; // split at whitespace
    int index, len;
    len = strlen(buf);
    while (len > 0 && strchr(splitters, buf[0]) != NULL) // trim left whitespace
    {
        buf++;
        len--;
    }
    while (len > 0 && strchr(splitters, buf[len - 1]) != NULL)
        buf[--len] = 0; // trim right whitespace

    if (len > 0 && buf[len - 1] == '?') // auto-complete
        command->auto_complete = true;
    if (len > 0 && buf[len - 1] == '&') // background
        command->background = true;

    char *pch = strtok(buf, splitters);
    if (pch == NULL) {
        command->name = (char *)malloc(1);
        command->name[0] = 0;
    } else {
        command->name = (char *)malloc(strlen(pch) + 1);
        strcpy(command->name, pch);
    }

    command->args = (char **)malloc(sizeof(char *));

    int redirect_index;
    int arg_index = 0;
    char temp_buf[1024], *arg;
    while (1) {
        // tokenize input on splitters
        pch = strtok(NULL, splitters);
        if (!pch)
            break;
        arg = temp_buf;
        strcpy(arg, pch);
        len = strlen(arg);

        if (len == 0)
            continue; // empty arg, go for next
        while (len > 0 && strchr(splitters, arg[0]) != NULL) // trim left whitespace
        {
            arg++;
            len--;
        }
        while (len > 0 && strchr(splitters, arg[len - 1]) != NULL)
            arg[--len] = 0; // trim right whitespace
        if (len == 0)
            continue; // empty arg, go for next

        // piping to another command
        if (strcmp(arg, "|") == 0) {
            struct command_t *c = malloc(sizeof(struct command_t));
            int l = strlen(pch);
            pch[l] = splitters[0]; // restore strtok termination
            index = 1;
            while (pch[index] == ' ' || pch[index] == '\t')
                index++; // skip whitespaces

            parse_command(pch + index, c);
            pch[l] = 0; // put back strtok termination
            command->next = c;
            continue;
        }

        // background process
        if (strcmp(arg, "&") == 0)
            continue; // handled before

        // handle input redirection
        redirect_index = -1;
        if (arg[0] == '<')
            redirect_index = 0;
        if (arg[0] == '>') {
            if (len > 1 && arg[1] == '>') {
                redirect_index = 2;
                arg++;
                len--;
            } else
                redirect_index = 1;
        }
        if (redirect_index != -1) {
            command->redirects[redirect_index] = malloc(len);
            strcpy(command->redirects[redirect_index], arg + 1);
            continue;
        }

        // normal arguments
        if (len > 2 &&
            ((arg[0] == '"' && arg[len - 1] == '"') ||
             (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg
        {
            arg[--len] = 0;
            arg++;
        }
        command->args =
                (char **)realloc(command->args, sizeof(char *) * (arg_index + 1));
        command->args[arg_index] = (char *)malloc(len + 1);
        strcpy(command->args[arg_index++], arg);
    }
    command->arg_count = arg_index;

    // increase args size by 2
    command->args = (char **)realloc(command->args,
                                     sizeof(char *) * (command->arg_count += 2));

    // shift everything forward by 1
    for (int i = command->arg_count - 2; i > 0; --i)
        command->args[i] = command->args[i - 1];

    // set args[0] as a copy of name
    command->args[0] = strdup(command->name);
    // set args[arg_count-1] (last) to NULL
    command->args[command->arg_count - 1] = NULL;

    return 0;
}

void prompt_backspace() {
    putchar(8);   // go back 1
    putchar(' '); // write empty over
    putchar(8);   // go back 1 again
}
/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command) {
    int index = 0;
    char c;
    char buf[4096];
    static char oldbuf[4096];

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt
    static struct termios backup_termios, new_termios;
    tcgetattr(STDIN_FILENO, &backup_termios);
    new_termios = backup_termios;
    // ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL
    new_termios.c_lflag &=
            ~(ICANON |
              ECHO); // Also disable automatic echo. We manually echo each char.
    // Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    show_prompt();
    buf[0] = 0;
    while (1) {
        c = getchar();
        // printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

        if (c == 9) // handle tab
        {
            buf[index++] = '?'; // autocomplete
            break;
        }

        if (c == 127) // handle backspace
        {
            if (index > 0) {
                prompt_backspace();
                index--;
            }
            continue;
        }

        if (c == 27 || c == 91 || c == 66 || c == 67 || c == 68) {
            continue;
        }

        if (c == 65) // up arrow
        {
            while (index > 0) {
                prompt_backspace();
                index--;
            }

            char tmpbuf[4096];
            printf("%s", oldbuf);
            strcpy(tmpbuf, buf);
            strcpy(buf, oldbuf);
            strcpy(oldbuf, tmpbuf);
            index += strlen(buf);
            continue;
        }

        putchar(c); // echo the character
        buf[index++] = c;
        if (index >= sizeof(buf) - 1)
            break;
        if (c == '\n') // enter key
            break;
        if (c == 4) // Ctrl+D
            return EXIT;
    }
    if (index > 0 && buf[index - 1] == '\n') // trim newline from the end
        index--;
    buf[index++] = '\0'; // null terminate string

    strcpy(oldbuf, buf);

    parse_command(buf, command);

    // print_command(command); // DEBUG: uncomment for debugging

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
    return SUCCESS;
}
int process_command(struct command_t *command);
int main() {
    while (1) {
        struct command_t *command = malloc(sizeof(struct command_t));
        memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

        int code;
        code = prompt(command);
        if (code == EXIT)
            break;

        code = process_command(command);
        if (code == EXIT)
            break;

        free_command(command);
    }

    printf("\n");
    return 0;
}

int process_command(struct command_t *command) {
    int r;
    if (strcmp(command->name, "") == 0)
        return SUCCESS;

    if (strcmp(command->name, "exit") == 0)
        return EXIT;

    if (strcmp(command->name, "cd") == 0) {
        if (command->arg_count > 0) {
            r = chdir(command->args[0]);
            if (r == -1)
                printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
            return SUCCESS;
        }
    }

    pid_t pid = fork();
    if (pid==0) // child
    {
        if (command->background){
            pid_t pid2 = fork();
            if (pid2){
                exit(0);
            }
        }

        /// This shows how to do exec with environ (but is not available on MacOs)
        // extern char** environ; // environment variables
        // execvpe(command->name, command->args, environ); // exec+args+path+environ

        /// This shows how to do exec with auto-path resolve
        // add a NULL argument to the end of args, and the name to the beginning
        // as required by exec

        char *fileName = command->name;
        char currentDirPath[strlen(fileName)+3];
        char execPath[1000];
        char *path = getenv("PATH");
        char *dupPath = strdup(path);
        char *delim = ":";

        sprintf(currentDirPath,"./%s",command->name);
        if (access(currentDirPath,F_OK)==0){
            // command file exists in the current directory.
            //printf("command found in the current directory.\n");
            char cwd[1000];
            getcwd(cwd, sizeof(cwd));

            char cwdExecPath[strlen(cwd)+2];
            sprintf(cwdExecPath,"%s/%s",cwd,command->name);
            strcpy(execPath,cwdExecPath);

        } else {
            char *token = strtok(dupPath,delim);
            while (token != NULL){

                char filePath[strlen(token)+strlen(fileName)+2];
                sprintf(filePath,"%s/%s",token,fileName);

                if (access(filePath,F_OK)==0){
                    strcpy(execPath,filePath);
                    break;
                }

                token = strtok(NULL,delim);
            }
        }

        if (command->redirects[1] != NULL){
            fclose(fopen(command->redirects[1],"w"));
            int fd = open(command->redirects[1], O_WRONLY | O_APPEND);
            dup2(fd, 1);
            execv(execPath, command->args);
        } else if (command->redirects[2] != NULL){
            FILE *file;
            if ((file = fopen(command->redirects[2],"r"))){
                fclose(file);
            } else {
                fclose(fopen(command->redirects[2],"a"));
            }
            int fd = open(command->redirects[2], O_WRONLY | O_APPEND);
            dup2(fd, 1);
            execv(execPath, command->args);
        } else if (command->redirects[0] != NULL){

            char *newArgs[command->arg_count+1];
            for (int i = 0; i < command->arg_count; i++){
                newArgs[i] = command->args[i];
            }
            newArgs[command->arg_count-1] = command->redirects[0];
            newArgs[command->arg_count] = NULL;
            execv(execPath,newArgs);
        } else if (command->next != NULL){
            /// piped process

            // count the total number of processes
            int commandCount = 1;
            struct command_t *commandPtr = command;
            while (commandPtr->next != NULL){
                commandCount++;
                commandPtr = commandPtr->next;
            }
            int fds[commandCount-1][2];

            commandPtr = command;
            // executing children
            for (int i = 0; i < commandCount-1; i++){
                pipe(fds[i]);
                if (fork()==0){
                    if (i > 0){
                        dup2(fds[i-1][0],STDIN_FILENO);
                        close(fds[i-1][0]);
                    }
                    dup2(fds[i][1], STDOUT_FILENO);
                    // find the path of the command
                    dupPath = strdup(path);
                    fileName = commandPtr->name;
                    char *token = strtok(dupPath,delim);
                    char exectPath[1000];
                    char innerCurrentDirPath[strlen(fileName)+3];
                    sprintf(innerCurrentDirPath,"./%s",fileName);
                    if (access(innerCurrentDirPath,F_OK)==0){
                        // command file exists in the current directory.
                        //printf("command found in the current directory.\n");
                        char cwd[1000];
                        getcwd(cwd, sizeof(cwd));

                        char cwdExecPath[strlen(cwd)+2];
                        sprintf(cwdExecPath,"%s/%s",cwd,fileName);
                        strcpy(exectPath,cwdExecPath);
                        execv(exectPath,commandPtr->args);
                    } else {
                        while (token != NULL){
                            char filePath[strlen(token)+strlen(fileName)+2];
                            sprintf(filePath,"%s/%s",token,fileName);

                            if (access(filePath,F_OK)==0){

                                execv(filePath,commandPtr->args);
                                break;
                            }

                            token = strtok(NULL,delim);
                        }
                    }

                    close(fds[i][1]);

                    //perror("execv failed");

                    exit(1);
                }
                if (i > 0){
                    close(fds[i-1][0]);
                }
                close(fds[i][1]);

                commandPtr = commandPtr->next;

            }
            dup2(fds[commandCount-2][0],STDIN_FILENO);
            close(fds[commandCount-2][0]);
            close(fds[commandCount-2][1]);
            dupPath = strdup(path);
            fileName = commandPtr->name;
            char *token = strtok(dupPath,delim);
            char exectPath[1000];
            char finalCurrentDirPath[strlen(fileName)+3];
            sprintf(finalCurrentDirPath,"./%s",fileName);
            if (access(finalCurrentDirPath,F_OK)==0){
                // command file exists in the current directory.
                //printf("command found in the current directory.\n");
                char cwd[1000];
                getcwd(cwd, sizeof(cwd));

                char cwdExecPath[strlen(cwd)+2];
                sprintf(cwdExecPath,"%s/%s",cwd,fileName);
                strcpy(exectPath,cwdExecPath);
                execv(exectPath,commandPtr->args);
            } else {
                while (token != NULL){
                    char filePath[strlen(token)+strlen(fileName)+2];
                    sprintf(filePath,"%s/%s",token,fileName);

                    if (access(filePath,F_OK)==0){

                        execv(filePath,commandPtr->args);
                        break;
                    }

                    token = strtok(NULL,delim);
                }
                // find path of the last command and execute
            }
        }
        else {
            execv(execPath, command->args);
        }

        //execvp(command->name, command->args); // exec+args+path
        exit(0);


    }
    else
    {
        wait(0); // wait for child process to finish
        return SUCCESS;
    }

}
