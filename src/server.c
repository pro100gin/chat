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
#include <sys/shm.h>

#include "../include/common.h"


void* info(void* a){
    key_t key, key_mem;
    int msgid, count;
    ssize_t bytes_read;
    int shmid;
    char *shm, *start;
    count = 0;
    key = ftok("server", 'D');
    key_mem = ftok("client", 'A');
    CLIENT_LIST_STRUCT *rcv_msg, *current, *temp;
    temp = malloc(sizeof(CLIENT_LIST_STRUCT));

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
      perror("msgget error");
      exit(EXIT_FAILURE);
    }
    if ((shmid = shmget(key_mem, 4096, IPC_CREAT | 0666)) < 0) {
        perror("shmget error");
        exit(1);
    }
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat error");
        exit(1);
    }
    start = shm;
    do {
        /* receive the message */
	    rcv_msg = malloc(sizeof(CLIENT_LIST_STRUCT));
        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(CLIENT_LIST_STRUCT), 0L, 0);
        printf("%s connected with prio: %ld\n", rcv_msg->name, rcv_msg->prio);
        CHECK(bytes_read >= 0);
        switch(rcv_msg->msg_type){
            case MSG_CONNECT:/*TODO add mutex to sync*/
		        count++;
                rcv_msg->next = head->next;
                head->next->prev = rcv_msg;
                rcv_msg->prev = head;
                head->next = rcv_msg;


                break;
            case MSG_DISCONNECT:/*TODO add mutex to sync*/
        	    count--;
                current = head->next;
            	while(current != tail){
            	    if(current->prio == rcv_msg->prio){
                        current->prev->next = current->next;
            	        current->next->prev = current->prev;
            	        free(current);
                        break;
            	    }
            	}
                break;
        }
        shm = start;
        current = head->next;
        while(current!=tail){
            memcpy(shm, current, sizeof(CLIENT_LIST_STRUCT));
            shm+=sizeof(CLIENT_LIST_STRUCT);
            current=current->next;
        }
        temp->msg_type = rcv_msg->msg_type;
        strncpy(temp->name, rcv_msg->name, 20);
        current = head->next;
        while(current != tail){
            temp->prio = current->prio;
            temp->msg_type = count;
            printf("send new connection to %ld\n", temp->prio);
            msgsnd(msgid, (void*) temp, sizeof(CLIENT_LIST_STRUCT), 0);
            current = current->next;
        }
        usleep(0.001);
        

    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup add clean */
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
        printf("rcv mcg: %s from %s\n", rcv_msg->mtext, rcv_msg->sender_name);

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
