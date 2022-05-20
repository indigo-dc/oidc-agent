#include "device.h"

#include <time.h>
#include <unistd.h>
#include <utils/agentLogger.h>

#include "defines/ipc_values.h"
#include "defines/oidc_values.h"
#include "ipc/cryptCommunicator.h"
#include "ipc/pipe.h"
#include "oidc-agent/oidc/device_code.h"
#include "utils/json.h"
#include "utils/printer.h"
#include "utils/string/stringUtils.h"

char* _pollDeviceCode(const char* json_device, size_t interval,
                      time_t expires_at, const unsigned char only_at,
                      const unsigned char remote, struct ipcPipe* pipes) {
  while (expires_at ? expires_at > time(NULL) : 1) {
    sleep(interval);
    char* res = pipes ? ipc_communicateThroughPipe(*pipes, REQUEST_DEVICE,
                                                   json_device, only_at)
                      : ipc_cryptCommunicate(remote, REQUEST_DEVICE,
                                             json_device, only_at);
    if (NULL == res) {
      return NULL;
    }
    INIT_KEY_VALUE(IPC_KEY_STATUS, OIDC_KEY_ERROR, IPC_KEY_CONFIG,
                   OIDC_KEY_ACCESSTOKEN);
    if (CALL_GETJSONVALUES(res) < 0) {
      printError("Could not decode json: %s\n", res);
      printError("This seems to be a bug. Please hand in a bug report.\n");
      SEC_FREE_KEY_VALUES();
      secFree(res);
      return NULL;
    }
    secFree(res);
    KEY_VALUE_VARS(status, error, config, at);
    if (_error) {
      if (strequal(_error, OIDC_SLOW_DOWN)) {
        interval++;
        SEC_FREE_KEY_VALUES();
        continue;
      }
      if (strequal(_error, OIDC_AUTHORIZATION_PENDING)) {
        SEC_FREE_KEY_VALUES();
        continue;
      }
      oidc_seterror(_error);
      oidc_errno = OIDC_EERROR;
      SEC_FREE_KEY_VALUES();
      return NULL;
    }
    secFree(_status);
    if (only_at) {
      secFree(_config);
    } else {
      secFree(_at);
    }
    return only_at ? _at : _config;
  }
  oidc_seterror("Device code is not valid any more!");
  oidc_errno = OIDC_EERROR;
  return NULL;
}

char* pollDeviceCode(const char* json_device, size_t interval,
                     time_t expires_at, const unsigned char remote,
                     const unsigned char only_at) {
  return _pollDeviceCode(json_device, interval, expires_at, only_at, remote,
                         NULL);
}

char* agent_pollDeviceCode(const char* json_device, size_t interval,
                           time_t expires_at, const unsigned char only_at,
                           struct ipcPipe* pipes) {
  return _pollDeviceCode(json_device, interval, expires_at ?: time(NULL) + 300,
                         only_at, 0, pipes);
}
