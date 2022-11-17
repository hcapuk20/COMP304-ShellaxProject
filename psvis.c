#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    int pid = atoi(argv[1]);
    char *outputFile = argv[2];
    if (fork() == 0){
        char syscall[1000];
        // just to hold content in the dmesg before delete, so we do not get it as output on the terminal.
        system("sudo -S dmesg -c > prevlog.txt");
        sprintf(syscall,"sudo -S insmod mymodule.ko given_pid=%d",pid);
        system(syscall);
        system("sudo -S dmesg | grep -e '->' > dm.txt");
        system("sudo -S dmesg | grep color > dm2.txt");

        system("sudo -S rmmod mymodule");
        FILE *readFile ,*readFile2, *writeFile;

        readFile = fopen("./dm.txt","r");
        readFile2 = fopen("./dm2.txt","r");

        writeFile = fopen("graph.gv","w");

        fprintf(writeFile,"digraph dg { \n");

        char line[1000];
        char *delim = "]";
        char *cut;
        while (fgets(line, sizeof(line), readFile)) {
            cut = strtok(line,delim);
            cut = strtok(NULL,delim);
            fprintf(writeFile,"%s",cut);
        }
        char line2[1000];
        char *cut2;
        while (fgets(line2, sizeof(line2), readFile2)) {
            cut2 = strtok(line2,delim);
            cut2 = strtok(NULL,delim);
            fprintf(writeFile,"%s]\n",cut2);
        }
        fprintf(writeFile,"}");
        fclose(readFile);
        fclose(readFile2);
        fclose(writeFile);
        // dot -Tpng graph.gv -o psvis.png
        char finalCall[1000];
        sprintf(finalCall,"dot -Tpng graph.gv -o %s",outputFile);
        system(finalCall);
    } else {
        wait(NULL);
        // delete temporary files
        system("rm dm.txt dm2.txt prevlog.txt graph.gv");
    }


    return 0;
}

