# Internal Details

## inject
An injector used to inject `libadbex_init.so` into `init` process using `ptrace`.

## adbex_init
Hook `execve` to inject `libadbex_adbd.so` into `adbd` process using `LD_PRELOAD`.

## adbex_adbd
Magic happens here.

### ADB Root
Fake current os as userdebug.

#### Fake properties
- ro.secure to 0
- ro.debuggable to 1
- service.adb.root to 1
- ro.boot.verifiedbootstate to orange

#### Fixup SELinux Policies
Add missing [SELinux policies](https://android.googlesource.com/platform/system/sepolicy/+/refs/heads/main/private/adbd.te) in production build.
```
userdebug_or_eng(`
  allow adbd self:process setcurrent;
  allow adbd su:process dyntransition;
')
```

I don't want to fix it bit by bit, so simply set `su` and `adbd` to permissive:
```
permissive { su }
permissive { adbd }
```

Fix init to adbd transition:
```
allow init adbd process noatsecure
allow init adbd process2 nosuid_transition
```

Fix `adb install` failed/hang:
```
allow system_server su fd use
allow system_server su unix_stream_socket { connectto getattr getopt ioctl read write shutdown }
allow system_server su binder call
```

Fix app debug:
```
allow untrusted_app su unix_stream_socket { connectto getattr getopt ioctl read write shutdown }
```

### Custom Shell
Hook `execle` to replace shell with whatever we want.

The shell path is read from property `persist.sys.adb.shell`, similar to LineageOS.
