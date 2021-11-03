#include <stdio.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void process_one_connect(int);
void startServer(int, char**);
void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg);
int read_requesthdrs(rio_t *rp, String *requesthdrs, char *host);
void get_port(char *host, char *port);



/*
 * process one connect.
 */
void process_one_connect(int connfd) {

    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio, rio_host;
    int host_fd;

    /* Read request line and headers */
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
        return;
    // printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(connfd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }

    String requesthdrs;
    char host[MAXLINE];
    initString(&requesthdrs);

    if (read_requesthdrs(&rio, &requesthdrs, host))
        goto end;

    /* 解析host和端口号 */
    char port[MAXLINE];
    get_port(host, port);

    /* 建立链接 */
    host_fd = Open_clientfd(host, port);

    // 处理uri的内容
    char *new_uri = uri;
    if (strstr(uri, "//")) new_uri = strstr(uri, "//") + 2;
    else new_uri = uri;

    if (strchr(new_uri, '/')) new_uri = strchr(new_uri, '/');
    else new_uri = NULL;


    // 发送请求头和请求体
    sprintf(buf, "%s %s %s\r\n", method, new_uri ? new_uri : "/", "HTTP/1.0");
    printf("%s", buf);
    Rio_writen(host_fd, buf, strlen(buf));
    // printf("%s", requesthdrs.data_);
    Rio_writen(host_fd, requesthdrs.data_, requesthdrs.size_);

    // 接收服务端发送过来的数据
    Rio_readinitb(&rio_host, host_fd);
    ssize_t readn;
    String host_cont;
    initString(&host_cont);

    while ((readn = Rio_readnb(&rio_host, buf, MAXLINE)) != 0) {
        if (readn < 0) {
            printf("Read Content from Host Error!\n");
            break;
        }
        // printf("%d\n", readn);
        append(&host_cont, buf, (size_t)readn);
    }
    // endString(&host_cont);
    Close(host_fd);

    // printf("||||||||||\n");

    // 将从host接收的数据全部发送给client
    // printf("%s", host_cont.data_);
    printf("%d\n", host_cont.size_);

    Rio_writen(connfd, host_cont.data_, host_cont.size_);
    Close(connfd);

end:
    freeString(&requesthdrs);
    freeString(&host_cont);

}


void startServer(int argc, char* argv[]) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
	process_one_connect(connfd);                                             //line:netp:tiny:doit

    }
}

void clienterror(int fd, char *cause, char *errnum,
		 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

/*
 * read_requesthdrs - read HTTP request headers
 * Replace the content.
 */
/* $begin read_requesthdrs */
int read_requesthdrs(rio_t *rp, String *requesthdrs, char *host)
{
    char buf[MAXLINE];
    char *ignore_headers[] = {"User-Agent", "Conection", "Proxy-Connection"};
    int has_host = 0;

    while (1) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
        if (strcmp(buf, "\r\n") == 0) break;
        char *idx = strchr(buf, ':');
        if (idx == NULL) {
            printf("Parser Header Error!\n");
            return -1;
        }
        *idx = '\0';
        if (strcmp(buf, "Host") == 0) {
            has_host = 1;
            char *cur_idx = idx + 1;
            // Copy host.
            while (*cur_idx == ' ') ++cur_idx;
            while (*cur_idx != '\r') *host++ = *cur_idx++;
            *host = '\0';
        }
        for (int i = 0; i < 3; ++i) {
            if (strcmp(buf, ignore_headers[i]) == 0) goto ignore;
        }
        *idx = ':';
        append(requesthdrs, buf, strlen(buf));
        ignore:
            continue;
    }

    // 如果没有host的话，则直接返回错误
    if (!has_host) return -1;

    // 加入特定的header
    append(requesthdrs, user_agent_hdr, strlen(user_agent_hdr));
    append(requesthdrs, "Connection: close\r\n", 19);
    append(requesthdrs, "Proxy-Connection: close\r\n", 25);
    append(requesthdrs, "\r\n", 2);
    // endString(requesthdrs);
    return 0;
}

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */ //line:netp:parseuri:isstatic
	strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
	strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1
	strcat(filename, uri);                           //line:netp:parseuri:endconvert1
	if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
	    strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
	return 1;
    }
    else {  /* Dynamic content */                        //line:netp:parseuri:isdynamic
	ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	}
	else
	    strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
	strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
	strcat(filename, uri);                           //line:netp:parseuri:endconvert2
	return 0;
    }
}
/* $end parse_uri */


void get_port(char *host, char *port) {
    char *idx = strchr(host, ':');
    if (idx != NULL) {
        *idx = '\0';    // delete port from host
        ++idx;
        while (*idx) *port++ = *idx++;  // copy port
        *port = '\0';
    } else {
        // default http port 80
        *port++ = '8';
        *port++ = '0';
        *port = '\0';
    }
}


int main(int argc, char* argv[])
{
    startServer(argc, argv);
    return 0;
}
