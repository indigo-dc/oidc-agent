#ifndef IPC_H
#define IPC_H

// modes
#define PROMPT 3
#define PROMPT_PASSWORD 2
#define PRINT 1
#define PRINT_AND_CLOSE 0
#define PROMPT_CHAR PROMPT + '0'
#define PROMPT_PASSWORD_CHAR PROMPT_PASSWORD + '0'
#define PRINT_CHAR PRINT + '0'
#define PRINT_AND_CLOSE_CHAR PRINT_AND_CLOSE + '0'


int ipc_init(int isServer);
int ipc_bind(void(callback)());
int ipc_connect();
char* ipc_read(int _sock);
int ipc_write(int _sock, char* msg);
int ipc_writeWithMode(int _sock, char* msg, int mode) ;
int ipc_close();

#endif // IPC_H
