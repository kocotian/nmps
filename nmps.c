#define _XOPEN_SOURCE 700

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "arg.h"
#include "http.h"
#include "util.c"

#define VERSION "a0.1.1"

extern void herror(const char *s);

static size_t authorize(const char *host, const char *port, const char *username, const char *password);
static size_t request(char *hostname, unsigned short port, char *command, char *args[], char **buffer);
static void usage(void);

char *argv0;

static size_t
authorize(const char *host, const char *port, const char *username, const char *password)
{
	size_t size;
	char *buffer, *args[] = {username, password, NULL};
	if (!(size = request(host, atoi(port), "/auth", args, &buffer)))
		return 1;
	else {
		if(!strcmp(truncateHeader(buffer), "Authorized"))
			return size;
		else
			die("%s", truncateHeader(buffer));
	}
}

static size_t
request(char *hostname, unsigned short port,
		char *command, char *args[], char **buffer)
{
	char *argv = calloc(0, 1);
	char *request_template = "GET %s?argv=%s HTTP/1.0\r\nHost: %s\r\n\r\n";
	char request[BUFSIZ];
	int iter = -1, argvsize = 0, request_length;
	while (args[++iter] != NULL) {
		argv = realloc(argv, argvsize += strlen(args[iter]) + 1);
		strcat(argv, args[iter]);
		strcat(argv, "\1");
	}
	request_length = snprintf(request, BUFSIZ, request_template,
			command, argv, hostname);
	return sendHTTPRequest(hostname, port, request, request_length, buffer);
}

static void
usage(void)
{
	die("usage: %s [-p port] <host>", argv0);
}

int
main(int argc, char *argv[])
{
	char *port = "80", *host = "localhost", username[32] = "unknown", password[64] = "";
	int character, chiter = -1;

	ARGBEGIN {
	case 'p':
		port = ARGF();
		break;
	case 'h': /* fallthrough */
	default:
		usage();
		break;
	} ARGEND;

	if (argc != 1)
		usage();

	getlogin_r(username, 32);
	host = argv[argc - 1];

	fprintf(stderr, "Connecting to %s ", host);
	if (gethostbyname(host) == NULL) {
		herror("failed");
		return -1;
	}

	fprintf(stderr, "succeeded!\n");
	fprintf(stderr, "%s@%s's password: \033[8m", username, host);
	while ((character = fgetc(stdin)) != '\n') {
		password[++chiter] = character;
	}
	password[++chiter] = 0;
	fprintf(stderr, "\033[0m");

	if(authorize(host, port, username, password))
		printf("Authorization successful!\n");
	return 0;
}
