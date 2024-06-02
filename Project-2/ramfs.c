//
// Created by 15461 on 2023/1/19.
//
#include "ramfs.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct node {
    enum {
        file,
        directory
    } type;
    char *name;
    int size;
    struct node *next_class;
    struct node *next_unit;
    void *content;
    int offset;
    int flag;
    int FD;
} node;

typedef struct opened {
    int F_D;
    node *opening;
} opened;


opened pointers[4096];
node *root;

int file_descriptor = 1;
//file_descriptor从1开始每用一个就加一
int number = 0;
//number用来标识被打开的文件或者文件夹pointerss[number]
void initial(node *new_directory) {
    new_directory->name = malloc(32 * sizeof(char));
    new_directory->next_unit = NULL;
    new_directory->size = 0;
    new_directory->next_class = NULL;
    new_directory->offset = 0;
    new_directory->flag = 0;
    new_directory->FD = -1;
    new_directory->content =NULL;
}
//在创建一个文件或者目录后的初始化操作
int bool_legal(const char* str){
    if((*str>=48&&*str<=57)||(*str>=65&&*str<=90)||(*str>=97&&*str<=122)||*str==46){
        return 0;
    } else return -1;
}
//判断路径名是否合法

int null_pointer(){
    for (int i = 0; i < 4096; ++i) {
        if(pointers[i].F_D<=0)return i;
    }
    return -1;
}
int ropen(const char *pathname, int flags) {
    if(strlen(pathname)>1024)return -1;     //路径的总长度不能超过1024
    node *present_class ;
    node *priority ;
    present_class = root;
    priority =root;
    int index = 0; //从pathname的第一个一直遍历
    char *tempt = malloc(100 * sizeof(char)); //用来储存没一层的目录或者文件
    char *legal = malloc(1);
    memset(tempt, 0, 100);
    if(*pathname!=47){
        free(tempt);
        free(legal);
        return -1;//如果一开始没有/就是违法的
    }
    while (1) {
        int i = 0;  //从tempt写入到tempt+i
        int judge = 0;//用来判断是不是开始向tempt里面写入目录名了
        int bool = 0; //判断现在读入的tempt是父目录还是最后要带开的目录或者文件
        while (1) {
            if (judge == 0) {
                if (*(pathname + index) != 47) {
                    judge = 1;
                    memcpy(tempt, (pathname + index),1);
                    i++;
                }
                if(*(pathname+index)==0){
                    free(tempt);
                    free(legal);
                    return -1;   //把从index开始的 “/”略过
                }
            } else {
                if (*(pathname + index) == 0) {
                    bool = 1;
                    break;
                } //读到路径名末尾了就说明是最后要打开的文件了
                else if (*(pathname + index) == 47 && *(pathname + index + 1) != 47 && *(pathname + index + 1) != 0) {
                    break;
                }// 发现上册目录        在读完上层目录后继续往后读，发现了除了“/” 和 0 之外的字符 说明还有子目录
                else if (*(pathname + index) != 47) {
                    memcpy(legal,pathname+index,1);
                    if(bool_legal(legal)==0)
                    {memcpy(tempt + i, pathname + index, 1);}
                    if(bool_legal(legal)==-1)return -1;
                    i++;
                }
            }
            index++;
        }
        int length= strlen(tempt);
        if(length>32){
            free(tempt);
            free(legal);
            return -1;//路径名小于32
        }
        if (bool == 0) {
            if(present_class==NULL){
                free(tempt);
                free(legal);
                return -1;  //在找父目录之前如果present class就是空的，说明错误
            }
            while (1) {
                if (strcmp(tempt, present_class->name) == 0) {
                    priority = present_class;
                    present_class = present_class->next_class;
                    break; //找到父目录
                } else if (present_class->next_unit == NULL) {
                    free(tempt);
                    return -1;
                }else {
                    priority = present_class;
                    present_class = present_class->next_unit;
                }
            }
        }
        //找父目录
        if (bool == 1) {
            if(present_class==NULL){
                if ((flags & 0100) != 0) {
                    if(priority->type==file)return -1;
                    node *new_file = malloc(sizeof(node));
                    initial(new_file);
                    strcpy(new_file->name, tempt);
                    new_file->FD = file_descriptor;
                    new_file->type = file;
                    number=null_pointer();
                    pointers[number].F_D = file_descriptor;
                    file_descriptor++;
                    new_file->flag = flags;
                    priority->next_class=new_file;
                    pointers[number].opening =new_file;
                    return file_descriptor - 1;
                } else {
                    free(tempt);
                    return -1;
                }
            }//如果presentclass 是空的，就检查有没有要create如果需要就创建
            while (1) {
                if (present_class == NULL) {
                    if ((flags & 0100) != 0) {
                        node *new_file = malloc(sizeof(node));
                        initial(new_file);
                        strcpy(new_file->name, tempt);
                        new_file->FD = file_descriptor;
                        new_file->type = file;
                        number=null_pointer();
                        pointers[number].F_D = file_descriptor;
                        file_descriptor++;
                        new_file->flag = flags;
                        priority->next_unit=new_file;
                        pointers[number].opening =new_file;
                        return file_descriptor - 1;
                    } else {
                        free(tempt);
                        return -1;
                    }
                } else if (strcmp(tempt, present_class->name) == 0) {

                    if ((flags & 02000) != 0) {
                        present_class->offset = present_class->size; //以append打开
                    } else{
                        present_class->offset=0; //非append情况offset归零
                    }
                    if ((flags & 0100) != 0 && ((flags&1)!=0||(flags&2)!=0)&&present_class->type==file) {
                        free(present_class->content);
                        present_class->size = 0;
                        present_class->offset = 0;
                    }    //trunc和读写打开后 清空
                    if ((flags&1)!=0&&(flags&2)!=0) {
                        present_class->flag = flags-2;
                    } else {
                        present_class->flag = flags;
                    }  //当只读时 前面两位时 00 但是只写时时 01 读写时是10 当读写和只写或运算得到11时是只写 那就将flags减二保证11变成01
                    present_class->FD = file_descriptor;
                    number=null_pointer();
                    pointers[number].F_D = file_descriptor;
                    pointers[number].opening = present_class;
                    file_descriptor++;
                    return file_descriptor - 1;
                } else if (strcmp(tempt, present_class->name) != 0) {
                    priority=present_class;
                    present_class = present_class->next_unit;
                }
            }
        }
        memset(tempt, 0, 100);
    }
}

