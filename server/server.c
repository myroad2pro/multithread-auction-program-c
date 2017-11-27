#include <pthread.h>
#include "app.h"

void *thread_process(void *args);

/********************************** MAIN **************************************/
int main(int argc, char *argv[])
{
    int listen_sock, conn_sock; /* file descriptors */
    struct sockaddr_in server;  /* server's address information */
    struct sockaddr_in *client; /* client's address information */
    int sin_size;
    int serv_port;
    char *endptr;
    int number_of_users = 0;

    struct Session sessions[100];
    struct User users[100];
    struct Thread_Arguments *thread_args = (struct Thread_Arguments *)malloc(sizeof(struct Thread_Arguments));
    pthread_t tid;

    if ((number_of_users = load_users_data(users)) == -1)
    {
        return -1;
    }
    init_sessions(sessions);

    // Check terminal command arguments
    if (argc != 2)
    {
        printf("Invalid arguments\n");
        exit(-1);
    }
    serv_port = (in_port_t)strtol(argv[1], &endptr, 10);
    if (strlen(endptr) != 0)
    {
        printf("Invalid port!\n");
        exit(-1);
    }

    // Connect to client
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(serv_port);         /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    sin_size = sizeof(struct sockaddr_in);
    client = malloc(sin_size);
    // Communicate with clients
    while (1)
    {
        //accept request
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client->sin_addr)); /* prints client's IP */
        /* For each client, spawns a thread, and the thread handles the new client */
        *thread_args = (struct Thread_Arguments){.conn_sock = conn_sock, .sessions = sessions, .users = users, .number_of_users = &number_of_users};
        pthread_create(&tid, NULL, &thread_process, (void *)thread_args);
    }
    close(listen_sock);
    return 0;
}

/************************** FUNCTIONS DEFINITION ************************************/
void *thread_process(void *args)
{
    char *data;
    int current_session = -1;
    char reply_message[30];
    struct Thread_Arguments *thread_args = (struct Thread_Arguments *)args;
    //start conversation
    while (1)
    {
        // receive message from client
        if ((data = recv_msg(thread_args->conn_sock)))
        {
            printf("%s\n", data);
            // create new session or connect to previously established one
            current_session = create_session(thread_args->sessions, thread_args->conn_sock);
            if (current_session == -1)
            {
                strcpy(reply_message, "FF");
                send_msg(thread_args->conn_sock, reply_message);
                break;
            }
            handle_message(data, thread_args->sessions, current_session, thread_args->users, thread_args->number_of_users, reply_message);
        }
        else
        {
            break;
        }
        if (send_msg(thread_args->conn_sock, reply_message) == -1)
            break;
        if (save_users_data(thread_args->users, *(thread_args->number_of_users) ) == -1)
        {
            break;
        }
    }
    close(thread_args->conn_sock);
    return NULL;
}
