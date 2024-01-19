# SPDX-License-Identifier: GPL-2.0-or-later

CC = gcc -O

ptmcheck: ptmcheck.c
	$(CC) -o $@ ptmcheck.c -lpci

.PHONY: clean
clean:
	rm -f ptmcheck
