#!/bin/bash

mkdir -p tmp/scenariu_test_src/app tmp/scenariu_test_src/lib/include

cat << 'EOF' > tmp/scenariu_test_src/lib/include/util.h
int util_add(int a, int b);
EOF

cat << 'EOF' > tmp/scenariu_test_src/lib/util.c
#include "util.h"
int util_add(int a, int b) { return a + b; }
EOF

cat << 'EOF' > tmp/scenariu_test_src/app/main_demo.c
#include <stdio.h>
#include "util.h"
int main() {
    printf("%d\n", util_add(2,3));
    return 0;
}
EOF

export CFLAGS="-Itmp/scenariu_test_src/lib/include -std=c11 -Wall -Wextra"

./tools/fileops.sh build --src tmp/scenariu_test_src

if [ ! -x bin/demo ]; then
    echo "Executabilul bin/demo nu există!"
    exit 1
fi

./bin/demo > tmp/demo_out.txt

OUTPUT=$(cat tmp/demo_out.txt)
if [ "$OUTPUT" != "5" ]; then
    echo "Output incorect: $OUTPUT. Mă așteptam la 5."
    exit 1
fi

exit 0
