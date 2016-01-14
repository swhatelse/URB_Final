#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<getopt.h>
#include<string.h>
#include<assert.h>
#include<unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include"listener.h"
#include"group.h"
#include"common.h"
#include"communication.h"

void* interpreter(){
    char msg[] = "test";

    printf("Type 's' to send a message 'h' to halt\n");
    char cmd;
    while(!terminate){
        cmd = getc(stdin);
        switch(cmd){
        case 's':
            urb((void*)msg, sizeof(msg));
            break;
        case 'h':
            terminate = true;
            break;
        default:
            break;
        }
    }
}

int main(int argc, char *argv[]){
    int opt;
    pthread_t tsid, tcid, tiid;
    char* hostfile = NULL;
    int port = 0;

    if(argc < 3){
        fprintf(stderr,"arguments missing");
        return EXIT_FAILURE;
    }

    while( (opt = getopt(argc, argv, "hp:f:") ) != -1){
        switch(opt){
        case 'h':
            fprintf(stderr,"Usage: ./main -f [hostfile] -p [port]");
            break;
        case 'f':
            hostfile = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 't':
            program_halt();
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    // TODO: make it proper
    init(hostfile, "127.0.0.1", port);
    
    pthread_create(&tsid,NULL,&listener_run, NULL);
    sleep(2);

    pthread_create(&tcid,NULL,&message_handler, NULL);
    pthread_join(tcid, NULL);

    pthread_create(&tiid,NULL,&interpreter, NULL);
    
    pthread_join(tsid, NULL);

    return EXIT_SUCCESS;
}
