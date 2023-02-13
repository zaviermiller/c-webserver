#pragma once

int dir_exists(char *root_dir);
char *get_file_contents(char* path);
char *get_extension(char *path);
int is_dir(char *path);
char *readall(int fd);
char *to_lower(char *path);
