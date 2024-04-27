#include <chrono>
#include <thread>

#include "tcpclient.h"
#include "tcpserver.h"

#include "utils.h"

int sessions = 100;
int message_size = 128;

int main(int argc, char *argv[])
{
    parse_arguments(argv, argc);
}