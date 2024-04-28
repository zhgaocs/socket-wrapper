#ifndef UTILS_H
#define UTILS_H 1

#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <sysexits.h>

extern int sessions;
extern int message_size;

void parse_arguments(char *const *const argv, int argc);

#endif