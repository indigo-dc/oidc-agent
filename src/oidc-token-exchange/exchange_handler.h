#ifndef EXCHANGE_HANDLER_H
#define EXCHANGE_HANDLER_H

#include "oidc-token-exchange_options.h"

void handleTokenExchange(struct arguments* arguments);
void handleRemove(struct arguments* arguments);
void handleTokenRequest(struct arguments* arguments);

#endif  // EXCHANGE_HANDLER_H
