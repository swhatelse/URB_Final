#include<stdio.h>
#include<stdbool.h>
#include<glib.h>

#include"common.h"

#ifndef COMMUNICATION
#define COMMUNICATION

// Type definition
typedef struct message_t{
    char type;
    int id; // Id of the message
    /* node_t sender; */
    int node_id; // Id of origin node
    size_t size;
    void* content;
}message_t;

// Used to authenticate a node
typedef struct message_id_t{
    char type;
    int node_id;
}message_id_t;

typedef struct message_element_t{
    message_t* msg;
    GHashTable* acks;
}message_element_t;

// Global vars
GList* already_received;
GList* not_received_yet;
GList* delivered;

// Functions
void insert_message(message_t* msg, GList** list);
GList* get_msg_from_list(GList* list, message_t* msg);
bool is_already_in(GList* list, message_t* msg);
bool is_already_delivered(GList* list, message_t* msg);

GHashTable* acks_create();
void acknowledge(message_t msg);
void add_ack(message_element_t* msg, int* node_id);

void multicast(message_t* msg, size_t size);
int urb(const void* content, size_t size);
bool recv_all(int socket, void* buf, size_t length);

void deliver(message_element_t* message);
bool is_replicated(message_element_t* msg);
#endif
