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
#include <sys/shm.h>

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
    key_t key, key_srv;
    int msgid, msgid_srv;
    char *name = (char*) arg;
    char buffer[MAX_SIZE + 1];
    ssize_t bytes_read;
    long pid = getpid();
    key = ftok("server", 'B');
    key_srv = ftok("server", 'D');
    MESSAGE_BUFFER *snd_msg;
    CLIENT_LIST_STRUCT *snd_srv_msg;
    snd_srv_msg = malloc(sizeof(CLIENT_LIST_STRUCT));
    snd_msg = malloc(sizeof(MESSAGE_BUFFER));
    strncpy(snd_msg->sender_name, name, 20);
    msgid = msgget(key, 0);
    if (msgid == -1) {
      perror("msgget failed with error1");
      exit(EXIT_FAILURE);
    }
    msgid_srv = msgget(key_srv, 0);

    if (msgid_srv == -1) {
      perror("msgget failed with error1");
      exit(EXIT_FAILURE);
    }

    while(1){
        fgets(buffer, MAX_SIZE, stdin);
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }

        if(!strcmp(buffer, "exit")){
            snd_srv_msg->prio = pid*10;
            snd_srv_msg->msg_type = MSG_DISCONNECT;
            strncpy(snd_srv_msg->name, name, 20);
            msgsnd(msgid_srv, (void*)snd_srv_msg, sizeof(CLIENT_LIST_STRUCT), 0);
            exit(0);
	}
        else{
            snd_msg->mtype = 1L;
            strncpy(snd_msg->mtext, buffer, MAX_SIZE);
            msgsnd(msgid, (void*)snd_msg, sizeof(MESSAGE_BUFFER), 0);
        }
    }

    msgctl(msgid, IPC_RMID, 0);
}

void* msg_info(void* arg){
    key_t key, key_mem;
    int msgid, i;
    ssize_t bytes_read;
    long pid;
    int shmid;
    char *shm, *start;
    char* name = (char*) arg;
    key = ftok("server", 'D');
    key_mem = ftok("client", 'A');
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

    if ((shmid = shmget(key_mem, 4096, 0)) < 0) {
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

        bytes_read = msgrcv(msgid, (void *)rcv_msg, sizeof(CLIENT_LIST_STRUCT), pid, 0);
        CHECK(bytes_read >= 0);

        current = (CLIENT_LIST_STRUCT*)shm;

        printf("------------------------------\nlist of users:\n");
        for(i = 0; i < rcv_msg->msg_type; ++i){
            printf("%s\n", current->name);
            current++;
        }
        printf("------------------------------\n");

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
