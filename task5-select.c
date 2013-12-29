#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int PORT = 8080; 

int main()
{
    fd_set master;
    fd_set read_fds;
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    int fdmax;
    int listener;
    int newfd;
    char buf[1024];
    int nbytes;
    int yes = 1;
    socklen_t addrlen;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

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

    FD_SET(listener, &master);
    fdmax = listener;

    while (true) {
        read_fds = master;
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    addrlen = sizeof(clientaddr);
                    newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen);
                    FD_SET(newfd, &master);
                    if(newfd > fdmax) {
                        fdmax = newfd;
                    }
                }
                else {
                    if((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else {
                        send(i, buf, nbytes, 0);
                    }
                }
            }
        }
    }
    return 0;
}
