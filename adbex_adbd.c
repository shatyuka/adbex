#include "utils.h"
#include "prop_info.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <paths.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/system_properties.h>

#define LOG_TAG "[adbex][adbd]"

EXPORT int __android_log_is_debuggable() { return 1; }

// For Android 9+
typedef void (*callback_t)(void* cookie, const char* name, const char* value, uint32_t serial);
typedef void (*__system_property_read_callback_t)(const prop_info* pi, callback_t callback, void* cookie);
__system_property_read_callback_t orig___system_property_read_callback;
EXPORT void __system_property_read_callback(const prop_info* pi, callback_t callback, void* cookie) {
  if (unlikely(!strcmp(pi->name, "ro.secure"))) {
    klog(LOG_TAG, "__system_property_read: replaced ro.secure");
    callback(cookie, pi->name, "0", pi->serial);
  } else if (unlikely(!strcmp(pi->name, "service.adb.root"))) {
    klog(LOG_TAG, "__system_property_read_callback: replaced service.adb.root");
    callback(cookie, pi->name, "1", pi->serial);
  } else if (unlikely(!strcmp(pi->name, "ro.boot.verifiedbootstate"))) {
    klog(LOG_TAG, "__system_property_read_callback: replaced ro.boot.verifiedbootstate");
    callback(cookie, pi->name, "orange", pi->serial);
  } else if (likely(orig___system_property_read_callback)) {
    orig___system_property_read_callback(pi, callback, cookie);
  }
}

// For Android 8+
typedef int (*__system_property_read_t)(const prop_info* pi, char* name, char* value);
__system_property_read_t orig___system_property_read;
EXPORT int __system_property_read(const prop_info* pi, char* name, char* value) {
  if (unlikely(!strcmp(pi->name, "ro.secure"))) {
    klog(LOG_TAG, "__system_property_read: replaced ro.secure");
    if (name) strlcpy(name, pi->name, PROP_NAME_MAX);
    strcpy(value, "0");
    return (int)strlen("0");
  } else if (unlikely(!strcmp(pi->name, "service.adb.root"))) {
    klog(LOG_TAG, "__system_property_read: replaced service.adb.root");
    if (name) strlcpy(name, pi->name, PROP_NAME_MAX);
    strcpy(value, "1");
    return (int)strlen("1");
  } else if (unlikely(!strcmp(pi->name, "ro.boot.verifiedbootstate"))) {
    klog(LOG_TAG, "__system_property_read: replaced ro.boot.verifiedbootstate");
    if (name) strlcpy(name, pi->name, PROP_NAME_MAX);
    strcpy(value, "orange");
    return (int)strlen("orange");
  } else if (likely(orig___system_property_read)) {
    return orig___system_property_read(pi, name, value);
  }
  return 0;
}

// For Android 7+
typedef int (*__system_property_get_t)(const char* name, char* value);
__system_property_get_t orig___system_property_get;
EXPORT int __system_property_get(const char* name, char* value) {
  if (unlikely(!strcmp(name, "ro.secure"))) {
    klog(LOG_TAG, "__system_property_get: replaced ro.secure");
    strcpy(value, "0");
    return (int)strlen("0");
  } else if (unlikely(!strcmp(name, "service.adb.root"))) {
    klog(LOG_TAG, "__system_property_get: replaced service.adb.root");
    strcpy(value, "1");
    return (int)strlen("1");
  } else if (unlikely(!strcmp(name, "ro.boot.verifiedbootstate"))) {
    klog(LOG_TAG, "__system_property_get: replaced ro.boot.verifiedbootstate");
    strcpy(value, "orange");
    return (int)strlen("orange");
  } else if (likely(orig___system_property_get)) {
    return orig___system_property_get(name, value);
  }
  return 0;
}

typedef int (*execle_t)(const char* path, const char* arg0, ...);
execle_t orig_execle;
EXPORT int execle(UNUSED const char* path, const char* arg0, ...) {
  va_list ap;
  va_start(ap, arg0);
  int argc = 1;
  const char *arg, *tmp;
  while ((tmp = va_arg(ap, char*))) {
    arg = tmp;
    argc++;
  }
  char** envp = va_arg(ap, char**);
  va_end(ap);

  const char* sh_path = _PATH_BSHELL;
  char shell[PROP_VALUE_MAX];
  shell[0] = 0;
  __system_property_get("persist.sys.adb.shell", shell);
  if (likely(shell[0] && access(shell, X_OK) == 0)) {
    sh_path = shell;
  }

  if (unlikely(!orig_execle)) {
    errno = EINVAL;
    return -1;
  }

  int ret = -1;
  if (likely(argc == 1 || argc == 2)) {
    ret = orig_execle(sh_path, sh_path, "-", NULL, envp);
  } else if (argc == 3) {
    ret = orig_execle(sh_path, sh_path, "-c", arg, NULL, envp);
  } else {
    errno = EINVAL;
  }

  return ret;
}

CONSTRUCTOR UNUSED void adbex_adbd_main() {
  klog(LOG_TAG, "injected into adbd");
  unsetenv("LD_PRELOAD");

  void* libc = dlopen("libc.so", RTLD_NOW);
  if (libc) {
    orig___system_property_read_callback =
        (__system_property_read_callback_t)dlsym(libc, "__system_property_read_callback");
    orig___system_property_read = (__system_property_read_t)dlsym(libc, "__system_property_read");
    orig___system_property_get = (__system_property_get_t)dlsym(libc, "__system_property_get");
    orig_execle = (execle_t)dlsym(libc, "execle");
  }
  klog(LOG_TAG, "__system_property_read_callback: %p", orig___system_property_read_callback);
  klog(LOG_TAG, "__system_property_read: %p", orig___system_property_read);
  klog(LOG_TAG, "__system_property_get: %p", orig___system_property_get);
  klog(LOG_TAG, "execle: %p", orig_execle);
}
