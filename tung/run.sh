#! /bin/bash

bash gtk_compile.sh "sahur_gen.c" out
gcc -o bomb bomb.c

./bomb out