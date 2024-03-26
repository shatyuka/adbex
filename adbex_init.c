#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <plthook.h>

#define LOG_TAG "[adbex][init]"

#define ADBD_PATH_APEX "/apex/com.android.adbd/bin/adbd"
#define ADBD_PATH "/system/bin/adbd"

char adbex_adbd[PATH_MAX];

int (*orig_execv)(const char* path, char* const argv[]);
int hook_execv(const char* path, char* const argv[]) {
  if (unlikely(!strcmp(path, ADBD_PATH_APEX) || !strcmp(path, ADBD_PATH))) {
    klog(LOG_TAG, "adbd pid: %d\n", getpid());
    setenv("LD_PRELOAD", adbex_adbd, 1);
    return orig_execv(path, argv);
  }
  return orig_execv(path, argv);
}

CONSTRUCTOR UNUSED void adbex_init_main() {
  klog(LOG_TAG, "injected into init");

  const char* adbex_path;
  if (access("/sbin/adbex", F_OK) == 0) {
    adbex_path = "/sbin/adbex";
  } else {
    adbex_path = "/debug_ramdisk/adbex";
  }
  snprintf(adbex_adbd, sizeof(adbex_adbd), "%s/libadbex_adbd.so", adbex_path);
  klog(LOG_TAG, "adbex_adbd: %s", adbex_adbd);

  plthook_t* plthook;
  int err = plthook_open_by_address(&plthook, (void*)get_module_base(1, "/init\n"));
  klog(LOG_TAG, "plthook err: %d\n", err);
  if (err) return;
  plthook_replace(plthook, "execv", (void*)&hook_execv, (void**)&orig_execv);
}
