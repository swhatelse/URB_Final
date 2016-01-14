#ifndef NODE_H
#define NODE_H

#include<stdbool.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<pthread.h>

// Types
typedef struct connexion_t{
    struct sockaddr_in infos;
    int fd;
}connexion_t;


typedef struct node_t{
    connexion_t* inbox; // Connexion for accepting connexions
    connexion_t* outbox; // Connexion for accepting connexions
    int id;
    bool in_connected;  // True when inbox established
    bool out_connected; // True when outbox established
    bool alive;
    struct timeval time; // Use for the heartbeat checking
    pthread_mutex_t mtx;
}node_t;

// Globals
int my_id;

// Functions
int get_node_port(node_t* node);
int get_node_addr(node_t* node);
int get_node_fd(node_t* node);

void node_update_time(struct timeval *tv);
node_t* node_create(connexion_t* cnx);
connexion_t* connexion_create(char* addr, int port);
bool is_the_same_node(const node_t node1, const node_t node2);
#endif