int rclose(int fd) {
    for (int i = 0; i < 4096; ++i) {
        if (pointers[i].opening == NULL)return -1;   //结束循环
        if (pointers[i].F_D == fd&&fd!=0) {
            pointers[i].F_D = 0;
            return 0;
        }
    } //关闭文件时就是把对应的pointer.FD归0；这样可能有点问题但是应该不会WA
    return -1;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    for (int i = 0; i < 4096; ++i) {
        if (pointers[i].opening == NULL)return -1;
        if (pointers[i].F_D == fd&&fd!=0) {
            if (((pointers[i].opening->flag & 1) != 0 || (pointers[i].opening->flag & 2) != 0) &&  //这里时只要当前两位中有1位不是0就说明是可写的
                pointers[i].opening->type == file) {   //保证这是文件
                if (pointers[i].opening->offset > pointers[i].opening->size) {  //如果偏移量超过文件末尾
                    size_t expand = pointers[i].opening->offset + count - pointers[i].opening->size;  //扩大的文件量
                    if(pointers[i].opening->content==NULL){
                        pointers[i].opening->content= malloc(expand);  //第一次分配
                    } else{
                        pointers[i].opening->content = realloc(pointers[i].opening->content, expand+pointers[i].opening->size);//二次分配
                    }
                    memset(pointers[i].opening->content + pointers[i].opening->size, 0,
                           pointers[i].opening->offset - pointers[i].opening->size); //从文件末尾到偏移量中间填0
                    pointers[i].opening->size = pointers[i].opening->size + expand;
                } else if (pointers[i].opening->offset + count > pointers[i].opening->size&&pointers[i].opening->offset <= pointers[i].opening->size) { //如果offset<size但是仍然需要扩容
                    size_t expand = pointers[i].opening->offset + count - pointers[i].opening->size;
                    if(pointers[i].opening->content==NULL){
                        pointers[i].opening->content= malloc(expand);
                    } else{
                        pointers[i].opening->content = realloc(pointers[i].opening->content, expand+pointers[i].opening->size);
                    }
                    pointers[i].opening->size = pointers[i].opening->size + expand;
                }
                memcpy(pointers[i].opening->content + pointers[i].opening->offset, buf, count); //写入
                pointers[i].opening->offset = pointers[i].opening->offset + count; //偏移量后移
                return count;
            } else if (pointers[i].opening->FD == fd && (pointers[i].opening->type == directory ||((pointers[i].opening->flag & 1) == 0 &&(pointers[i].opening->flag & 2) == 0))) {
                return -1;
            }
        }
    }
}

