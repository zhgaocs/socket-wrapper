#include "utils.h"

void parse_arguments(char *const *const argv, int argc)
{
    option longopts[] = {
        {"help", no_argument, nullptr, 'h'},
        {"message-size", required_argument, nullptr, 'm'},
        {"sessions", required_argument, nullptr, 's'},
        {nullptr, 0, nullptr, 0}};

    int opt, longindex = 0;

    while ((opt = getopt_long(argc, argv, ":hm:s:", longopts, &longindex)) != -1)
    {
        switch (opt)
        {
        case 'h':
            fprintf(stdout,
                    "Usage: %s [OPTIONS]\n"
                    "Options:\n"
                    "  -h, --help            Show this help message and exit\n"
                    "  -m, --message-size    Set the message size (default: 128)\n"
                    "  -s, --sessions        Set the number of sessions (default: 100)\n",
                    argv[0]);
            exit(EX_OK);

        case 'm':
            message_size = atoi(optarg);
            break;

        case 's':
            sessions = atoi(optarg);
            break;

        case ':':
            fprintf(stderr,
                    "%s: missing parameter\nTry '%s --help' for more information.\n",
                    argv[0], argv[0]);
            exit(EX_USAGE);

        case '?':
            if (optopt)
                fprintf(stderr,
                        "%s: invalid option -- '%c'\nTry '%s --help' for more information.\n",
                        argv[0], optopt, argv[0]);
            else
                fprintf(stderr,
                        "%s: unrecognized option '%s'\nTry '%s --help' for more information.\n",
                        argv[0], argv[optind - 1], argv[0]);
            exit(EX_USAGE);

        default:
            exit(EX_USAGE);
        }
    }
}

void fill_buffer(char *buf, size_t bufsize)
{
    constexpr char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (bufsize)
    {
        for (int i = 0; i < bufsize; ++i)
        {
            int idx = rand() % (sizeof(charset) - 1);
            buf[i] = charset[idx];
        }
    }
}