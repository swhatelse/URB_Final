#include<getopt.h>
#include"../listener.h"

int main(int argc, char** argv){
    int opt;
    int port;
    char* hostfile = NULL;
    
    if(argc < 2){
        perror("Argument missing");
        exit(EXIT_FAILURE);
    }
  
    while( (opt = getopt(argc, argv, "p:f:") ) != -1){
        switch(opt){
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            hostfile = optarg;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    init(hostfile, "127.0.0.1", port);
    listener_init(port);
    listener_run();
    return EXIT_SUCCESS;
}
