#ifndef GROUP_H
#define GROUP_H
#include<stdio.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<glib.h>

#include"common.h"
#include"node.h"

// Types
typedef struct group_t{
    node_t **nodes;
    int count;
}group_t;

// Globals
group_t send_sockets;
pthread_mutex_t send_sockets_mtx;
group_t receive_sockets;
pthread_mutex_t receive_sockets_mtx;
GHashTable* group;

// Functions
int connexion(connexion_t* cnx);
bool is_node_active(int node_id);
node_t* get_node_by_id(int node_id);
void join();
int add_node(connexion_t* cnx, const int node_id);
void remove_node(node_t* node);
void* message_handler();
#endif
