#ifndef DB_FORMAT_H
#define DB_FORMAT_H

#include <stdint.h>

#define MAGIC_NUMBER 0x464F5053
#define DB_VERSION 1

#define STATUS_OPEN 0
#define STATUS_SEALED 1

#define TYPE_FILE 0
#define TYPE_PROC 1

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint64_t timestamp;
    uint8_t status;
    uint8_t type;
    uint32_t record_count;
} db_header_t;

typedef struct {
    char path[256];
    uint64_t size;
    uint32_t mode;
    uint32_t uid;
    uint64_t mtime;
} file_record_t;

typedef struct {
    uint32_t pid;
    uint32_t ppid;
    char name[256];
    char state;
} proc_record_t;

#endif
