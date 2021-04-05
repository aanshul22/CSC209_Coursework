#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define BUF_SIZE 128

#define MAX_AUCTIONS 5
#ifndef VERBOSE
#define VERBOSE 0
#endif

#define ADD 0
#define SHOW 1
#define BID 2
#define QUIT 3

/* Auction struct - this is different than the struct in the server program
 */
struct auction_data {
    int sock_fd;
    char item[BUF_SIZE];
    int current_bid;
};

/* Displays the command options available for the user.
 * The user will type these commands on stdin.
 */

void print_menu() {
    printf("The following operations are available:\n");
    printf("    show\n");
    printf("    add <server address> <port number>\n");
    printf("    bid <item index> <bid value>\n");
    printf("    quit\n");
}

/* Prompt the user for the next command 
 */
void print_prompt() {
    printf("Enter new command: ");
    fflush(stdout);
}


/* Unpack buf which contains the input entered by the user.
 * Return the command that is found as the first word in the line, or -1
 * for an invalid command.
 * If the command has arguments (add and bid), then copy these values to
 * arg1 and arg2.
 */
int parse_command(char *buf, int size, char *arg1, char *arg2) {
    int result = -1;
    char *ptr = NULL;
    if(strncmp(buf, "show", strlen("show")) == 0) {
        return SHOW;
    } else if(strncmp(buf, "quit", strlen("quit")) == 0) {
        return QUIT;
    } else if(strncmp(buf, "add", strlen("add")) == 0) {
        result = ADD;
    } else if(strncmp(buf, "bid", strlen("bid")) == 0) {
        result = BID;
    } 
    ptr = strtok(buf, " "); // first word in buf

    ptr = strtok(NULL, " "); // second word in buf
    if(ptr != NULL) {
        strncpy(arg1, ptr, BUF_SIZE);
    } else {
        return -1;
    }
    ptr = strtok(NULL, " "); // third word in buf
    if(ptr != NULL) {
        strncpy(arg2, ptr, BUF_SIZE);
        return result;
    } else {
        return -1;
    }
    return -1;
}

/* Connect to a server given a hostname and port number.
 * Return the socket for this server
 */
int add_server(char *hostname, int port) {
        // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }
    
    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    struct addrinfo *ai;
    
    /* this call declares memory and populates ailist */
    if(getaddrinfo(hostname, NULL, NULL, &ai) != 0) {
        close(sock_fd);
        return -1;
    }
    /* we only make use of the first element in the list */
    server.sin_addr = ((struct sockaddr_in *) ai->ai_addr)->sin_addr;

    // free the memory that was allocated by getaddrinfo for this list
    freeaddrinfo(ai);

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        return -1;
    }
    if(VERBOSE){
        fprintf(stderr, "\nDebug: New server connected on socket %d.  Awaiting item\n", sock_fd);
    }
    return sock_fd;
}
/* ========================= Add helper functions below ========================
 * Please add helper functions below to make it easier for the TAs to find the 
 * work that you have done.  Helper functions that you need to complete are also
 * given below.
 */

/* Print to standard output information about the auction
 */
void print_auctions(struct auction_data *a, int size) {
    printf("Current Auctions:\n");

    /* TODO Print the auction data for each currently connected 
     * server.  Use the follosing format string:
     *     "(%d) %s bid = %d\n", index, item, current bid
     * The array may have some elements where the auction has closed and
     * should not be printed.
     */
    for (int i = 0; i < size; ++i) 
	{
		if (a[i].sock_fd != -1)
		{
			printf("(%d) %s bid = %d\n", i, a[i].item, a[i].current_bid);
		}
	}
    
}

/* Process the input that was sent from the auction server at a[index].
 * If it is the first message from the server, then copy the item name
 * to the item field.  (Note that an item cannot have a space character in it.)
 */
void update_auction(char *buf, int size, struct auction_data *a, int index) {
    
    // TODO: Complete this function

	char *ptr = NULL;

	// first word in buf - item
	ptr = strtok(buf, " ");
	if (ptr == NULL) {
		fprintf(stderr, "ERROR malformed bid: %s", buf);
		return;
	}
	if (a[index].item[0] == '\0') { // If we don't have item name, add it
		strncpy(a[index].item, ptr, BUF_SIZE);
	}

	// second word in buf - bid value
	ptr = strtok(NULL, " ");
	if (ptr != NULL){ // Setting current bid value
		a[index].current_bid = strtol(ptr, NULL, 10);
	} 
	else {
		fprintf(stderr, "ERROR malformed bid: %s", buf);
		return;
	}

	// third word in buf - time left
	int time_left = 0;
	ptr = strtok(NULL, " ");
	if (ptr != NULL) {
		time_left = strtol(ptr, NULL, 10);
	} 
	else {
		fprintf(stderr, "ERROR malformed bid: %s", buf);
		return;
	}

	printf("\nNew bid for %s [%d] is %d (%d seconds left)\n", a[index].item, 
															  index, 
															  a[index].current_bid, 
															  time_left);
}

