#include<getopt.h>
#include"../common.h"
#include"../group.h"
#include"../communication.h"

int main(int argc, char** argv){
    int opt;
    char* config;
    /* char buf[8]; */
    /* int sfd[3]; */
    int port;
  
    if(argc < 2){
        perror("Argument missing");
        exit(EXIT_FAILURE);
    }
  
    while( (opt = getopt(argc, argv, "f:p:") ) != -1){
        switch(opt){
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            config = optarg;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    init(config, "127.0.0.1", port);
    join();

    /* message_t msg; */
    /* msg.type = 'M'; */
    /* msg.content = NULL; */

    char msg[] = "test";
    
    urb((void*)msg, sizeof(msg));

    return EXIT_SUCCESS;
}
