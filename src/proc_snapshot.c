#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <dirent.h>
#include <time.h>
#include "db_format.h"

int is_numeric(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') return 0;
    }
    return 1;
}

int main() {
    int db_fd = open("proc.db", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (db_fd < 0) { perror("Eroare creare DB"); return 1; }

    if (flock(db_fd, LOCK_EX) == -1) { perror("Eroare flock"); return 1; }

    db_header_t header = { MAGIC_NUMBER, DB_VERSION, time(NULL), STATUS_OPEN, TYPE_PROC, 0 };
    write(db_fd, &header, sizeof(db_header_t));

    DIR *dir = opendir("/proc");
    uint32_t records_written = 0;

    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && is_numeric(entry->d_name)) {
                char stat_path[256];
                snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);
                
                FILE *f = fopen(stat_path, "r");
                if (f) {
                    proc_record_t rec;
                    memset(&rec, 0, sizeof(rec));
                    rec.pid = atoi(entry->d_name);
                    
                    fscanf(f, "%*d (%255[^)]) %c %d", rec.name, &rec.state, &rec.ppid);
                    
                    write(db_fd, &rec, sizeof(proc_record_t));
                    records_written++;
                    fclose(f);
                }
            }
        }
        closedir(dir);
    }

    header.status = STATUS_SEALED;
    header.record_count = records_written;
    lseek(db_fd, 0, SEEK_SET);
    write(db_fd, &header, sizeof(db_header_t));

    flock(db_fd, LOCK_UN);
    close(db_fd);

    printf("Snapshot creat: %d procese salvate.\n", records_written);
    return 0;
}