ssize_t rread(int fd, void *buf, size_t count) {
    for (int i = 0; i <4096; ++i) {
        if (pointers[i].opening == NULL)return -1;
        if (pointers[i].F_D == fd&&fd!=0) {
            if ((pointers[i].opening->flag % 2) == 0 && pointers[i].opening->type == file) {   //可读的文件的第一位一定是0   00 10
                if(pointers[i].opening->offset  > pointers[i].opening->size)return 0;
                if (pointers[i].opening->offset + count > pointers[i].opening->size) {
                    size_t expand = pointers[i].opening->size - pointers[i].opening->offset;
                    memcpy(buf, pointers[i].opening->content + pointers[i].opening->offset, expand);
                    pointers[i].opening->offset = pointers[i].opening->offset + expand;
                    return expand;
                } else {
                    memcpy(buf, pointers[i].opening->content + pointers[i].opening->offset, count);
                    pointers[i].opening->offset = pointers[i].opening->offset + count;
                    return count;
                }
            } else if (pointers[i].opening->type == directory || pointers[i].opening->flag % 2 != 0) {
                return -1;
            }
        }
    }
}

off_t rseek(int fd, off_t offset, int whence) {
    for (int i = 0; i < 4096; ++i) {
        if (pointers[i].opening == NULL)return -1;
        if (pointers[i].F_D == fd&&fd!=0) {
            if (whence == SEEK_SET) {
                if (offset < 0)return -1;
                pointers[i].opening->offset = offset;
            } else if (whence == SEEK_CUR) {
                pointers[i].opening->offset = pointers[i].opening->offset + offset;
            } else if (whence == SEEK_END) {
                pointers[i].opening->offset = pointers[i].opening->size + offset;
            }
            return pointers[i].opening->offset;
        }
    }
}

int rmkdir(const char *pathname) {
    if(strlen(pathname)>1024)return -1;
    node *present_class;
    node *priority;
    present_class = root;
    priority =root;
    int index = 0;
    char *tempt = malloc(100 * sizeof(char));
    char *legal= malloc(3);
    memset(tempt, 0, 100);
    if(*pathname!=47){
        free(tempt);
        free(legal);
        return -1;
    }
    while (1) {
        int i = 0;
        int judge = 0;
        int bool_make = 0;
        while (1) {
            if (judge == 0) {
                if (*(pathname + index) != 47) {
                    judge = 1;
                    memcpy(tempt, (pathname + index),1);
                    i++;
                }
                if(*(pathname+index)==0)return -1;//把开头的许多//都给直接跳过去
            } else {
                if (*(pathname + index) == 0) {
                    bool_make = 1;
                    break;
                } //发现是文件了
                else if (*(pathname + index) == 47 && *(pathname + index + 1) != 47 && *(pathname + index + 1) != 0) {
                    break;
                }// 发现上册目录
                else if (*(pathname + index) != 47) {
                    memcpy(legal,pathname+index,1);
                    if(bool_legal(legal)==0)
                    {memcpy(tempt + i, pathname + index, 1);}
                    if(bool_legal(legal)==-1)return -1;
                    i++;
                }
            }
            index++;
        }
        int length= strlen(tempt);
        if(length>32)return -1;
        if (bool_make == 0) {
            if(present_class==NULL)return -1;
            while (1) {
                if (strcmp(tempt, present_class->name) == 0) {
                    priority = present_class;
                    present_class = present_class->next_class;
                    break;
                } else if ( present_class->next_unit == NULL) {
                    free(tempt);
                    return -1;
                }else {
                    priority = present_class;
                    present_class = present_class->next_unit;
                }
            }
        }
        //找父目录
        if (bool_make == 1) {
            if(priority->type==file)return -1;
            if(present_class==NULL){
                node *new_directory = malloc(sizeof(node));
                initial(new_directory);
                new_directory->type = directory;
                strcpy(new_directory->name, tempt);
                priority->next_class = new_directory;
                free(tempt);
                return 0;
            }
            while (1) {
                if (present_class == NULL) {
                    node *new_directory = malloc(sizeof(node));
                    initial(new_directory);
                    new_directory->type = directory;
                    strcpy(new_directory->name, tempt);
                    priority->next_unit = new_directory;
                    free(tempt);
                    return 0;
                } else if (strcmp(tempt, present_class->name) == 0) {
                    free(tempt);
                    return -1;
                } else {
                    priority = present_class;
                    present_class = present_class->next_unit;
                }
            }

        }
        memset(tempt, 0, 100);
    }
}

