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

char* file_to_str(const char* path);

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
    //if (key && pos)
    //    printf("%s is %s\n", key, pos + 2);
    free(key);
}

char* process_request(const char* request)
{
    char* req = strdup(request);
    const char* ws = " \t";
    char* ctx;

    char* method = strtok_r(req, ws, &ctx);
    char* path = strtok_r(NULL, ws, &ctx);

    char* ret = strdup(path);

    printf("Method %s on path %s\n", method, path);
    free(req);

    return ret;
}

char* read_request_headers(int clientfd)
{
    char request[512];
    read(clientfd, &request, sizeof request - 1);

    const char* sep = "\r\n";
    char* ctx;

    char* line = strtok_r(request, sep, &ctx);

    char* r = process_request(line);

    while (line != NULL && strlen(line) > 0) {
        parse_header(line);
        line = strtok_r(NULL, sep, &ctx);
    }
    return r;
}

char* response(const char* req)
{
    char* response;
    if (req[0] != '/') {
        response = strdup("error, incorrect filename");
    } else if (strcmp(req, "/") == 0) {
        response = strdup(welcome_msg);
    } else {
        response = file_to_str(req + 1);
    }
    return response;
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

char* build_response(char* content)
{
    char* response = malloc(sizeof(char) * (strlen(http_ok) + strlen(content)));
    sprintf(response, http_ok, strlen(content), content);
    return response;
}

char* prettify(char* str)
{
    size_t len = strlen(str);
    size_t newlines = 0;

    for (size_t i = 0; i <= len; ++i) {
        if (str[i] == '\n') ++newlines;
    }

    char* p = (char*) malloc(sizeof(char) * (len + (3 * newlines)));

    size_t j = 0;
    for (size_t i = 0; i <= len; ++i) {
        if (str[i] == '\n') {
            p[j++] = '<';
            p[j++] = 'b';
            p[j++] = 'r';
            p[j++] = '>';
        } else {
            p[j++] = str[i];
        }
    }
    p[j] = 0;
    return p;
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
        char* r = read_request_headers(cli_fd);
        char* resp = response(r);
        free(r);
       
        char* p = prettify(resp);
        free(resp);

        char* response = build_response(p);
        write(cli_fd, response, strlen(response));

        free(response);
        close(cli_fd);
    }

    return 0;
}
