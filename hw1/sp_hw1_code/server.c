#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>

#define ERR_EXIT(a) { perror(a); exit(1); }

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
	int item;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

// Forwards

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

static int handle_read(request* reqP);
// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error

typedef struct {
    int id;
    int amount;
    int price;
} Item;

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
    struct flock lock;

    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;

    int res = fcntl(fd, cmd, &lock);

    if (cmd == F_SETLK)
        return res;

    return lock.l_type != F_UNLCK;
}

#define write_lock(fd, offset, whence, len) \
        lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))

#define un_lock(fd, offset, whence, len) \
        lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

#define has_lock(fd, offset, whence, len) \
        lock_reg((fd), F_GETLK, F_WRLCK, (offset), (whence), (len))

int main(int argc, char** argv) {
    int i, ret;

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;

    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }

    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Get file descripter table size and initize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // "struct" deleted
    fd_set master_set, working_set;
    FD_ZERO(&master_set);

    // <0: closed
    // =0: waiting for 1st input
    // >0: waiting for 2nd input, also means item_id
    int *fd_table = (int*) malloc(sizeof(int) * maxfd);
    for (i = 0; i < maxfd; i++)
        fd_table[i] = -1;
    int fd_maxnum = -1;
    int fd_count = 0;
    int item_id;

    int write_lock_table[20] = {0};

    off_t offset;
    int whence;
    off_t len;

    Item item;
    file_fd = open("item_list", O_RDWR);

    while (1) {

        FD_ZERO(&working_set);
        FD_SET(svr.listen_fd, &working_set);
        int svr_res = select(svr.listen_fd + 1, &working_set, NULL, NULL, &timeout);
        if (svr_res > 0) {

            // Check new connection
            clilen = sizeof(cliaddr);
            conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
            
            // can be deleted?
            if (conn_fd < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;  // try again
                if (errno == ENFILE) {
                    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                    continue;
                }
                ERR_EXIT("accept")
            }
            requestP[conn_fd].conn_fd = conn_fd;
            strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
            fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
            
            FD_SET(conn_fd, &master_set);
            fd_table[conn_fd] = 0;
            ++fd_count;

            continue;
        }
        else if (svr_res < 0) {
            printf("server select error\n");
            break;
        }

        // now there's no new connection
        if (fd_count == 0)
            continue;
        fd_maxnum = -1;
        memcpy(&working_set, &master_set, sizeof(master_set));
        for (i = 0; i < maxfd; i++)
            if (fd_table[i] >= 0) // waiting for input
                fd_maxnum = i;
        int cli_res = select(fd_maxnum + 1, &working_set, NULL, NULL, &timeout);
        if (cli_res > 0) {
            for (conn_fd = 0; conn_fd < maxfd; conn_fd++)
                if (FD_ISSET(conn_fd, &working_set))
                    break;

            // now conn_fd is ready for reading

#ifdef READ_SERVER
            goto read_flag;
#else
            if (fd_table[conn_fd] == 0)
                goto read_flag;
            else
                goto second_read_flag;
#endif
        }
        else if (cli_res < 0) {
            printf("client select error, errno: %d\n", errno);
            break;
        }
        else
            continue;

read_flag:

        ret = handle_read(&requestP[conn_fd]); // parse data from client to requestP[conn_fd].buf
        if (ret < 0) {
            fprintf(stderr, "bad request from %s\n", requestP[conn_fd].host);
            continue;
        }

        item_id = atoi(requestP[conn_fd].buf);
        if (item_id < 1 || item_id > 20) {
            dprintf(requestP[conn_fd].conn_fd, "Error: id should be in [1, 20].\n");
            goto continue_flag;
        }

        len = sizeof(Item);
        offset = len * (item_id - 1);
        whence = SEEK_SET;

#ifdef READ_SERVER

        // check lock
        if (has_lock(file_fd, offset, whence, len)) {
            dprintf(requestP[conn_fd].conn_fd, "This item is locked.\n");
            goto continue_flag;
        }

        pread(file_fd, &item, len, offset);
        
        dprintf(requestP[conn_fd].conn_fd, "item%d $%d remain: %d\n", item.id, item.price, item.amount);

#else
        // check lock
        if (write_lock_table[item_id] == 1 || 
            has_lock(file_fd, offset, whence, len)) {
            dprintf(requestP[conn_fd].conn_fd, "This item is locked.\n");
            goto continue_flag;
        }

        // add lock
        write_lock_table[item_id] = 1;
        write_lock(file_fd, offset, whence, len);

        dprintf(requestP[conn_fd].conn_fd, "This item is modifiable.\n");
        fd_table[conn_fd] = item_id;
        continue;

second_read_flag:

        item_id = fd_table[conn_fd];
        
        offset = sizeof(Item) * (item_id - 1);
        whence = SEEK_SET;
        len = sizeof(Item);

        handle_read(&requestP[conn_fd]);
        if (ret < 0) {
            fprintf(stderr, "bad request from %s\n", requestP[conn_fd].host);
            continue;
        }

        pread(file_fd, &item, len, offset);

        char *cmd = strtok(requestP[conn_fd].buf, " ");
        int number = atoi(strtok(NULL, " "));

        if (strcmp(cmd, "buy") == 0) {
            if (item.amount < number) {
                dprintf(requestP[conn_fd].conn_fd, "Operation failed.\n");
            }
            else
                item.amount -= number;
        }
        else if (strcmp(cmd, "sell") == 0) {
            item.amount += number;
        }
        else if (strcmp(cmd, "price") == 0) {
            if (number < 0) {
                dprintf(requestP[conn_fd].conn_fd, "Operation failed.\n");
            }
            else
                item.price = number;
        }
        else {
            dprintf(requestP[conn_fd].conn_fd, "Error: invalid argument.\n");
        }

        // unlock
        write_lock_table[item_id] = 0;
        un_lock(file_fd, offset, whence, len);

        pwrite(file_fd, &item, len, offset);

#endif

continue_flag:

        close(requestP[conn_fd].conn_fd);
        free_request(&requestP[conn_fd]);

        FD_CLR(conn_fd, &master_set);
        fd_table[conn_fd] = -1;
        --fd_count;
    }
    
    close(file_fd);
    free(requestP);
    return 0;
}


// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->item = 0;
    reqP->wait_for_write = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error
static int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
	char* p1 = strstr(buf, "\015\012");
	int newline_len = 2;
	// be careful that in Windows, line ends with \015\012
	if (p1 == NULL) {
		p1 = strstr(buf, "\012");
		newline_len = 1;
		if (p1 == NULL) {
			ERR_EXIT("this really should not happen...");
		}
	}
	size_t len = p1 - buf + 1;
	memmove(reqP->buf, buf, len);
	reqP->buf[len - 1] = '\0';
	reqP->buf_len = len-1;
    return 1;
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }
}
