#ifndef NO_STATLOG
#include "statid.h"

#include "defines/msys.h"
#include "oidc-agent/http/http_ipc.h"
#include "utils/config/agent_config.h"
#include "utils/crypt/crypt.h"
#include "utils/file_io/file_io.h"
#include "utils/file_io/oidc_file_io.h"
#include "utils/memory.h"
#include "utils/string/stringUtils.h"
#include "utils/system_runner.h"
#include "utils/tempenv.h"

#ifndef ANY_MSYS
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static char* tmpBootIdFilePath() {
  const char* tmpdir = get_tmp_env();
  if (!strValid(tmpdir)) {
    tmpdir = "/tmp";
  }
  char* filename =
#ifdef ANY_MSYS
      oidc_strcopy("oidc-agent-boot_id");
#else
      oidc_sprintf("oidc-agent-boot_id_%d", getuid());
#endif

  char* path = oidc_pathcat(tmpdir, filename);
  secFree(filename);
  return path;
}

static char* getBootID() {
  char* b = readFile("/proc/sys/kernel/random/boot_id");
  if (strValid(b)) {
    return b;
  }
  char* tmpBPath = tmpBootIdFilePath();
  secFree(b);
  b = readFile(tmpBPath);
  if (strValid(b)) {
    secFree(tmpBPath);
    return b;
  }
  secFree(b);
  b = randomString(24);
  writeFile(tmpBPath, b);
  secFree(tmpBPath);
  return b;
}

static char* getMachineID() {
  char* m = readFile("/etc/machine-id");
  if (strValid(m)) {
    return m;
  }
  const char* const ownIDFileName = "machine_id.config";
  secFree(m);
  m = readOidcFile(ownIDFileName);
  if (strValid(m)) {
    return m;
  }
  secFree(m);
  m = randomString(24);
  writeOidcFile(ownIDFileName, m);
  return m;
}

static char* getOSInfo() {
#ifndef ANY_MSYS
  return getOutputFromCommand("uname -sorvm");
#else
  return oidc_strcopy("windows");
#endif
}

static time_t location_updated = 0;

static char* getLocation() {
  char* code =
      httpsGET("http://ip-api.com/line/?fields=countryCode", NULL, NULL);
  if (code) {
    location_updated = time(NULL);
    if (lastChar(code) == '\n') {
      lastChar(code) = '\0';
    }
  }
  return code;
}

static struct statid* id = NULL;

#define ONE_DAY 86400

static void upd_location() {
  if (id == NULL) {
    return;
  }
  time_t now = time(NULL);
  if (now < location_updated + ONE_DAY) {
    return;
  }
  char* code = getLocation();
  if (code) {
    secFree(id->location);
    id->location = code;
  }
}

struct statid getStatID() {
  if (id != NULL) {
    upd_location();
    return *id;
  }
  char* b = getBootID();
  char* m = getMachineID();
  if (lastChar(b) == '\n') {
    lastChar(b) = '\0';
  }
  if (lastChar(m) == '\n') {
    lastChar(m) = '\0';
  }
  id  = secAlloc(sizeof(struct statid));
  *id = (struct statid){
#ifdef ANY_MSYS
      .machine_id = m,
      .boot_id    = b,
#else
      .machine_id = oidc_sprintf("%s_%d", m, getuid()),
      .boot_id    = oidc_sprintf("%s_%d", b, getuid()),
#endif
      .os_info = getOSInfo(),
  };
  if (getAgentConfig()->stats_collect_location) {
    id->location = getLocation();
  }
#ifndef ANY_MSYS
  secFree(b);
  secFree(m);
#endif
  return *id;
}

#endif  // NO_STATLOG