//
// Created by 15461 on 2023/1/19.
//

#ifndef INC_2022_CPL_CODING_RAMFS_H
#define INC_2022_CPL_CODING_RAMFS_H
#include <stdint.h>

#define O_APPEND 02000
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef intptr_t ssize_t;
typedef uintptr_t size_t;
typedef long off_t;

int ropen(const char *pathname, int flags);//finished
int rclose(int fd);//finished
ssize_t rwrite(int fd, const void *buf, size_t count);
ssize_t rread(int fd, void *buf, size_t count);
off_t rseek(int fd, off_t offset, int whence);
int rmkdir(const char *pathname);//finished
int rrmdir(const char *pathname);//finished
int runlink(const char *pathname);
void init_ramfs();
#endif //INC_2022_CPL_CODING_RAMFS_H
