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
            exit(EXIT_SUCCESS);

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
            exit(EXIT_FAILURE);

        case '?':
            if (optopt)
                fprintf(stderr,
                        "%s: invalid option -- '%c'\nTry '%s --help' for more information.\n",
                        argv[0], optopt, argv[0]);
            else
                fprintf(stderr,
                        "%s: unrecognized option '%s'\nTry '%s --help' for more information.\n",
                        argv[0], argv[optind - 1], argv[0]);
            exit(EXIT_FAILURE);

        default:
            opterr = 1;
            exit(EXIT_FAILURE);
        }
    }
}
