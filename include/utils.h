#ifndef UTILS_H
#define UTILS_H 1

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

extern int sessions;
extern int message_size;

void parse_arguments(char *const *const argv, int argc);
void fill_buffer(char *buf, size_t bufsize);

#endif