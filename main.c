#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <termio.h>
#include <assert.h>

#define TEST 1
#define MAX_LEN 100

unsigned long int TimeCompare(int hour, int minuate);
void ShowHelp();

int main(int argc, char* argv[]){
    int fd,sfd;

    //data type
    typedef struct ALARM_SCHEDULE{
        int startMinate;
        int startHour;
        int finishMinate;
        int finishHour;
    }schedule;
    schedule rec;

    /*
        fprintf(stderr, "%s Manul : HowToUse\n", argv[0]);
        fprintf(stderr, "1. set : %s set hour minuate\n", argv[0]);
        fprintf(stderr, "2. confirm : %s confirm\n", argv[0]);
        fprintf(stderr, "3. remove : %s remove\n", argv[0]);
        return 0;
    */

    //open file

    if(!(TEST)){
        printf("Initializing...\n");
        sfd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);
        assert(sfd != -1);

        struct termios newtio;
        memset(&newtio, 0, sizeof(struct termios));
        newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR | ICRNL;
        newtio.c_oflag = 0;
        newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG);

        tcflush(sfd, TCIFLUSH);
        tcsetattr(sfd, TCSANOW, &newtio);
        printf("Done.\n");
    }  

    char receive[MAX_LEN];
    int readSize = 0;

    write(sfd,"OK",2);
    while(1){
        readSize = read(sfd,receive,MAX_LEN);
        //printf("%d ", readSize);    
        if(readSize == 2){
            printf("%s. Ready.\n",receive);
            break;
        } 
    }

    char *av[10] = {NULL, };
    int ac = 0, i = 0;
    char command[MAX_LEN];


    printf("Enter the Command.\nIf you check the menual, type 'help'.\n");

    while(1){
        if((fd=open("schedule", O_RDWR|O_CREAT, 0660)) == -1){
            fprintf(stderr, "err : file open");
            return -1;
        }

        printf("> ");
        fgets(command,MAX_LEN,stdin);
        command[strlen(command)-1]='\0';

        char *ptr;
        ptr = strtok(command, " ");
        //ptr = strtok(NULL, " ");
        ac = 0;
        while(ptr != NULL){
            av[i] = ptr;
            i++;    ac++;
            ptr = strtok(NULL, " ");
        }

        if(!(strcmp(av[0], "help"))){
            printf("\n");
            ShowHelp();
            printf("\n");
        }
        else if(!(strcmp(av[0], "set"))){
            if(ac!=3){
                fprintf(stderr, "1. set : %s [hour] [minuate]\n", av[0]);
                continue;
            }else if(atoi(av[1])<0 || atoi(av[1])>23){
                fprintf(stderr, "err : hour\n");
                continue;
            }else if(atoi(av[2])<0 || atoi(av[2])>59){
                fprintf(stderr, "err : munuate\n");
                continue;
            }
        //set
            rec.startHour=atoi(av[1]);
            rec.startMinate=atoi(av[2]);

            //printf("%s %d %d\n",av[1], rec.hour, rec.minuate);

            alarm(TimeCompare(rec.startHour, rec.startMinate));

            lseek(fd, 0L, SEEK_END);
            if(write(fd, &rec, sizeof(rec)) == sizeof(rec)){
                printf("Setting Success\n");
            }
            else{
                printf("Setting Failed. Try again.\n");
            }
        }
        //confirm : exeption
        else if(!(strcmp(av[0], "confirm"))){
            if(ac!=1){
                fprintf(stderr, "2. confirm : %s\n", av[0]);
                continue;
            }
            //confirm
            lseek(fd, 0L, SEEK_SET);
            while(1){
                printf("Checking...\n");
                if(read(fd, &rec, sizeof(rec))>0) printf("hour: %d minuate: %d\n", rec.startHour, rec.startMinate);
                else break;
            }
        }

        //remove : exeption
        else if(!(strcmp(av[0], "remove"))){
            if(ac!=1){
                fprintf(stderr, "3. remove : %s", av[0]);
                return 1;
            }
        //remove
            if((remove("./schedule"))==0)printf("remove success\n");
            else printf("remove fail\n");
        }

    
        else if(!(strcmp(av[0], "exit"))){
            close(fd);
            break;
        }

        else{
            fprintf(stderr,"\nCommand '%s' is not found. If you check the menual, type 'help'\n\n",av[0]);
        }

        close(fd);
    }

    close(sfd);
    return 0;
}


unsigned long int TimeCompare(int hour, int minuate){
        int chaHour=0, chaMinuate=0;
        unsigned long int sum=0;

        struct tm *t;
        time_t timer;
        timer=time(NULL);
        t=localtime(&timer);

        chaHour=abs(hour-(t->tm_hour));
        chaMinuate=abs(minuate-(t->tm_min));

        sum+=chaHour*3600;
        sum+=chaMinuate*60;

        return sum;
}


void ShowHelp(){
    printf("This is Automation Control Lamp. You can control the Lamp to the color you wnat.\n");
    fprintf(stderr, "- How to use this program? : help\n");
    fprintf(stderr, "- Setting Lamp Alarm : set [hour] [minuate]\n");
    fprintf(stderr, "- Confirm Lamp Alarm : confirm\n");
    fprintf(stderr, "- Remove Lamp Alarm : remove\n");
}