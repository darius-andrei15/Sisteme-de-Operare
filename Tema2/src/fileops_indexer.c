#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include "db_format.h"

int db_fd;
uint32_t records_written = 0;

void process_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat file_stat;
        if (stat(full_path, &file_stat) == 0) {
            if (S_ISREG(file_stat.st_mode)) {
                file_record_t rec;
                memset(&rec, 0, sizeof(rec));
                strncpy(rec.path, full_path, sizeof(rec.path) - 1);
                rec.size = file_stat.st_size;
                rec.mode = file_stat.st_mode;
                rec.uid = file_stat.st_uid;
                rec.mtime = file_stat.st_mtime;

                write(db_fd, &rec, sizeof(file_record_t));
                records_written++;
            } else if (S_ISDIR(file_stat.st_mode)) {
                process_directory(full_path);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Utilizare: %s <director>\n", argv[0]);
        return 1;
    }

    db_fd = open("index.db", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (db_fd < 0) { perror("Eroare creare DB"); return 1; }

    if (flock(db_fd, LOCK_EX) == -1) { perror("Eroare flock"); close(db_fd); return 1; }

    db_header_t header = { MAGIC_NUMBER, DB_VERSION, time(NULL), STATUS_OPEN, TYPE_FILE, 0 };
    write(db_fd, &header, sizeof(db_header_t));

    process_directory(argv[1]);

    header.status = STATUS_SEALED;
    header.record_count = records_written;
    lseek(db_fd, 0, SEEK_SET); 
    write(db_fd, &header, sizeof(db_header_t));

    flock(db_fd, LOCK_UN);
    close(db_fd);

    printf("Indexare completă: %d fișiere scrise.\n", records_written);
    return 0;
}
