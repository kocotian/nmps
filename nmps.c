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

#define VERSION "a0.2.1"

extern void herror(const char *s);

static size_t authorize(char *host, const char *port, char *username, char *password);
static int command(char *command, char *args, char *host, char *port);
static int gameplay(char *username, char *host, char *port);
static void mkconnect(char *username, char *host, char *port);
static size_t request(char *hostname, unsigned short port, char *command, char *args, char **buffer);
static void usage(void);

static char *authtoken = NULL;
char *argv0;

#include "config.h"

static size_t
authorize(char *host, const char *port, char *username, char *password)
{
	size_t size;
	char *buffer, *args,
		 *duplicate, *tokenized;
	args = calloc(strlen(username) + strlen(password) + 2, 1);
	strcat(args, username); strcat(args, "\1");
	strcat(args, password); strcat(args, "\0");
	if (!(size = request(host, atoi(port), "auth", args, &buffer)))
		return 1;
	else {
		duplicate = strdup(truncateHeader(buffer));
		tokenized = strtok_r(duplicate, "\n", &duplicate);
		if (!strcmp(tokenized, "Authorized")) {
			authtoken = strtok_r(duplicate, "\n", &duplicate);
			return size;
		} else
			die("Permission denied: %s", tokenized);
	}
	return 0;
}

static int
command(char *command, char *args, char *host, char *port)
{
	char *buffer, *truncbuf;
	size_t reqsize;
	if (!(reqsize = request(host, atoi(port), command, args, &buffer)))
		return -1;
	truncbuf = truncateHeader(buffer);
	if (*truncbuf > 0 && *truncbuf < 10)
		++truncbuf; /* there will be steering sequences,
					   reserved for simple comunication server -> client */
	printf("%s%c", truncbuf,
			buffer[reqsize - 1] == '\n' || buffer[reqsize - 1]
			== 030 /* ASCII 030 on the end simply means:
					  PLZ DON'T INSERT ENDL ON THE END!!1!1!!1 */
			? '\0' : '\n');
	free(buffer);
	return 0;
}

static int
gameplay(char *username, char *host, char *port)
{
	size_t linesize = 256;
	char *line, *linedup, *token, *cmd,
		 *args;
	size_t argsize;
	int cash = 100;
	short health = 100, saturation = 100,
		  protection = 100, emotion = 100, hydration = 100;

	line = malloc(linesize);

	while (1) {
		argsize = 0;
		printf(PS1);
		getline(&line, &linesize, stdin);
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;
		linedup = strdup(line);
		cmd = strtok_r(linedup, " ", &linedup);
		args = calloc(0, 1);
		while ((token = strtok_r(linedup, " ", &linedup))) {
			argsize += strlen(token) + 1;
			args = realloc(args, argsize);
			strcat(args, token); strcat(args, "\001");
		}
		if(command(cmd, args, host, port))
			die("\033[1;31mSomething went wrong with your request!\033[0m");
	}
	free(line);
	return 0;
}

static void
mkconnect(char *username, char *host, char *port)
{
	int character, chiter = -1;
	char password[64] = "";

	fprintf(stderr, "Connecting to %s ", host);
	if (gethostbyname(host) == NULL) {
		herror("failed");
		exit(1);
	}

	fprintf(stderr, "succeeded!\n");
	fprintf(stderr, "%s@%s's password: \033[8m", username, host);
	while ((character = fgetc(stdin)) != '\n') {
		password[++chiter] = character;
	}
	password[++chiter] = 0;
	fprintf(stderr, "\033[0m");

	if(authorize(host, port, username, password))
		printf("Authorization successful!\nAuth token: %s\n", authtoken);
}

static size_t
request(char *hostname, unsigned short port,
		char *command, char *args, char **buffer)
{
	char *request_template = "GET /%s HTTP/1.0\r\nargv: %s\r\nAuth-Token: %s\r\nHost: %s\r\n\r\n";
	char request[BUFSIZ];
	size_t request_length;
	request_length = snprintf(request, BUFSIZ, request_template,
			command, args, authtoken, hostname);
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
	char *port = "80", *host = "localhost",
		 username[32] = "unknown";

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

	mkconnect(username, host, port);
	command("motd", "", host, port);
	gameplay(username, host, port);

	return 0;
}
