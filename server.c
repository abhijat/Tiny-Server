#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "strings.h"

const char* welcome_msg = "Welcome to a tiny server";

void usage(const char* pname)
{
    fprintf(stderr, "usage: %s <port>\n", pname);
}

void show_peer_address(const struct sockaddr_in* peer)
{
    char* ip = inet_ntoa(peer->sin_addr);
    printf("Peer IP is %s\n", ip);
    return;
}

void parse_header(const char* header_string)
{
    char* pos = strchr(header_string, ':');
    char* key = strndup(header_string, pos - header_string);
    if (key && pos)
        printf("%s is %s\n", key, pos + 2);
    free(key);
}

void process_request(const char* request)
{
    char* req = strdup(request);
    const char* ws = " \t";
    char* ctx;

    char* method = strtok_r(req, ws, &ctx);
    char* path = strtok_r(NULL, ws, &ctx);

    printf("Method %s on path %s\n", method, path);
    free(req);
}

void read_request_headers(int clientfd)
{
    char request[512];
    read(clientfd, &request, sizeof request - 1);

    const char* sep = "\r\n";
    char* ctx;

    char* line = strtok_r(request, sep, &ctx);

    process_request(line);

    while (line != NULL && strlen(line) > 0) {
        parse_header(line);
        line = strtok_r(NULL, sep, &ctx);
    }
    return;
}

void init_server_socket(struct sockaddr_in* ss, int port)
{
    bzero( (char*) ss, sizeof(*ss) );
    ss->sin_family = AF_INET;
    ss->sin_addr.s_addr = INADDR_ANY;
    ss->sin_port = htons(port);
}

void set_reusable(int sockfd)
{
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
}

char* build_response()
{
    char* response = malloc(sizeof(char) * (strlen(http_ok) + strlen(welcome_msg)));
    sprintf(response, http_ok, strlen(welcome_msg), welcome_msg);
    return response;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    init_server_socket(&serv_addr, atoi(argv[1]));

    set_reusable(sockfd);

    bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    listen(sockfd, 5);

    struct sockaddr_in cli_addr;
    unsigned cli_len = sizeof(cli_addr);

    for (;;) {
        unsigned cli_fd = accept(sockfd, (struct sockaddr*) &cli_addr, &cli_len);
        read_request_headers(cli_fd);

        char* response = build_response();
        write(cli_fd, response, strlen(response));
        free(response);

        close(cli_fd);
    }

    return 0;
}
