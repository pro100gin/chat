#ifndef COMMON_H_
#define COMMON_H_

#define QUEUE_INFO_NAME  "/info_queue"
#define QUEUE_MSGS_NAME  "/msgs_queue"
#define MAX_SIZE    1024
#define MSG_STOP    "exit"
#define MSG_CONNECT = 10
#define MSG_DISCONNECT = 11


#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

typedef struct cls {
	int msg_type;
	long prio;
	char name[20];

        struct cls* next;
        struct cls* prev;
} CLIENT_LIST_STRUCT;
CLIENT_LIST_STRUCT *head, *tail;



#endif
