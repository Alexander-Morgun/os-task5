#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

const int PORT = 8080; 

int main()
{
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    char buf[1024];
    int nbytes;
    int listener;
    int yes = 1;
    socklen_t addrlen;
    struct pollfd users[1024];
    nfds_t n_users = 0;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        exit(1);
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);

    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        exit(1);
    }

    if (listen(listener, 10) == -1) {
         exit(1);
    }

    users[0].fd = listener;
    users[0].events = POLLIN;
    n_users = 1;

    while (true) {
        int ret = poll(users, n_users,-1);
        for (int i = n_users - 1; i >= 0 && ret > 0; --i) {
            if (users[i].revents & POLLIN) {
                if (i == 0) { 
                    addrlen = sizeof(clientaddr);
                    users[n_users].fd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen);
                    users[n_users].events = POLLIN | POLLOUT | POLLRDHUP;
                    ++n_users;
                }
                else {
                    nbytes = recv(users[i].fd, buf, sizeof(buf), 0);
                    if (users[i].revents & POLLOUT) {
                        send(users[i].fd, buf, nbytes, 0);
                    }
                }
            }
            if (users[i].revents & (POLLHUP | POLLRDHUP | POLLERR | POLLRDNORM)) {
                --n_users;
                close(users[i].fd); 
                users[i].fd = users[n_users].fd;
            }
        }
    }
    return 0;
}
