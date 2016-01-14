#include<assert.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/time.h>

#include"listener.h"
#include"communication.h"

/********************************************
 *
 *             Macros
 *
 *******************************************/

#define BACKLOG 50

/********************************************
 *
 *             Functions
 *
 *******************************************/

/** Put the connexion in the waiting queue
 * Connexions are put in standby in order to wait for the remote to
 * Identify it self by sending its id.
 * @param fd File descriptor of the incomming connexion.
 * @param addr Address informations of the incomming connexion.
 */
void connexion_pending_add(int fd, struct sockaddr_in addr){
    connexion_t* cnx = malloc(sizeof(connexion_t)); 
    cnx->fd = fd;
    cnx->infos = addr;

    connexions_pending = g_list_append(connexions_pending, (gpointer)cnx);
}

/** Extract a connexion from the pending list.
 *
 */
connexion_t* connexion_pending_pop(GList* cnx_pnd){
    connexion_t* cnx = (connexion_t*)cnx_pnd->data;
    connexions_pending = g_list_remove(connexions_pending, cnx);
    return cnx;
}

/** Pop and destroy the connexion
 *
 */
void connexion_pending_remove(GList* cnx_pnd){
    connexion_t* cnx =  (connexion_t*)cnx_pnd->data;
    connexions_pending = g_list_remove(connexions_pending, cnx);
    free(cnx);
    cnx = NULL;
}

GList* connexions_pending_get(int fd){
    GList* current;
    for(current = connexions_pending; current != NULL; current = current->next){
        connexion_t* cnx = (connexion_t*)current->data;
        if(cnx->fd == fd){
            return current;
        }
    }    
    return NULL;
}

void handle_id(message_id_t* msg){
    PRINT("Node identified");
    printf("%d\n", msg->node_id);    
}

/** Manage the received acks
 * @param ack The received acknowledgment.
 * @param sender The node which sent ack.
 */
void handle_ack(message_t* ack, node_t* sender){
    // First check if we already received the corresponding message.
    // If not create an entry in the already_received list and add the ack.
    // The message will be filled later when received.
    GList* element_list = get_msg_from_list(delivered, ack);
    
    // If already delivered skip it
    if(!element_list){
        element_list = get_msg_from_list(already_received, ack);
        message_element_t* element_msg;
        if(!element_list){
            // Received an ack for a message we didn't received yet
            GList* element_list = get_msg_from_list(not_received_yet, ack);
            if(!element_list){
                // Message not received yet and received the first ack for it
                // Create an empty message to register the ack and register them
                // for the futur message.
                message_t* msg = malloc(sizeof(message_t));
                msg->node_id = ack->node_id;
                msg->id = ack->id;
                msg->content = NULL;
                insert_message(msg, &not_received_yet);
            }
            else{
                element_msg = (message_element_t*)element_list->data;
                add_ack(element_msg, &sender->id);    
            }
        }
        else{
            element_msg = (message_element_t*)element_list->data;
            add_ack(element_msg, &sender->id);
            if(is_replicated(element_msg)){
                deliver(element_msg);
            }
        }
    }
    else{
        
    }
}

void handle_normal(message_t* msg, node_t* sender){
    if(!is_already_in(delivered, msg)){
        // Message have not received yet
        if(!is_already_in(already_received, msg)){
            GList* element = get_msg_from_list(not_received_yet, msg);
            // Check is the message is already referenced in the not_received yet
            if(element){
                message_element_t* msg_elmnt = (message_element_t*)element->data;
                not_received_yet = g_list_remove(not_received_yet, msg_elmnt);
                already_received = g_list_append(already_received, msg_elmnt);
                add_ack(msg_elmnt, &my_id);
            }
            else{
                // Completely new message
                insert_message(msg, &already_received);
            }
            /* DEBUG_RECV("[%d][%d] Message received from [%s:%d][%d]\n", msg->node_id, msg->id, inet_ntoa(sender->inbox->infos.sin_addr), ntohs(sender->inbox->infos.sin_port), sender->inbox->fd); */
            DEBUG_RECV("[%d][%d] Message received from [%d]\n", msg->node_id, msg->id, sender->id);
            acknowledge(*msg);
            multicast(msg, sizeof(message_t));
            DEBUG_SEND("[%d][%d] Retransmited\n", msg->node_id, msg->id);
        }
        else{
            // Message already received, we can drop it
            free(msg);
            msg = NULL;
        }
    }
    else{
        free(msg);
        msg = NULL;
    }
}

void handle_message(message_t* msg, node_t* sender){
    node_update_time(&sender->time);
    switch(msg->type){
    case 'M':
         handle_normal(msg, sender);
        break;
    case 'A':
        handle_ack((message_t*)msg, sender);
        break;
    case 'H':
        // Received heartbeat
        break;
    default:
        PRINT("Unknown type");
        break;
    }
}

