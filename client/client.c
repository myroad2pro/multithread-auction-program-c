#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 8196
#define WINDOW_SIZE 1024

void welcome();
void print_server_message(char *buff);
char *recv_msg(int conn_sock);
int send_msg(int conn_sock, char *message, int msg_len);

int main(int argc, char *argv[])
{
    int client_sock;
    char buff[BUFF_SIZE], *msg;
    struct sockaddr_in server_addr; /* server's address information */
    int serv_port = 0;
    char serv_ip[16], *endptr;
    ;
    int msg_len;

    // Step 1: Get command from terminal
    if (argc != 3)
    {
        printf("Invalid arguments!\n");
        exit(-1);
    }

    strcpy(serv_ip, argv[1]);
    serv_port = (in_port_t)strtol(argv[2], &endptr, 10);
    if (strlen(endptr) != 0)
    {
        printf("Invalid port!\n");
        exit(-1);
    }

    //Step 2: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    //Step 3: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serv_port);
    server_addr.sin_addr.s_addr = inet_addr(serv_ip);

    //Step 4: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }

    //Step 5: Communicate with server
    while (1)
    {
        welcome();
        memset(buff, '\0', strlen(buff));
        fgets(buff, BUFF_SIZE, stdin);
        msg_len = strlen(buff);
        if (buff[msg_len - 1] == '\n')
            buff[msg_len - 1] = '\0';
        if (strlen(buff) == 0)
            break;
        printf("%s\n", buff);
        if(strcmp(buff, "EXIT") == 0) break;
        send_msg(client_sock, buff, strlen(buff));
        msg = recv_msg(client_sock);
        print_server_message(msg);
    }
    //Step 4: Close socket
    close(client_sock);
    return 0;
}

void welcome()
{
    printf("TCP Auction Management Application\n");
    printf("Nguyen Hoang Anh - 20130135 & Dinh Trong Thang - 2013????\n");
    printf("SYNTAX:\n");
    printf("Log In: LOGIN username password\n");
    printf("Sign Up: SIGNUP username password\n");
    printf("Log out: LOUT username password\n");
    printf("Exit: EXIT\n");
    printf("Your command: ");
}

void print_server_message(char *message)
{
    if (strcmp(message, "00") == 0)
    {
        printf("Log In Successful.\n\n");
    }
    else if (strcmp(message, "01") == 0)
    {
        printf("Wrong username!!!\n\n");
    }
    else if (strcmp(message, "10") == 0)
    {
        printf("Log Out Successful.\n\n");
    }
    else if (strcmp(message, "11") == 0)
    {
        printf("Wrong username. Try again.\n\n");
    }
    else if (strcmp(message, "20") == 0)
    {
        printf("Sign up successfully.\n\n");
    }
    else if (strcmp(message, "21") == 0)
    {
        printf("User already exists!!!\n\n");
    }
    else if (strcmp(message, "22") == 0)
    {
        printf("Password must have at least 8 characters!!!\n\n");
    }
    else if (strcmp(message, "F0") == 0)
    {
        printf("Wrong password, try again.\n\n");
    }
    else if (strcmp(message, "F1") == 0)
    {
        printf("User is blocked!!!\n\n");
    }
    else if (strcmp(message, "F2") == 0)
    {
        printf("INVALID COMMAND!!!\n\n");
    }
    else if (strcmp(message, "FF") == 0)
    {
        printf("No socket available!!!\n\n");
    }
}

char *recv_msg(int conn_sock)
{
    int ret, nLeft, msg_len, index = 0;
    char recv_data[WINDOW_SIZE], *data;
    // receive the length of message
    int bytes_received = recv(conn_sock, &msg_len, sizeof(int), MSG_WAITALL);
    if (bytes_received <= 0)
    {
        return NULL;
    }
    nLeft = msg_len;
    data = (char *)malloc(msg_len);
    memset(data, 0, msg_len);
    index = 0;

    //receives message from server
    while (nLeft > 0)
    {
        ret = recv(conn_sock, recv_data, WINDOW_SIZE, 0);
        if (ret == -1)
        {
            break;
        }
        memcpy(data + index, recv_data, ret);
        index += ret;
        nLeft -= ret;
    }
    data[msg_len] = '\0';
    return data;
}

int send_msg(int conn_sock, char *message, int msg_len)
{
    int bytes_sent;
    //send the length of the message to server
    bytes_sent = send(conn_sock, &msg_len, sizeof(int), 0);
    if (bytes_sent <= 0)
    {
        return -1;
    }

    // send the message to server
    bytes_sent = send(conn_sock, message, msg_len, 0);
    if (bytes_sent <= 0)
    {
        return -1;
    }
    return 0;
}
