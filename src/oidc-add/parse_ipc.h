#ifndef PARSE_IPC_ADD_H
#define PARSE_IPC_ADD_H

void          add_parseResponse(char* res);
void          add_parseLoadedAccountsResponse(char* res);
unsigned char add_checkLoadedAccountsResponseForAccount(char*       res,
                                                        const char* account);

#endif  // PARSE_IPC_ADD_H
