#include"common.h"

#define NODE_COORDINATE_SIZE 32

unsigned short get_my_port(){
    return ntohs(my_addr.sin_port);
}

char* get_my_addr(){
    return inet_ntoa(my_addr.sin_addr);
}

int get_my_id(){
    return my_id;
}

void set_my_port(unsigned short port){
    my_addr.sin_port = htons((port));
}

void set_my_id(int id){
    my_id = id;
}

void trash_connexions(GList* list){
   GList *element = list;
   while(element != NULL){
       connexion_t* cnx = (connexion_t*)element->data;
       list = g_list_delete_link (list, element);
       free(cnx);
       free(list);
   }
}

void trash_acks(GHashTable* table){
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, table);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        // Remove nodes
        bool* ack = (bool*) value;
        g_hash_table_remove(table, key);
        free(ack);
    }
}

void trash_nodes(){
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, group);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        node_t *node = (node_t*) value;
        close(node->inbox->fd);
        close(node->outbox->fd);
        g_hash_table_remove(group, key);
        free(node);
    }
}

void trash_message_element(GList* list){
   GList *element = list;
   while(element != NULL){
       message_element_t* msg_elm = (message_element_t*)element->data;
       list = g_list_delete_link (list, element);
       free(msg_elm->msg);
       free(msg_elm);
       free(list);
   }
}

void trash_messages(GList *list){
    GList *element = list;
   while(element != NULL){
       message_t* msg = (message_t*)element->data;
       list = g_list_delete_link (list, element);
       free(msg);
       free(list);
   }
}

void program_halt(){
    trash_nodes();
    trash_connexions(connexions_pending);
    trash_messages(already_received);
    trash_messages(not_received_yet);
    trash_messages(delivered);
}

/** Initialize the structures of the group
 * @param file Name of the file containing the hosts. Host format must be in A.B.C.D:port
 */
int init(char *file, char *my_addr, int port){
    assert(file != NULL);
    assert(strcmp(file, "") != 0);
    assert(my_addr != NULL);
    assert(port != 0);
    assert(strcmp(my_addr, "") != 0);

    int group_count = 0;
    FILE *fd;
    char buf[NODE_COORDINATE_SIZE];
    int i = 0;
    char *addr;
    int remote_port;
    int node_id;
    char sep = ':';

    // Need to ignore sigpipe to avoid to
    // program to crash
    signal(SIGPIPE, SIG_IGN);
    
    terminate = false;
    set_my_port(port);

    DEBUG("Begin of initialization\n");
    DEBUG("Address : %s\n", my_addr);
    DEBUG("Listening port : %d\n", port);
    
    pthread_mutex_init(&send_sockets_mtx, NULL);
    pthread_mutex_init(&receive_sockets_mtx, NULL);
    
    fd = fopen(file, "r");

    // Get the size of the group by couting line of the hostfile.
    while(fgets(buf, sizeof(buf), fd) != NULL){
        group_count++;
    }

    // Generate the group
    group = g_hash_table_new(g_int_hash, g_int_equal);
    rewind(fd);
    while(fgets(buf, sizeof(buf), fd) != NULL){
        addr = strtok(buf, &sep);
        remote_port = atoi(strtok(NULL, &sep));
        node_id = atoi( strtok(NULL, &sep) );

        if( remote_port != port || strcmp(addr,my_addr) != 0){
            node_t* peer = node_create(NULL);
            peer->id = node_id ;
            peer->outbox = connexion_create(addr,remote_port);
            g_hash_table_insert(group, &peer->id, peer);
            i++;
        }
        else{
            DEBUG("Node id : %d\n", node_id);
            set_my_id(node_id);
        }
    }

    fclose(fd);
    
    listener_init();
    
    return EXIT_SUCCESS;
}
