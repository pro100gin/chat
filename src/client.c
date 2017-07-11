#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <time.h>
#include "../include/common.h"

CLIENT_LIST_STRUCT *cl_head, *cl_tail;

void* msg_rcv(void* a){
    key_t key;
    int msgid;
    ssize_t bytes_read;
    long pid;
    key = ftok("server", 'B');/*?????????*/
    CLIENT_LIST_STRUCT  *current;
    MESSAGE_BUFFER *rcv_msg;
    rcv_msg = malloc(sizeof(MESSAGE_BUFFER));
    msgid = msgget(key, 0);
    if (msgid == -1) {
      perror("msgget failed with error");
      exit(EXIT_FAILURE);
    }
    pid = getpid();
    do {
        /* receive the message */
        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(MESSAGE_BUFFER), pid, 0);
        CHECK(bytes_read >= 0);
        printf("\nmsg_receive: %s\n", rcv_msg->mtext);
    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup */
    msgctl(msgid, IPC_RMID, 0);
}

void* msg_send(void* a){
    key_t key;
    int msgid;
    char buffer[MAX_SIZE + 1];
    ssize_t bytes_read;
    key = ftok("server", 'B');
    MESSAGE_BUFFER *snd_msg;
    snd_msg = malloc(sizeof(MESSAGE_BUFFER));

    msgid = msgget(key, 0);
    if (msgid == -1) {
      perror("msgget failed with error1");
      exit(EXIT_FAILURE);
    }
    while(1){
        fgets(buffer, MAX_SIZE, stdin);
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
        snd_msg->mtype = getpid();
        strncpy(snd_msg->mtext, buffer, MAX_SIZE);
        msgsnd(msgid, (void*)snd_msg, sizeof(MESSAGE_BUFFER), 0);
    }
    msgctl(msgid, IPC_RMID, 0);
}

void* msg_info(void* a){
    key_t key;
    int msgid;
    ssize_t bytes_read;
    char buffer[21];
    key = ftok("server", 'A');
    CLIENT_LIST_STRUCT *rcv_msg, *current;
    rcv_msg = malloc(sizeof(CLIENT_LIST_STRUCT));
    current = malloc(sizeof(CLIENT_LIST_STRUCT));
    

    msgid = msgget(key, 0);
    if (msgid == -1) {
      perror("msgget failed with error2");
      exit(EXIT_FAILURE);
    }

    current->prio = getpid();
    current->msg_type = MSG_CONNECT;
    
    fgets(buffer, MAX_SIZE, stdin);
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
    }
    strncpy(current->name, buffer, 20);

    msgsnd(msgid, (void*)current, sizeof(MESSAGE_BUFFER), 0);
    
    /*
        add msg_receive to arrey with names
    */

    do {
        /* receive the message */
        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(CLIENT_LIST_STRUCT), 1L, 0);
        CHECK(bytes_read >= 0);
        switch(rcv_msg->msg_type){
            case MSG_CONNECT:/*TODO add mutex to sync*/
                rcv_msg->next = head->next;
                head->next->prev = rcv_msg;
                rcv_msg->prev = head;
                head-> next = rcv_msg;
                current = head->next;
                break;
            case MSG_DISCONNECT:/*TODO add mutex to sync*/
                current = head->next;
                while(current != tail){
                    if(current->prio == rcv_msg->prio){
                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                        free(current);
                        break;
                    }
                    current = current->next;
                }
                break;
        }
    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup */
    msgctl(msgid, IPC_RMID, 0);
}

int main(int argc, char **argv)
{
    pthread_t tid_send, tid_rcv, tid_info;
    int *status = malloc(sizeof(int*));
    cl_head = malloc(sizeof(CLIENT_LIST_STRUCT));
    cl_tail = malloc(sizeof(CLIENT_LIST_STRUCT));

    cl_head->next = tail;
    cl_head->prev = NULL;

    cl_tail->prev = cl_head;
    cl_tail->next = NULL;

    pthread_create(&tid_info, NULL, msg_info, NULL);
    sleep(1);
    pthread_create(&tid_send, NULL, msg_send, NULL);
    pthread_create(&tid_rcv, NULL, msg_rcv, NULL);

    pthread_join(tid_info, (void**) &status);
    if(!status){
        perror("pthread error");
        exit(-1);
    }
    pthread_join(tid_send, (void**) &status);
    if(!status){

        perror("pthread error");
        exit(-1);
    }
    pthread_join(tid_rcv, (void**) &status);
    if(!status){

        perror("pthread error");
        exit(-1);
    }
    return 0;
}
