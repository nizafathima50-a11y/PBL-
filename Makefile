CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LIBS = -lrt

all: logger core ui launcher standalone

logger: Ausaf_log.c
	$(CC) $(CFLAGS) Ausaf_log.c -o logger $(LIBS)

core: wilona_core.c
	$(CC) $(CFLAGS) wilona_core.c -o core $(LIBS)

ui: najim_ui.c
	$(CC) $(CFLAGS) najim_ui.c -o ui $(LIBS)

launcher: launcher.c
	$(CC) $(CFLAGS) launcher.c -o launcher $(LIBS)

standalone: standalone.c
	$(CC) $(CFLAGS) standalone.c -o standalone

clean:
	rm -f logger core ui launcher standalone *.log
	-ipcrm -a
