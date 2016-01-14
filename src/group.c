#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>

#include"group.h"
#include"listener.h"
#include"node.h"
#include"communication.h"

/********************************************
 *
 *             Functions
 *
 *******************************************/

int connexion(connexion_t* cnx){
    assert(cnx);
    
    cnx->fd = socket(AF_INET, SOCK_STREAM,0);
    if(cnx->fd == -1){
        perror("Failed to create the socket");
        return EXIT_FAILURE;    
    }
    
    if(connect(cnx->fd, (struct sockaddr*) &(cnx->infos), sizeof(cnx->infos)) < 0){
        perror("Failed to name the socket");
        return EXIT_FAILURE;    
    }

    int set = 1;
    
    message_id_t msg;
    msg.type = 'I';
    msg.node_id = my_id;
    if( send(cnx->fd, &msg, sizeof(msg), 0) > 0){
        DEBUG_SEND("Send id %d to [%s:%d][%d]\n", my_id, inet_ntoa(cnx->infos.sin_addr), ntohs(cnx->infos.sin_port), cnx->fd);
        message_t heartbeat;
        heartbeat.type = 'H';
        heartbeat.node_id = my_id;
        heartbeat.id = -1;
        multicast(&heartbeat, sizeof(message_t));
    }
    else{
        DEBUG_SEND("Failed to send id %d to [%s:%d][%d]\n", my_id, inet_ntoa(cnx->infos.sin_addr), ntohs(cnx->infos.sin_port), cnx->fd);
    }
    
    return EXIT_SUCCESS;
}

bool is_node_active(int node_id){
    return get_node_by_id(node_id)->in_connected;
}

node_t* get_node_by_id(int node_id){
    return (node_t*) g_hash_table_lookup(group, &node_id);    
}

void foreach_connect(gpointer key, gpointer value, gpointer userdata){
    node_t *node = (node_t*) value;
    if(pthread_mutex_lock(&node->mtx) != 0){
        perror("Failed to lock the node in foreach connect");
        exit(EXIT_FAILURE);
    }
    if(!node->out_connected){
        if(connexion(node->outbox) == EXIT_SUCCESS){
            node->out_connected = true;
            DEBUG("Connected to [%s:%d][%d]\n", inet_ntoa(node->outbox->infos.sin_addr), ntohs(node->outbox->infos.sin_port), node->outbox->fd);
        }
        else{
            node->out_connected = false;
            DEBUG_ERR("Failed to connect to [%s:%d][%d]\n", inet_ntoa(node->outbox->infos.sin_addr), ntohs(node->outbox->infos.sin_port), node->outbox->fd);
        }
    }
    if(pthread_mutex_unlock(&node->mtx) != 0){
        perror("Failed to unlock the node in foreach connect");
        exit(EXIT_FAILURE);
    }
}

/** Join the group
 *
 */
void join(){
    int *fds = NULL;
    fds = calloc(g_hash_table_size(group), sizeof(int));

    g_hash_table_foreach(group, (GHFunc)foreach_connect, NULL);
}
    
/** Add a node to the receiving list
 * @param fd File descriptor associated to the addr
 * @param addr Address information of the incomming connexion
 * @return 0 if ok 1 if fails
 */
/* int add_node(const int fd, const struct sockaddr_in addr){ */
int add_node(connexion_t* cnx, const int node_id){
    assert(cnx);
    assert(cnx->fd > 2); // To be sure the fd is valid

    node_t* node = g_hash_table_lookup(group, &node_id);
    if(node){
        if(pthread_mutex_lock(&node->mtx) != 0){
            perror("Failed to lock node when adding node");
            exit(EXIT_FAILURE);
        }
        
        node->inbox = cnx;
        node->in_connected = true;
        node->alive = true;
        node_update_time(&node->time);
        
        if(pthread_mutex_unlock(&node->mtx) != 0){
            perror("Failed to unlock node when adding node");
            exit(EXIT_FAILURE);
        }
        return EXIT_SUCCESS;
    }
    else{
        node->in_connected = false;
        node->alive = false;
        return EXIT_FAILURE;
    }
}

void remove_node(node_t* node){
    free(node);
}

void* message_handler(){
    join();
    sleep(2);
    return NULL;
}

