#pragma once

#define MOUNT_POINT "/sdcard"

void init_tf(void);
void list_dir(const char *path);
void make_dir(const char *path);
void write_file(const char *path, const char *data);
void unmount_tf(void);
