main(int argc, char **argv) {
    int listenfd, connfd, udpfd, nready, maxfdp1;
    char buffTCP[DIM_BUFF], buffUDP[DIM_BUFF];
    fd_set rset;
    int n, len, nread, nwrite;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;
    void gestore(int signo) {
        int stato;
        printf("esecuzione gestore di SIGCHLD\n");
        wait(&stato);
    }
/* creazione socket d’ascolto */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("apertura socket d’ascolto");
        exit(1);
    }
    printf("Server: creata la socket d’ascolto fd=%d\n", listenfd);
/* inizializzazione indirizzo server */
    memset((char *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = 12345;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("settaggio opzioni socket d’ascolto");
        exit(1);
    }
    printf("Server: settaggio opzioni socket d’ascolto ok\n");
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind socket d’ascolto");
        exit(1);
    }
    printf("Server: bind socket d’ascolto ok\n");
    if (listen(listenfd, 5) < 0) {
        perror("listen");
        exit(1);
    }
    printf("Server: listen ok\n");
/* creazione socket UDP */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    4if(udpfd < 0)
    {
        perror("apertura socket UDP");
        exit(1);
    }
    printf("Server: creata la socket UDP fd=%d\n", udpfd);
/* inizializzazione indirizzo server */
    memset((char *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = 12345;
    if (bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind socket udp");
        exit(1);
    }
    printf("Server: bind socket udp ok\n");
    sigset(SIGCHLD, gestore);
    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd) + 1;
    for (;;) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        if ((nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) continue;
            else {
                perror("select");
                exit(1);
            }
        }
        if (FD_ISSET(listenfd, &rset)) { /* richiesta proveniente da client TCP */
            len = sizeof(cliaddr);
            if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len)) < 0) {
                if (errno == EINTR) continue;
                else {
                    perror("accept");
                    exit(1);
                }
            }
            if (fork() == 0) {
                close(listenfd);
                if (nread = read(connfd, buffTCP, DIM_BUFF) < 0) {
                    perror("read");
                    exit(1);
                }
                printf("Server: stringa ricevuta da client TCP -> %s\n", buffTCP);
                if (nwrite = write(connfd, buffTCP, DIM_BUFF) < 0) {
                    perror("write");
                    exit(1);
                }
                exit(0);
            }
            close(connfd);
        } /* if TCP */
        if (FD_ISSET(udpfd, &rset)) { /* richiesta proveniente da client UDP */
            len = sizeof(cliaddr);
            if ((n = recvfrom(udpfd, buffUDP, DIM_BUFF, 0, (struct sockaddr *) &cliaddr,
                              &len)) < 0) {
                perror("recvfrom");
                exit(1);
            }
            printf("Server: stringa ricevuta da client UDP -> %s\n", buffUDP);
            if (sendto(udpfd, buffUDP, n, 0, (struct sockaddr *) &cliaddr, len) < 0) {
                perror("sendto");
                exit(1);
            }
        } /* if UDP */
    } /* for */
}