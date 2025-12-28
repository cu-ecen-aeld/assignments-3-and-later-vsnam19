# Assignment 4 - Buildroot Setup Summary

## Completed Steps

### 1. Added Buildroot Submodule ✓
- Added buildroot as a git submodule using branch 2024.02.x
- Source: https://gitlab.com/buildroot.org/buildroot/
- Committed to repository

### 2. Created base_external Structure ✓
Created the following files in base_external/:
- `Config.in` - References the aesd-assignments package
- `external.mk` - Includes all package makefiles
- `external.desc` - Defines external name as "project_base"
- `package/aesd-assignments/Config.in` - Package configuration
- `package/aesd-assignments/aesd-assignments.mk` - Package makefile

### 3. Package Configuration ✓
Updated aesd-assignments.mk with:
- Repository: git@github.com:cu-ecen-aeld/assignments-3-and-later-vsnam19.git
- Commit: 7c3eaedf15fc15cb57da726166e186e7e984c381
- Installed to /usr/bin: finder.sh, finder-test.sh, writer
- Installed to /etc/finder-app/conf/: configuration files

### 4. Modified finder-test.sh ✓
Changes made:
- Updated to use config files from /etc/finder-app/conf/
- Calls writer and finder.sh without ./ prefix (uses PATH)
- Writes output to /tmp/assignment4-result.txt

### 5. Created Build Scripts ✓
- `build.sh` - Builds the buildroot image
- `save-config.sh` - Saves configuration to defconfig
- `runqemu.sh` - Runs QEMU with the generated image
- `clean.sh` - Runs make distclean in buildroot
- `shared.sh` - Shared configuration variables

### 6. Buildroot Configuration ✓
Enabled packages in base_external/configs/aesd_qemu_defconfig:
- BR2_PACKAGE_AESD_ASSIGNMENTS=y
- BR2_PACKAGE_DROPBEAR=y (for SSH support)
- BR2_PACKAGE_LIBTOMCRYPT=y (dropbear dependency)
- BR2_TARGET_GENERIC_ROOT_PASSWD="root"

### 7. Created Directory Structure ✓
- Created assignments/assignment4/ for assignment4-result.txt

## Current Status

The buildroot build is currently running. This will take several hours to complete.

## Next Steps (After Build Completes)

1. Run `./runqemu.sh` to start the QEMU instance
2. Test SSH access: `ssh -p 10022 root@localhost` (password: root)
3. Login to QEMU and run `finder-test.sh` without path
4. Verify /tmp/assignment4-result.txt is created
5. Use `scp -P 10022 root@localhost:/tmp/assignment4-result.txt assignments/assignment4/`
6. Run `./full-test.sh` to verify everything works (requires sshpass)
7. Tag the repository with "assignment-4-complete"

## Repository Structure

```
.
├── buildroot/                 # Buildroot submodule
├── base_external/
│   ├── Config.in
│   ├── external.mk
│   ├── external.desc
│   ├── configs/
│   │   └── aesd_qemu_defconfig
│   └── package/
│       └── aesd-assignments/
│           ├── Config.in
│           └── aesd-assignments.mk
├── build.sh
├── save-config.sh
├── runqemu.sh
├── clean.sh
├── shared.sh
├── finder-app/
│   ├── finder.sh
│   ├── finder-test.sh
│   └── writer.c
├── conf/
│   ├── username.txt
│   └── assignment.txt
└── assignments/
    └── assignment4/

```

## Build Optimizations (Optional)

To speed up future builds, add these to buildroot menuconfig:
- BR2_DL_DIR=${HOME}/.dl (package download cache)
- Enable Compiler Cache in build options

## Validation Checklist

- [ ] Can clone repository and run ./build.sh twice successfully
- [ ] Can run ./clean.sh then ./build.sh twice
- [ ] Can run ./runqemu.sh to start QEMU
- [ ] Can login to QEMU and run finder-test.sh from any directory
- [ ] writer executable is cross-compiled for target
- [ ] /var/log/messages contains syslog messages from writer
- [ ] Can SSH to port 10022 with user root and password root
- [ ] Can use scp to transfer files to/from QEMU
- [ ] ./full-test.sh completes successfully

## Important Notes

- The aesd-assignments package uses the git site method
- SSH URL is used (not HTTPS) for compatibility with automated testing
- All scripts (finder.sh, finder-test.sh, writer) are in PATH (/usr/bin)
- Config files are at /etc/finder-app/conf/
- QEMU SSH port forwarding: host:10022 -> guest:22