/* Closes connections to all auctions by closing all the open sockets.
*/
void close_all_sockets(struct auction_data *a, int size) {
	for (int i = 0; i < size; ++i)
	{
		if (a[i].sock_fd != - 1)
		{
			close(a[i].sock_fd);
		}
	}
}

int main(void) {

    char name[BUF_SIZE];

    // Declare and initialize necessary variables
    // TODO
	int fd;
	int port;
	int command;

	char buf[BUF_SIZE];
	char arg1[BUF_SIZE];
	char arg2[BUF_SIZE];

	struct auction_data auctions[MAX_AUCTIONS];
	for (int index = 0; index < MAX_AUCTIONS; index++)
	{
		auctions[index].sock_fd = -1;
		auctions[index].item[0] = '\0';
		auctions[index].current_bid = -1;
	}

	// Get the user to provide a name.
	printf("Please enter a username: ");
	fflush(stdout);
    int num_read = read(STDIN_FILENO, name, BUF_SIZE);
    if(num_read <= 0){
        fprintf(stderr, "ERROR: read from stdin failed\n");
        exit(1);
    }
	name[num_read] = '\0';
	print_menu();

    // Initializing variables for select call
	int max_fd = STDIN_FILENO;
	fd_set all_fds;
	FD_ZERO(&all_fds);
	FD_SET(STDIN_FILENO, &all_fds);

	while(1) {
        print_prompt();

		fd_set listen_fds = all_fds;
		if (select(max_fd + 1, &listen_fds, NULL, NULL, NULL) == -1) {
			perror("server: select");
			exit(1);
		}

		// Checking for Standard input
		if (FD_ISSET(STDIN_FILENO, &listen_fds)) {
			// Reading input into buf
			if ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) == 0) {
				break;
			}
			buf[num_read] = '\0';

			command = parse_command(buf, BUF_SIZE, arg1, arg2);

			// Checking which command was called
			if (command == SHOW) {
				print_auctions(auctions, MAX_AUCTIONS);
			}
			else if (command == ADD) {
				// Find the next empty slot
				int i = 0;
				while (auctions[i].sock_fd != -1) { i++; }

				port = strtol(arg2, NULL, 10); // Converting port to integer

				auctions[i].sock_fd = add_server(arg1, port);

				// Add server's fd to all_fds so that select can look for it
				FD_SET(auctions[i].sock_fd, &all_fds);
				// Update max_fd
				if (auctions[i].sock_fd > max_fd) {
					max_fd = auctions[i].sock_fd;
				}

				// Write client name to server
				if (write(auctions[i].sock_fd, name, strlen(name) + 1) != strlen(name) + 1) {
					perror("Client: write name");
					close(auctions[i].sock_fd);
					exit(1);
				}
			}
			else if (command == BID) {
				// fd of auction bidding to
				fd = auctions[strtol(arg1, NULL, 10)].sock_fd;
				if (fd == -1) {
					printf("There is no auction open at %ld\n", strtol(arg1, NULL, 10));
				}
				else {
					// Write bid value to server
					if (write(fd, arg2, strlen(arg2) + 1) != strlen(arg2) + 1) {
						perror("client: write");
						close(auctions[strtol(arg1, NULL, 10)].sock_fd);
						exit(1);
					}
				}
			}
			else if (command == QUIT) {
				close_all_sockets(auctions, MAX_AUCTIONS);
				exit(0);
			}
			else { // If not a valid command
				print_menu();
			}
		}
		
		// Next, check all servers
		for (int index = 0; index < MAX_AUCTIONS; ++index)
		{
			// If connected to server and server has sent something
			if (auctions[index].sock_fd != -1 && FD_ISSET(auctions[index].sock_fd, &listen_fds))
			{
				// Reading from server 
				num_read = read(auctions[index].sock_fd, &buf, BUF_SIZE);
				if (num_read == 0) { // If server has disconnected
					close(auctions[index].sock_fd);
					FD_CLR(auctions[index].sock_fd, &all_fds);
					auctions[index].sock_fd = -1;
				}

				buf[num_read] = '\0';
				// Checking if auction has been closed
				if (strncmp(buf, "Auction closed", strlen("Auction closed")) == 0)
				{
					printf("%s", buf);
					close(auctions[index].sock_fd);
					FD_CLR(auctions[index].sock_fd, &all_fds);
					auctions[index].sock_fd = -1;
				}
				else { // server sent: item bid_value time_left
					update_auction(buf, strlen(buf), auctions, index);
				}
			}
		}
	}
    return 0; // Shoud never get here
}
