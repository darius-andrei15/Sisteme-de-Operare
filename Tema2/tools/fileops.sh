#!/bin/bash

EXIT_CODE=0
mkdir -p logs
LOG_FILE="logs/fileops.log"

echo "[$(date '+%Y-%m-%d %H:%M:%S')] Executat: $0 $@" >> "$LOG_FILE"

CMD="$1"

compile_recursive() {
    local current_dir="$1"
    
    for file in "$current_dir"/*; do
        if [ -d "$file" ]; then
            compile_recursive "$file"
            
        elif [ -f "$file" ] && [[ "$file" == *.c ]]; then
            local base=$(basename "$file")
            local name="${base%.c}"
            local obj="tmp/obj/${name}.o"

            if [ ! -f "$obj" ] || [ "$file" -nt "$obj" ]; then
                echo "Compilez: $file -> $obj"
                gcc -c ${CFLAGS} "$file" -o "$obj" -Iinclude
                
                if [ $? -ne 0 ]; then
                    echo "Eroare la compilarea $file" >&2
                    EXIT_CODE=1
                fi
            fi
        fi
    done
}

case "$CMD" in

    init)
        echo "Inițializez structura de directoare..."
        mkdir -p src/app src/lib bin tests tools tmp/obj reports logs include doc
        echo "Structura a fost creată cu succes."
        ;;

    build)
        echo "Încep procesul de build..."
        mkdir -p bin tmp/obj
        
        SRC_DIR="src"
        if [ "$2" == "--src" ] && [ -n "$3" ]; then
            SRC_DIR="$3"
        fi

        if [ -d "$SRC_DIR" ]; then
            compile_recursive "$SRC_DIR"
        fi

        shopt -s nullglob
        ALL_OBJS=(tmp/obj/*.o)
        
        if [ ${#ALL_OBJS[@]} -gt 0 ]; then
            AUX_OBJS=()
            MAIN_OBJS=()
            
            for obj in "${ALL_OBJS[@]}"; do
                base=$(basename "$obj")
                if [[ "$base" == main_* ]]; then
                    MAIN_OBJS+=("$obj")
                else
                    AUX_OBJS+=("$obj")
                fi
            done
            
            for main_obj in "${MAIN_OBJS[@]}"; do
                base=$(basename "$main_obj")
                exe_name="${base#main_}"
                exe_name="${exe_name%.o}"
                exe_path="bin/$exe_name"
                
                gcc ${CFLAGS} "$main_obj" "${AUX_OBJS[@]}" -o "$exe_path" -Iinclude
                if [ $? -eq 0 ]; then
                    echo "Executabil standard creat: $exe_path"
                else
                    echo "Eroare la linkarea $exe_path" >&2
                    EXIT_CODE=1
                fi
            done
        fi
        shopt -u nullglob

        echo "Compilez utilitarele pentru T3..."
        
        if [ -f "src/fileops_indexer.c" ]; then
            gcc src/fileops_indexer.c -o bin/fileops_indexer -Iinclude -Wall
            echo "Compilat: fileops_indexer"
        fi
        
        if [ -f "src/proc_snapshot.c" ]; then
            gcc src/proc_snapshot.c -o bin/proc_snapshot -Iinclude -Wall
            echo "Compilat: proc_snapshot"
        fi
        
        if [ -f "src/db_diff.c" ]; then
            gcc src/db_diff.c -o bin/db_diff -Iinclude -Wall
            echo "Compilat: db_diff"
        fi
        
        echo "Build finalizat."
        ;;

    run)
        shift
        EXE="$1"
        shift
        if [ -x "bin/$EXE" ]; then
            "./bin/$EXE" "$@"
        else
            echo "Eroare: Executabilul bin/$EXE nu există sau nu are permisiuni de execuție."
            EXIT_CODE=1
        fi
        ;;

    clean)
        rm -f tmp/obj/*.o bin/* *.db
        echo "Curățenie completă finalizată."
        ;;

    test)
        mkdir -p reports
        REPORT_FILE="reports/T2_tests.txt"
        > "$REPORT_FILE" 
        FAIL_COUNT=0

        echo "Rulez testele automate..."
        while IFS= read -r test_script; do
            echo "Rulez test: $test_script"
            if bash "$test_script"; then
                echo "$test_script: PASS" >> "$REPORT_FILE"
            else
                echo "$test_script: FAIL" >> "$REPORT_FILE"
                FAIL_COUNT=$((FAIL_COUNT + 1))
            fi
        done < <(find tests -type f -name "*.sh")

        if [ "$FAIL_COUNT" -gt 0 ]; then
            echo "Atenție: $FAIL_COUNT teste au picat. Verifică $REPORT_FILE"
            EXIT_CODE=1
        else
            echo "Toate testele au trecut cu succes!"
        fi
        ;;

    index)
        shift
        if [ -z "$1" ] || [ ! -d "$1" ]; then
            echo "Eroare. Utilizare: ./tools/fileops.sh index <director>"
            EXIT_CODE=1
        else
            if [ -x "./bin/fileops_indexer" ]; then
                ./bin/fileops_indexer "$1"
            else
                echo "Eroare: Utilitarul fileops_indexer lipsește. Rulează build."
                EXIT_CODE=1
            fi
        fi
        ;;

    proc)
        if [ -x "./bin/proc_snapshot" ]; then
            ./bin/proc_snapshot
        else
            echo "Eroare: Utilitarul proc_snapshot lipsește. Rulează build."
            EXIT_CODE=1
        fi
        ;;

    diff)
        shift
        if [ "$#" -ne 2 ]; then
            echo "Eroare. Utilizare: ./tools/fileops.sh diff <db_vechi> <db_nou>"
            EXIT_CODE=1
        else
            if [ -x "./bin/db_diff" ]; then
                ./bin/db_diff "$1" "$2"
            else
                echo "Eroare: Utilitarul db_diff lipsește. Rulează build."
                EXIT_CODE=1
            fi
        fi
        ;;

    *)
        echo "Comandă necunoscută: $CMD"
        echo "Comenzi disponibile:"
        echo "  - TEMA 2: init, build [--src <dir>], run <exe> [args...], clean, test"
        echo "  - TEMA 3: index <dir>, proc, diff <db_v1> <db_v2>"
        EXIT_CODE=1
        ;;
esac

echo "[$(date '+%Y-%m-%d %H:%M:%S')] Finalizat cu cod de ieșire: $EXIT_CODE" >> "$LOG_FILE"
echo "----------------------------------------" >> "$LOG_FILE"

exit $EXIT_CODE
