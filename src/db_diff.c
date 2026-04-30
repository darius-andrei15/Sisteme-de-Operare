#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "db_format.h"

void diff_files(int fd1, int fd2, uint32_t count1, uint32_t count2) {
    file_record_t *db1 = malloc(count1 * sizeof(file_record_t));
    file_record_t *db2 = malloc(count2 * sizeof(file_record_t));
    read(fd1, db1, count1 * sizeof(file_record_t));
    read(fd2, db2, count2 * sizeof(file_record_t));

    printf("--- Modificări sau Adăugări ---\n");
    for (uint32_t i = 0; i < count2; i++) {
        int found = 0;
        for (uint32_t j = 0; j < count1; j++) {
            if (strcmp(db2[i].path, db1[j].path) == 0) {
                found = 1;
                if (db2[i].size != db1[j].size || db2[i].mtime != db1[j].mtime) {
                    printf("[MODIFICAT] %s (Dimensiune nouă: %lu)\n", db2[i].path, db2[i].size);
                }
                break;
            }
        }
        if (!found) printf("[ADĂUGAT] %s\n", db2[i].path);
    }

    printf("\n--- Ștergeri ---\n");
    for (uint32_t i = 0; i < count1; i++) {
        int found = 0;
        for (uint32_t j = 0; j < count2; j++) {
            if (strcmp(db1[i].path, db2[j].path) == 0) { found = 1; break; }
        }
        if (!found) printf("[ȘTERS] %s\n", db1[i].path);
    }

    free(db1); free(db2);
}

void diff_procs(int fd1, int fd2, uint32_t count1, uint32_t count2) {
    proc_record_t *db1 = malloc(count1 * sizeof(proc_record_t));
    proc_record_t *db2 = malloc(count2 * sizeof(proc_record_t));
    read(fd1, db1, count1 * sizeof(proc_record_t));
    read(fd2, db2, count2 * sizeof(proc_record_t));

    printf("--- Procese Noi / Schimbare de Stare ---\n");
    for (uint32_t i = 0; i < count2; i++) {
        int found = 0;
        for (uint32_t j = 0; j < count1; j++) {
            if (db2[i].pid == db1[j].pid) {
                found = 1;
                if (db2[i].state != db1[j].state) {
                    printf("[STARE MODIFICATĂ] PID %d (%s): %c -> %c\n", 
                           db2[i].pid, db2[i].name, db1[j].state, db2[i].state);
                }
                break;
            }
        }
        if (!found) printf("[PROCES NOU] PID %d (%s)\n", db2[i].pid, db2[i].name);
    }

    printf("\n--- Procese Închise ---\n");
    for (uint32_t i = 0; i < count1; i++) {
        int found = 0;
        for (uint32_t j = 0; j < count2; j++) {
            if (db1[i].pid == db2[j].pid) { found = 1; break; }
        }
        if (!found) printf("[PROCES ÎNCHIS] PID %d (%s)\n", db1[i].pid, db1[i].name);
    }

    free(db1); free(db2);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Utilizare: %s <db1> <db2>\n", argv[0]);
        return 1;
    }

    int fd1 = open(argv[1], O_RDONLY);
    int fd2 = open(argv[2], O_RDONLY);

    db_header_t h1, h2;
    read(fd1, &h1, sizeof(db_header_t));
    read(fd2, &h2, sizeof(db_header_t));

    if (h1.magic != MAGIC_NUMBER || h2.magic != MAGIC_NUMBER) {
        fprintf(stderr, "Eroare: Magic number invalid!\n"); return 1;
    }
    if (h1.type != h2.type) {
        fprintf(stderr, "Eroare: Nu poți compara un index de fișiere cu unul de procese!\n"); return 1;
    }

    if (h1.type == TYPE_FILE) diff_files(fd1, fd2, h1.record_count, h2.record_count);
    else diff_procs(fd1, fd2, h1.record_count, h2.record_count);

    close(fd1); close(fd2);
    return 0;
}
