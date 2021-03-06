#ifndef COMMON_H_
#define COMMON_H_

#define MAX_SIZE       1024
#define MSG_STOP       "exit"
#define MSG_CONNECT    10
#define MSG_DISCONNECT 11


#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

typedef struct cls {
    long prio;
    int msg_type;
    char name[20];

    struct cls* next;
    struct cls* prev;
} CLIENT_LIST_STRUCT;
CLIENT_LIST_STRUCT *head, *tail;

typedef struct msgbuf{
    long mtype;
    char sender_name[20];
    char mtext[MAX_SIZE];
} MESSAGE_BUFFER;


#endif
