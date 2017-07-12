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

#include "../include/common.h"


void* info(void* a){
    key_t key;
    int msgid;
    ssize_t bytes_read;
    key = ftok("server", 'D');
    CLIENT_LIST_STRUCT *rcv_msg, *current, *temp;
    rcv_msg = malloc(sizeof(CLIENT_LIST_STRUCT));
    temp = malloc(sizeof(CLIENT_LIST_STRUCT));

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
      perror("msgget failed with error");
      exit(EXIT_FAILURE);
    }

    do {
        /* receive the message */
        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(CLIENT_LIST_STRUCT), 0, 0);
        printf("%s connected with prio: %ld\n", rcv_msg->name, rcv_msg->prio);
        CHECK(bytes_read >= 0);
        switch(rcv_msg->msg_type){
            case MSG_CONNECT:/*TODO add mutex to sync*/
                
                rcv_msg->next = head->next;
                head->next->prev = rcv_msg;
                rcv_msg->prev = head;
                head->next = rcv_msg;
		        current = head->next;

                temp->msg_type = rcv_msg->msg_type;
                strncmp(temp->name, rcv_msg->name, 20);

                while(current != tail){
                    temp->prio = current->prio;
                    printf("send new connection to %ld\n", temp->prio);
                    msgsnd(msgid, (void*) temp, sizeof(CLIENT_LIST_STRUCT), 0);
                    current = current->next;
                }
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
        		current = head->next;
        		while(current != tail){
        		    msgsnd(msgid, (void*)rcv_msg, sizeof(CLIENT_LIST_STRUCT), 0);
        		    current = current->next;
        		}
                break;
        }
    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup */
    msgctl(msgid, IPC_RMID, 0);
}

void* msgs_send(void* a){
    key_t key;
    int msgid;
    ssize_t bytes_read;
    key = ftok("server", 'B');
    CLIENT_LIST_STRUCT *current;
    MESSAGE_BUFFER *rcv_msg;

    rcv_msg = malloc(sizeof(MESSAGE_BUFFER));

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
      perror("msgget failed with error");
      exit(EXIT_FAILURE);
    }


    do {
        /* receive the message */
        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(MESSAGE_BUFFER), 1L, 0);
        printf("rcv mcg: %s\n", rcv_msg->mtext);

        CHECK(bytes_read >= 0);
        current = head->next;
        while(current != tail){
            printf("send to: %ld\n", current->prio);
            rcv_msg->mtype = current->prio;
            msgsnd(msgid, (void*)rcv_msg, sizeof(MESSAGE_BUFFER), 0);
            current = current->next;
        }
    } while (1);

    msgctl(msgid, IPC_RMID, 0);
}

int main(int argc, char **argv)
{
    pthread_t tid_send, tid_info;
    int *status = malloc(sizeof(int*));
    head = malloc(sizeof(CLIENT_LIST_STRUCT));
    tail = malloc(sizeof(CLIENT_LIST_STRUCT));

    head->next = tail;
    head->prev = NULL;

    tail->prev = head;
    tail->next = NULL;
    pthread_create(&tid_info, NULL, info, NULL);
    pthread_create(&tid_send, NULL, msgs_send, NULL);

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

    return 0;
}