int connexion_accept(){
    int cfd;
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_size = sizeof(struct sockaddr_in);
  
    cfd = accept(listening_fd, (struct sockaddr*) &peer_addr, &peer_addr_size);
    if(cfd < 0){
        perror("Failed to accept\n");
    }

    FD_SET(cfd, &reception_fd_set);
    connexion_pending_add(cfd, peer_addr);    
    
    DEBUG("[?] Client request [%s:%d][%d]\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), cfd);
    
    return cfd;
}


void handle_connexion_requests(fd_set active_set){
     message_id_t msg;
    
     // First connexion step
     if(FD_ISSET(listening_fd, &active_set)){
          connexion_accept();
     }

     GList* current;
     for(current = connexions_pending; current != NULL; current = current->next){
         connexion_t* cnx = (connexion_t*)current->data;
         if(cnx && FD_ISSET(cnx->fd, &active_set)){
             bool retval = recv_all(cnx->fd, (void*)&msg, sizeof(message_id_t));
             // Client has validated the connexion
             // Remove it from pending connexion and register it using its id.
             if(retval){
                 DEBUG("[%d] Client validation validated [%s:%d][%d]\n", msg.node_id, inet_ntoa(cnx->infos.sin_addr), ntohs(cnx->infos.sin_port), cnx->fd);
                 // Register the node in the incomming connexion
                 add_node(cnx, msg.node_id);
                 connexions_pending = g_list_remove(connexions_pending, cnx);
                 // If the sending connexion is not establish, establishes it
                 node_t* node = get_node_by_id(msg.node_id);
                 if(!node->out_connected){
                     DEBUG_ERR("Rejoin %d / %d\n", msg.node_id, node->id);
                     connexion(node->outbox);
                     node->out_connected = true;
                 }
             }
             else{
                 // Disconnection
                 if(cnx){
                     DEBUG("[?] Client validation aborted [%s:%d][%d]\n", inet_ntoa(cnx->infos.sin_addr), ntohs(cnx->infos.sin_port), cnx->fd);
                     FD_CLR(cnx->fd, &reception_fd_set);
                     close(cnx->fd);
                     connexion_pending_remove(current);
                     connexions_pending = g_list_remove(connexions_pending, cnx);
                 }
             }
         }
     }
}

void handle_disconnexion(int index){
    node_t* node = g_hash_table_lookup(group, &index);
    if(node->inbox){
        DEBUG("[%d] Deconnexion\n", node->id);
        FD_CLR(node->inbox->fd, &reception_fd_set);
        close(node->inbox->fd);
        node->inbox = NULL;
        close(node->outbox->fd);
        node->outbox = NULL;
    }
}

void handle_event_group(gpointer key, gpointer value, gpointer userdata){
    node_t* node = (node_t*) value;
    fd_set active_set = *(fd_set*)userdata;
    if(node != NULL && node->inbox != NULL){
        if(FD_ISSET(node->inbox->fd, &active_set)){
            message_t *msg = malloc(sizeof(message_t));
            // The size of message_t is used here because it is the longuest we can receive.
            bool retval = recv_all(node->inbox->fd, (void*)msg, sizeof(message_t));
            if(retval){
                handle_message(msg, node);
            }
            else{
                free(msg);
                handle_disconnexion(*(int*)key);
            }
        }
    }
}

void handle_event(fd_set active_set){
    handle_connexion_requests(active_set);
    g_hash_table_foreach(group, handle_event_group, &active_set);
}

/** Prepare the socket for waiting connexion
 *
 */
void listener_init(){
    connexions_pending = NULL;
    already_received = NULL;
    not_received_yet = NULL;
    delivered = NULL;
    
    PRINT("Initialization of the listener");
    
    listening_fd = socket(AF_INET, SOCK_STREAM,0);
    if(listening_fd < 0){
        perror("Failed to attribute the socket\n");
        exit(EXIT_FAILURE);
    }

    int optval=1;
    setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    fcntl(listening_fd, F_SETFL, O_NONBLOCK);
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(listening_fd, (struct sockaddr*) &my_addr, sizeof(my_addr)) < 0 ){
        perror("Failed to name the socket\n");
        exit(EXIT_FAILURE);    
    }

    if(listen(listening_fd, BACKLOG) < 0){
        perror("Failed to listen\n");
        exit(EXIT_FAILURE);    
    }
}

void* listener_run(){
    struct timeval timeout;
    int event = 0;
    
    FD_ZERO(&reception_fd_set);
    FD_SET(listening_fd, &reception_fd_set);

    DEBUG("Start to listen\n");
    DEBUG("=============================================\n");
    while(!terminate){
        // Timeout needs to be reset each time
        timeout.tv_sec = 0;
        timeout.tv_usec = 150000;

        fd_set active_set;
        active_set = reception_fd_set;

        event = select(FD_SETSIZE, &active_set, NULL, NULL, &timeout);
        if(event == -1){
            perror("Select failed");
        }
        else if(event){
            handle_event(active_set);
        }
        else{
            
        }
    }
    return NULL;
}
