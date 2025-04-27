#! /bin/bash


if [[ $# -ne 2 ]];
then
	echo -e Usage: bash gtk_compile \"compile codes\" executable name
else
	gcc $1 -o $2 $(pkg-config --cflags --libs gtk+-3.0 gdk-pixbuf-2.0)
fi