int rrmdir(const char *pathname) {
    if(strlen(pathname)>1024)return -1;
    node *present_class;
    node *priority ;
    present_class = root;
    priority =root;
    int index = 0;
    char *tempt = malloc(100 * sizeof(char));
    char *legal = malloc(3);
    memset(tempt, 0, 100);
    if(*pathname!=47){
        free(tempt);
        free(legal);
        return -1;
    }

    while (1) {
        int i = 0;
        int judge = 0;
        int bool_delete = 0;
        while (1) {
            if (judge == 0) {
                if (*(pathname + index) != 47) {
                    judge = 1;
                    memcpy(tempt, (pathname + index),1);
                    i++;
                }
                if(*(pathname+index)==0)return -1;//把开头的许多//都给直接跳过去
            } else {
                if (*(pathname + index) == 0) {
                    bool_delete = 1;
                    break;
                } //发现是文件了
                else if (*(pathname + index) == 47 && *(pathname + index + 1) != 47 && *(pathname + index + 1) != 0) {
                    break;
                }// 发现上册目录
                else if (*(pathname + index) != 47) {
                    memcpy(legal,pathname+index,1);
                    if(bool_legal(legal)==0)
                    {memcpy(tempt + i, pathname + index, 1);}
                    if(bool_legal(legal)==-1)return -1;
                    i++;
                }
            }
            index++;
        }
        int length= strlen(tempt);
        if(length>32)return -1;

        if (bool_delete == 0) {
            if(present_class==NULL)return -1;
            while (1) {
                if (strcmp(tempt, present_class->name) == 0) {
                    priority = present_class;
                    present_class = present_class->next_class;
                    break;
                } else if ( present_class->next_unit == NULL) {
                    free(tempt);
                    return -1;
                }else {
                    priority = present_class;
                    present_class = present_class->next_unit;
                }
            }
        }
        //找父目录
        if (bool_delete == 1) {
            if(present_class==NULL)return -1;
            if(strcmp(present_class->name,tempt)==0){
                if (present_class->type != directory||present_class->next_class!=NULL) {
                    free(tempt);
                    return -1;
                } else {
                    if (present_class->next_unit == NULL) {
                        priority->next_class = NULL;
                        free(present_class);
                    } else {
                        present_class = present_class->next_unit;
                        priority->next_class = present_class;
                    }
                    return 0;
                }
            }
            while (1) {
                if (strcmp(tempt, present_class->name) == 0) {
                    if (present_class->type != directory||present_class->next_class!=NULL) {
                        free(tempt);
                        return -1;
                    } else {
                        if (present_class->next_unit == NULL) {
                            priority->next_unit = NULL;
                        } else {
                            present_class = present_class->next_unit;
                            priority->next_unit = present_class;
                        }
                        return 0;
                    }
                } else if (strcmp(tempt, present_class->name) != 0 && present_class->next_unit != NULL) {
                    priority = present_class;
                    present_class = present_class->next_unit;
                } else if (strcmp(tempt, present_class->name) != 0 && present_class->next_unit == NULL) {
                    free(tempt);
                    return -1;
                }
            }
        }
        memset(tempt, 0, 100);
    }
}

