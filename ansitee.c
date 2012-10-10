/* ansitee
 * Paul Battley <pbattley@gmail.com> 2012
 * Based on FreeBSD's tee.c, whose copyright follows.
 */

/*
 * Copyright (c) 1988, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _list {
    struct _list *next;
    int fd;
    int strip_ansi;
    const char *name;
} LIST;
static LIST *head;

static void add(int, int, const char *);
static void usage(void);
static ssize_t write_without_ansi(int, const void *, size_t);

int
main(int argc, char *argv[]) {
    LIST *p;
    int n, fd, rval, wval;
    char *bp;
    int append, strip_ansi, ch, exitval;
    char *buf;
#define BSIZE (8 * 1024)

    append = 0;
    strip_ansi = 0;
    while ((ch = getopt(argc, argv, "aish")) != -1) {
        switch((char)ch) {
            case 'a':
                append = 1;
                break;
            case 'i':
                (void)signal(SIGINT, SIG_IGN);
                break;
            case 's':
                strip_ansi = 1;
                break;
            case '?':
            case 'h':
            default:
                usage();
        }
    }
    argv += optind;
    argc -= optind;

    if ((buf = malloc(BSIZE)) == NULL) {
        err(1, "malloc");
    }

    add(STDOUT_FILENO, 0, "stdout");

    for (exitval = 0; *argv; ++argv) {
        if ((fd = open(*argv,
                       O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC),
                       DEFFILEMODE)) < 0) {
            warn("%s", *argv);
            exitval = 1;
        } else {
            add(fd, strip_ansi, *argv);
        }
    }

    while ((rval = read(STDIN_FILENO, buf, BSIZE)) > 0) {
        for (p = head; p; p = p->next) {
            n = rval;
            bp = buf;
            do {
                if (p->strip_ansi == 0) {
                    wval = write(p->fd, bp, n);
                } else {
                    wval = write_without_ansi(p->fd, bp, n);
                }
                if (wval == -1) {
                    warn("%s", p->name);
                    exitval = 1;
                    break;
                }
                bp += wval;
            } while (n -= wval);
        }
    }
    if (rval < 0) {
        err(1, "read");
    }
    exit(exitval);
}

static void
usage(void) {
    (void)fprintf(stderr, "usage: tee [-ai] [file ...]\n");
    exit(1);
}

static void
add(int fd, int strip_ansi, const char *name) {
    LIST *p;

    if ((p = malloc(sizeof(LIST))) == NULL) {
        err(1, "malloc");
    }
    p->fd = fd;
    p->name = name;
    p->strip_ansi = strip_ansi;
    p->next = head;
    head = p;
}

static ssize_t
write_without_ansi(int fd, const void *buf, size_t count) {
    int escaped = 0, i;
    char ch;
    for (i = 0; i < count; i++) {
        ch = ((char *)buf)[i];
        switch (escaped) {
            case 0:
                if (ch == 0x1B) { // Possible lead byte
                    escaped = 1;
                } else { // Normal output
                    write(fd, &ch, 1);
                }
                break;
            case 1:
                if (ch == '[') { // Now in an escape sequence
                    escaped = 2;
                } else { // False alarm. Emit the ESC
                    escaped = 0;
                    write(fd, "\x1B", 1);
                }
                break;
            case 2:
                if (ch >= 0x40 && ch <= 0x7E) { // End of sequence
                    escaped = 0;
                }
        }
    }
}
