CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lrt

SRC_DIR = src

ALL = logger core ui launcher standalone

all: $(ALL)

logger: $(SRC_DIR)/Ausaf_log.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

core: $(SRC_DIR)/wilona_core.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

ui: $(SRC_DIR)/najim_ui.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

launcher: launcher.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

standalone: $(SRC_DIR)/standalone.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(ALL) *.log
