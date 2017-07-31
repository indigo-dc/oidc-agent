#ifndef IPC_H
#define IPC_H

int ipc_init(int isServer);
int ipc_bind(void(callback)());
int ipc_connect();
char* ipc_read(int _sock);
int ipc_write(int _sock, char* msg);
int ipc_close();

#endif // IPC_H
