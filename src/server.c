#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>

#include "../include/common.h"


void* info(void*){
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE + 1];
    int must_stop = 0;
    ssize_t bytes_read;
    CLIENT_LIST_STRUCT *rcv_msg, *current;
    rcv_msg = malloc(sizeof(CLIENT_LIST_STRUCT));

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 1024;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(QUEUE_INFO_NAME, O_CREAT | O_RDWR, 0666, &attr);
    CHECK((mqd_t)-1 != mq);

    do {
        /* receive the message */
        bytes_read = mq_receive(mq, &rcv_msg, sizeof(CLIENT_LIST_STRUCT), NULL);
        CHECK(bytes_read >= 0);
        switch(rcv_msg->msg_type){
            case MSG_CONNECT:/*TODO add mutex to sync*/
                rcv_msg->next = head->next;
                head->next->prev = rcv_msg;
                rcv_msg->prev = head;
                head-> next = rcv_msg;
		current = head->next;
		while(current != tail){
                    mq_send(mq, rcv_msg, sizeof(CLIENT_LIST_STRUCT), current->prio);
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
		   }
		   current = current->next;
		}
		current = head->next;
		while(current != tail){
		    mq_send(mq, rcv_msg, sizeof(CLIENT_LIST_STRUCT), current->prio);
		    current = current->next;
		}
                break;
        }
    } while (1);/*CHANGE to CORRECT EXIT*/

    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(mq));
    CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));

}

void* msgs_send(void*){
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE + 1];
    int must_stop = 0;
    CLIENT_LIST_STRUCT *rcv_msg, *current;

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 1024;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(QUEUE_MSGS_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    CHECK((mqd_t)-1 != mq);

    do {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        if (!strncmp(buffer, MSG_STOP, strlen(MSG_STOP))){
            must_stop = 1;
        }
        else{
            
        }
    } while (!must_stop);

    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(mq));
    CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));
}

int main(int argc, char **argv)
{
    head = malloc(sizeof(CLIENT_LIST_STRUCT));
    tail = malloc(sizeof(CLIENT_LIST_STRUCT));

    head->next = tail;
    head->prev = NULL;

    tail->prev = prev;
    tail->next = NULL;

    return 0;
}