int runlink(const char *pathname) {
    if(strlen(pathname)>1024)return -1;
    node *present_class;
    node *priority;
    present_class = root;
    priority =root;
    int index = 0;
    char *tempt = malloc(100 * sizeof(char));
    char *legal = malloc(3);
    memset(tempt, 0, 32);
    if(*pathname!=47){
        free(tempt);
        free(legal);
        return -1;
    }
    while (1) {
        int i = 0;
        int judge = 0;
        int bool_delete = 0;
        while (1) {
            if (judge == 0) {
                if (*(pathname + index) != 47) {
                    judge = 1;
                    memcpy(tempt, (pathname + index),1);
                    i++;
                }
                if(*(pathname+index)==0)return -1;//把开头的许多//都给直接跳过去
            } else {
                if (*(pathname + index) == 0) {
                    bool_delete = 1;
                    break;
                } //发现是文件了
                else if (*(pathname + index) == 47 && *(pathname + index + 1) != 47 && *(pathname + index + 1) != 0) {
                    break;
                }// 发现上册目录
                else if (*(pathname + index) != 47) {
                    memcpy(legal,pathname+index,1);
                    if(bool_legal(legal)==0)
                    {memcpy(tempt + i, pathname + index, 1);}
                    if(bool_legal(legal)==-1)return -1;
                    i++;
                }
            }
            index++;
        }
        int length= strlen(tempt);
        if(length>32)return -1;
        if (bool_delete == 0) {
            if(present_class==NULL)return -1;
            while (1) {
                if (strcmp(tempt, present_class->name) == 0) {
                    priority = present_class;
                    present_class = present_class->next_class;
                    break;
                } else if ( present_class->next_unit == NULL) {
                    free(tempt);
                    return -1;
                } else {
                    priority = present_class;
                    present_class = present_class->next_unit;
                }
            }
        }
        //找父目录
        if (bool_delete == 1) {
            if(present_class==NULL)return -1;
            if(strcmp(present_class->name,tempt)==0){
                if (present_class->type != file ) {
                    free(tempt);
                    return -1;
                } else {
                    if (present_class->next_unit == NULL) {
                        priority->next_class = NULL;
                        free(present_class);
                    } else {
                        present_class = present_class->next_unit;
                        priority->next_class = present_class;
                    }
                    return 0;
                }
            }
            while (1) {
                if (strcmp(tempt, present_class->name) == 0) {
                    if (present_class->type != file ) {
                        free(tempt);
                        return -1;
                    } else {
                        if (present_class->next_unit == NULL) {
                            priority->next_unit = NULL;
                            free(present_class);
                        } else {
                            present_class = present_class->next_unit;
                            priority->next_unit = present_class;
                        }
                        return 0;
                    }
                } else if (strcmp(tempt, present_class->name) != 0 && present_class->next_unit != NULL) {
                    priority = present_class;
                    present_class = present_class->next_unit;
                } else if (strcmp(tempt, present_class->name) != 0 && present_class->next_unit == NULL) {
                    free(tempt);
                    return -1;
                }
            }
        }
        memset(tempt, 0, 100);
    }
}


void init_ramfs() {
    root = malloc(sizeof(node));
    root->next_unit = NULL;
    root->next_class = NULL;
    root->name = malloc(32 * sizeof(char));
    root->FD = -1;
    root->type = directory;
    root->size = 0;
    root->offset = 0;
    root->flag = -1;
    root->content= malloc(1);
}

