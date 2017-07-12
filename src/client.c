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
    printf("pid = %ld\n", pid);
    do {
        /* receive the message */
        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(MESSAGE_BUFFER), pid, 0);
        CHECK(bytes_read >= 0);
        printf("\n%s: %s\n",rcv_msg->sender_name ,rcv_msg->mtext);
    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup */
    msgctl(msgid, IPC_RMID, 0);
}

void* msg_send(void* arg){
    key_t key;
    int msgid;
    char *name = (char*) arg;
    char buffer[MAX_SIZE + 1];
    ssize_t bytes_read;
    key = ftok("server", 'B');
    MESSAGE_BUFFER *snd_msg;
    snd_msg = malloc(sizeof(MESSAGE_BUFFER));
    strncpy(snd_msg->sender_name, name, 20);
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
        snd_msg->mtype = 1L;
        strncpy(snd_msg->mtext, buffer, MAX_SIZE);
        msgsnd(msgid, (void*)snd_msg, sizeof(MESSAGE_BUFFER), 0);
    }

    msgctl(msgid, IPC_RMID, 0);
}

void* msg_info(void* arg){
    key_t key;
    int msgid;
    ssize_t bytes_read;
    long pid;
    char* name = (char*) arg;
    key = ftok("server", 'D');
    CLIENT_LIST_STRUCT *rcv_msg, *current;
    current = malloc(sizeof(CLIENT_LIST_STRUCT));

    msgid = msgget(key, 0);
    if (msgid == -1) {
      perror("msgget failed with error2");
      exit(EXIT_FAILURE);
    }
    pid = getpid();
    current->prio = pid;
    current->msg_type = MSG_CONNECT;

    strncpy(current->name, name, 20);

    msgsnd(msgid, (void*)current, sizeof(CLIENT_LIST_STRUCT), 0);

    /*
        add msg_receive to arrey with names
    */
    if ((shmid = shmget(key, 4096, 0)) < 0) {
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
        rcv_msg = maxlloc(sizeof(CLIENT_LIST_STRUCT));
	bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(CLIENT_LIST_STRUCT), pid, 0);
        CHECK(bytes_read >= 0);
        current = (CLIENT_LIST_STRUCT*) shm;
        
        /*switch(rcv_msg->msg_type){
            case MSG_CONNECT:/*TODO add mutex to sync
                rcv_msg->next = cl_head->next;
                cl_head->next->prev = rcv_msg;
                rcv_msg->prev = cl_head;
                cl_head->next = rcv_msg;
                break;
            case MSG_DISCONNECT:/*TODO add mutex to sync
                current = cl_head->next;
                while(current != cl_tail){
                    if(current->prio == rcv_msg->prio){
                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                        free(current);
                        break;
                    }
                    current = current->next;
                }
                break;
        }*/
    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup */
    msgctl(msgid, IPC_RMID, 0);
}

int main(int argc, char **argv)
{
    pthread_t tid_send, tid_rcv, tid_info;
    char* name;
    int *status = malloc(sizeof(int*));
    name = malloc(20*sizeof(char));
    cl_head = malloc(sizeof(CLIENT_LIST_STRUCT));
    cl_tail = malloc(sizeof(CLIENT_LIST_STRUCT));

    cl_head->next = cl_tail;
    cl_head->prev = NULL;

    cl_tail->prev = cl_head;
    cl_tail->next = NULL;

    fgets(name, 20, stdin);
    if (name[strlen(name) - 1] == '\n') {
            name[strlen(name) - 1] = '\0';
    }

    pthread_create(&tid_info, NULL, msg_info, (void*) name);
    pthread_create(&tid_send, NULL, msg_send,(void*) name);
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