/*#include "ramfs.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool IsLeagal(const char *pathname);

bool IsFdLeagal(int fd);

int findfd();

#define LEN 1000
#define FDMAX 5000
typedef struct File_node {
    enum {
        FILE_NODE, DIR_NODE
    } type;
    struct File_node **dirents; // if it's a dir, there's subentries.error
    void *content; // if it's a file, there's data content
    int size; // size of file
    char *name;
    off_t index;
} File_node;
typedef struct FD {
    off_t offset;
    int flags;
    File_node *f;
} FD;
FD open_file[5001];
File_node Root;

int ropen(const char *pathname, int flags) {
    if (pathname == NULL) {
        return -1;
    }
    if (!IsLeagal(pathname)) {
        return -1;
    }
    if ((flags & O_RDONLY) == 0 && (flags & O_RDWR) == 0 && (flags & O_APPEND) == 0 && (flags & O_CREAT) == 0 &&
        (flags & O_TRUNC) && (flags & O_WRONLY) == 0) {
        return -1;
    }
    int len = (int) strlen(pathname);
    int fd_index = findfd();
    int length = 0;
    for (int i = 0; i < len; ++i) {
        if (pathname[i] == '/') {
            length++;
        }
    }
    if (len == length) {
        open_file[fd_index].flags = flags;
        open_file[fd_index].f = &Root;
        open_file[fd_index].offset = 0;
        return fd_index;
    }
    File_node *cur = &Root;
    char *mid = calloc(50, sizeof(char));
    for (int i = 0; i < len;) {
        if (pathname[i] == '/') {
            i++;
        } else {
            for (int j = i + 1; j <= len; ++j) {
                if (j == len || pathname[j] == '/') {
                    if (j - i > 32) {
                        free(mid);
                        return -1;
                    }
                    memcpy(mid, pathname + i, j - i);
                    int judge = 0;
                    for (int k = j + 1; k < len; ++k) {
                        if (pathname[k] != '/') {
                            judge++;
                            break;
                        }
                    }
                    if (judge != 0) {
                        File_node *dir = cur;
                        for (int k = 0; k < cur->index; ++k) {
                            if (cur->dirents[k]->type == DIR_NODE && strcmp(cur->dirents[k]->name, mid) == 0) {
                                cur = cur->dirents[k];
                                break;
                            }
                        }
                        if (cur == dir) {
                            free(mid);
                            return -1;
                        }
                        memset(mid, '\0', j - i);
                    } else {
                        for (int k = 0; k < cur->index; ++k) {
                            if (strcmp(cur->dirents[k]->name, mid) == 0) {
                                cur = cur->dirents[k];
                                break;
                            }
                        }
                    }
                    if (cur->type == FILE_NODE && judge != 0) {
                        free(mid);
                        return -1;
                    }
                    i = j;
                    break;
                }
            }
        }
    }
    if (cur->type == FILE_NODE && strcmp(cur->name, mid) == 0 && pathname[len - 1] != '/') {
        open_file[fd_index].flags = flags;
        open_file[fd_index].f = cur;
        open_file[fd_index].offset = 0;
        if ((open_file[fd_index].flags & O_TRUNC) != 0 &&
            ((open_file[fd_index].flags & 1) != 0 || (open_file[fd_index].flags & (1 << 1)) != 0)) {
            memset(open_file[fd_index].f->content, '\0', open_file[fd_index].f->size);
            open_file[fd_index].f->size = 0;
        }
        if (open_file[fd_index].flags >= O_APPEND) {
            open_file[fd_index].offset = open_file[fd_index].f->size;
        }
        free(mid);
        return fd_index;
    } else if (cur->type == DIR_NODE && strcmp(cur->name, mid) == 0) {
        open_file[fd_index].flags = flags;
        open_file[fd_index].f = cur;
        open_file[fd_index].offset = 0;
        free(mid);
        return fd_index;
    }
    if ((flags & O_CREAT) != 0 && pathname[len - 1] != '/') {
        if (pathname[len - 1] == '/') {
            free(mid);
            return -1;
        }
        File_node *pro = calloc(1, sizeof(File_node));
        pro->index = LEN;
        pro->name = mid;
        pro->type = FILE_NODE;
        pro->size = 0;
        pro->content = calloc(LEN, sizeof(void));
        cur->dirents[cur->index] = pro;
        cur->index++;
        if (cur->index == cur->size) {
            cur->size += LEN;
            cur->dirents = realloc(cur->dirents, cur->size* sizeof(File_node*));
        }
        open_file[fd_index].flags = flags;
        open_file[fd_index].f = pro;
        open_file[fd_index].offset = 0;
        if ((open_file[fd_index].flags & O_TRUNC) != 0 &&
            ((open_file[fd_index].flags & 1) != 0 || (open_file[fd_index].flags & (1 << 1)) != 0)) {
            memset(open_file[fd_index].f->content, '\0', open_file[fd_index].f->size);
            open_file[fd_index].f->size = 0;
        }
        if (open_file[fd_index].flags >= O_APPEND) {
            open_file[fd_index].offset = open_file[fd_index].f->size;
        }
        return fd_index;
    }
    free(mid);
    return -1;
}

int rclose(int fd) {
    if (!IsFdLeagal(fd)) {
        return -1;
    }
    open_file[fd].f = NULL;
    return 0;
}

ssize_t rwrite(int fd, const void *buf, size_t count) {
    if (buf == NULL) {
        return -1;
    }
    if (!IsFdLeagal(fd)) {
        return -1;
    }
    if ((open_file[fd].flags & 1) == 0 && (open_file[fd].flags & (1 << 1)) == 0) {
        return -1;
    }
    if (open_file[fd].f->type == DIR_NODE) {
        return -1;
    }
    if (count == 0) {
        return 0;
    }
    if (count + open_file[fd].offset > open_file[fd].f->index) {
        open_file[fd].f->index = (off_t) count + open_file[fd].offset + 1;
        open_file[fd].f->content = realloc(open_file[fd].f->content,open_file[fd].f->index);
    }
    memcpy(open_file[fd].f->content + open_file[fd].offset, buf, count);
    if (open_file[fd].offset + count > open_file[fd].f->size) {
        open_file[fd].f->size = open_file[fd].offset + (off_t) count;
    }
    open_file[fd].offset += (off_t) count;
    return (ssize_t) count;
}

ssize_t rread(int fd, void *buf, size_t count) {
    if (buf == NULL) {
        return -1;
    }
    if (!IsFdLeagal(fd)) {
        return -1;
    }
    if ((open_file[fd].flags & 1) != 0) {
        return -1;
    }
    if (open_file[fd].f->type == DIR_NODE) {
        return -1;
    }
    if (open_file[fd].offset + count > open_file[fd].f->size) {
        int mid = open_file[fd].f->size - open_file[fd].offset;
        if (mid < 0) {
            return -1;
        }
        memcpy(buf, open_file[fd].f->content + open_file[fd].offset, mid);
        open_file[fd].offset = open_file[fd].f->size;
        return mid;
    } else {
        memcpy(buf, open_file[fd].f->content + open_file[fd].offset, count);
        open_file[fd].offset += (off_t) count;
        return (ssize_t) count;
    }
}

off_t rseek(int fd, off_t offset, int whence) {
    if (!IsFdLeagal(fd)) {
        return -1;
    }
    off_t mid = open_file[fd].offset;
    if (whence == SEEK_SET) {
        open_file[fd].offset = offset;
    } else if (whence == SEEK_CUR) {
        open_file[fd].offset += offset;
    } else if (whence == SEEK_END) {
        open_file[fd].offset = offset + (off_t) open_file[fd].f->size;
    } else {
        return -1;
    }
    if (open_file[fd].offset < 0) {
        open_file[fd].offset = mid;
        return -1;
    }
    return open_file[fd].offset;
}

int rmkdir(const char *pathname) {
    if (pathname == NULL) {
        return -1;
    }
    if (!IsLeagal(pathname)) {
        return -1;
    }
    File_node *cur = &Root;
    int len = (int) strlen(pathname);
    int length = 0;
    for (int i = 0; i < len; ++i) {
        if (pathname[i] == '/') {
            length++;
        }
    }
    if (len == length) {
        return -1;
    }
    char *mid = calloc(50, sizeof(char));
    int count = 0;
    for (int i = 0; i < len;) {
        if (pathname[i] == '/') {
            i++;
        } else {
            for (int j = i + 1; j <= len; ++j) {
                if (j == len || pathname[j] == '/') {
                    if (j - i > 32) {
                        free(mid);
                        return -1;
                    }
                    memcpy(mid, pathname + i, j - i);
                    int judge = 0;
                    for (int k = j + 1; k < len; ++k) {
                        if (pathname[k] != '/') {
                            judge++;
                            break;
                        }
                    }
                    if (judge != 0) {
                        File_node *dir = cur;
                        for (int k = 0; k < cur->index; ++k) {
                            if (cur->dirents[k]->type == DIR_NODE && strcmp(cur->dirents[k]->name, mid) == 0) {
                                cur = cur->dirents[k];
                                break;
                            }
                        }
                        if (cur == dir) {
                            count++;
                        }
                        memset(mid, '\0', j - i);
                    }
                    i = j;
                    break;
                }
            }
        }
    }
    if (count >= 1) {
        free(mid);
        return -1;
    }
    for (int i = 0; i < cur->index; ++i) {
        if (strcmp(mid, cur->dirents[i]->name) == 0) {
            free(mid);
            return -1;
        }
    }
    File_node *pro = calloc(1, sizeof(File_node));
    pro->dirents = calloc(LEN, sizeof(File_node *));
    pro->index = 0;
    pro->type = DIR_NODE;
    pro->name = mid;
    pro->size =  LEN;
    pro->dirents[pro->index] = NULL;
    cur->dirents[cur->index] = pro;
    cur->index++;
    if (cur->index == cur->size) {
        cur->size += LEN;
        cur->dirents = realloc(cur->dirents, cur->size* sizeof(File_node*));
    }
    return 0;
}

int rrmdir(const char *pathname) {
    if (pathname == NULL) {
        return -1;
    }
    if (!IsLeagal(pathname)) {
        return -1;
    }
    File_node *pre = &Root;
    File_node *cur = &Root;
    int len = (int) strlen(pathname);
    int length = 0;
    int index;
    for (int i = 0; i < len; ++i) {
        if (pathname[i] == '/') {
            length++;
        }
    }
    if (len == length) {
        return -1;
    }
    char *mid = calloc(50, sizeof(char));
    for (int i = 0; i < len;) {
        if (pathname[i] == '/') {
            i++;
        } else {
            for (int j = i + 1; j <= len; ++j) {
                if (j == len || pathname[j] == '/') {
                    if (j - i > 32) {
                        free(mid);
                        return -1;
                    }
                    memcpy(mid, pathname + i, j - i);
                    int judge = 0;
                    for (int k = j + 1; k < len; ++k) {
                        if (pathname[k] != '/') {
                            judge++;
                            break;
                        }
                    }
                    File_node *dir = cur;
                    for (int k = 0; k < cur->index; ++k) {
                        if (cur->dirents[k]->type == DIR_NODE && strcmp(cur->dirents[k]->name, mid) == 0) {
                            pre = cur;
                            cur = cur->dirents[k];
                            index = k;
                            break;
                        }
                    }
                    if (cur == dir) {
                        free(mid);
                        return -1;
                    }
                    if (judge != 0) {
                        memset(mid, '\0', j - i);
                    }
                    i = j;
                    break;
                }
            }
        }
    }
    if (cur->index == 0 && strcmp(cur->name, mid) == 0 && cur->type == DIR_NODE) {
        pre->index--;
        pre->dirents[index] = pre->dirents[pre->index];
        pre->dirents[pre->index] = NULL;
        free(cur->dirents);
        free(cur);
        free(mid);
        return 0;
    }
    free(mid);
    return -1;
}

int runlink(const char *pathname) {
    if (pathname == NULL) {
        return -1;
    }
    if (!IsLeagal(pathname)) {
        return -1;
    }
    File_node *pre = &Root;
    File_node *cur = &Root;
    int len = (int) strlen(pathname);
    int length = 0;
    for (int i = 0; i < len; ++i) {
        if (pathname[i] == '/') {
            length++;
        }
    }
    if (len == length) {
        return -1;
    }
    char *mid = calloc(50, sizeof(char));
    int index;
    for (int i = 0; i < len;) {
        if (pathname[i] == '/') {
            i++;
        } else {
            for (int j = i + 1; j <= len; ++j) {
                if (j == len || pathname[j] == '/') {
                    if (j - i > 32) {
                        free(mid);
                        return -1;
                    }
                    int judge = 0;
                    for (int k = j + 1; k < len; ++k) {
                        if (pathname[k] != '/') {
                            judge++;
                            break;
                        }
                    }
                    memcpy(mid, pathname + i, j - i);
                    if (judge != 0) {
                        File_node *dir = cur;
                        for (int k = 0; k < cur->index; ++k) {
                            if (cur->dirents[k]->type == DIR_NODE && strcmp(cur->dirents[k]->name, mid) == 0) {
                                pre = cur;
                                cur = cur->dirents[k];
                                index = k;
                                break;
                            }
                        }
                        if (cur == dir) {
                            free(mid);
                            return -1;
                        }
                        memset(mid, '\0', j - i);
                    } else {
                        for (int k = 0; k < cur->index; ++k) {
                            if (cur->dirents[k]->type == FILE_NODE && strcmp(cur->dirents[k]->name, mid) == 0) {
                                pre = cur;
                                cur = cur->dirents[k];
                                index = k;
                                break;
                            }
                        }
                    }
                    i = j;
                    break;
                }
            }
        }
    }
    if (strcmp(cur->name, mid) == 0 && cur->type == FILE_NODE && pathname[len - 1] != '/') {
        pre->index--;
        pre->dirents[index] = pre->dirents[pre->index];
        pre->dirents[pre->index] = NULL;
        free(mid);
        free(cur->content);
        free(cur);
        return 0;
    }
    free(mid);
    return -1;
}

void init_ramfs() {
    Root.dirents = calloc( LEN, sizeof(File_node *));
    Root.name = "/";
    Root.index = 0;
    Root.type = DIR_NODE;
    Root.size =  LEN;
}

bool IsLeagal(const char *pathname) {
    int len = strlen(pathname);
    if (len > 1024) {
        return false;
    }
    if (pathname[0] != '/') {
        return false;
    }
    for (int i = 0; i < len; ++i) {
        if (!((pathname[i] >= '0' && pathname[i] <= '9') || (pathname[i] >= 'a' && pathname[i] <= 'z')
              || (pathname[i] >= 'A' && pathname[i] <= 'Z') || pathname[i] == '/' || pathname[i] == '.')) {
            return false;
        }
    }
    return true;
}

bool IsFdLeagal(int fd) {
    if (fd < 0 || fd > FDMAX || open_file[fd].f == NULL) {
        return false;
    }
    return true;
}

int findfd() {
    for (int i = 0; i <= FDMAX; ++i) {
        if (open_file[i].f == NULL) {
            return i;
        }
    }
    return -1;
}*/