#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

// GLOBAL VARIABLES, STRUCTS, ETC...
#define BACKLOG 100 /* Number of allowed connections */
#define BUFF_SIZE 1024

enum session_status
{
    NOT_CONNECTED = -1,
    NOT_IDENTIFIED_USER = 1,
    NOT_AUTHENTICATED = 2,
    AUTHENTICATED = 4,
    IN_ROOM = 8
};

enum account_status
{
    NOT_FOUND = -1,
    BLOCKED = 0,
    ACTIVE = 1,
};

enum command_status
{
    INVALID_COMMAND = -2,   // invalid command
    SUCCESS = 1,    // command success
    LOGIN = 2,      // log in
    LOUT = 3,       // log out
    SIGNUP = 4,       // sign up
    CRRM = 5,       // create room
    JOIN = 6,       // join room
    CRPD = 7,       // create product 
    AUCP = 8,       // auction product
    LEAVE = 9,      // leave room
    SBUY = 10,      // instant buy
    DELR = 11,      // delete room
    DELP = 12       // delete product
};

enum product_status{
    NOT_IDENTIFIED = -1,
    NOT_AUCTION = 0,
    IN_AUCTION = 1,
    SOLD = 2
};

typedef struct User
{
    char id[30];
    char pass[30];
    int accStatus; //0 : blocked, 1: active
} User;

typedef struct Product
{
    char productName[30];
    char productDescription[100];
    struct User seller;
    struct User currentBidder;
    int currentHighestPrice;
    int productStatus;
};

typedef struct ProductNode
{
    struct Product product;
    struct ProductNode *next_Product;
};

typedef struct Room
{
    struct ProductNode *queue;
    int room_number;
};

typedef struct Session
{
    struct User user;
    int sessStatus;
    int conn_sock;
    int number_of_retries;
} Session;

typedef struct Thread_Arguments
{
    int conn_sock;
    struct Session *sessions;
    struct User *users;
    int *number_of_users;
} Thread_Arguments;

/*************************** FUNCTION DECLARATIONS *****************************/
char *recv_msg(int conn_sock);                               // receive messages from client
int send_msg(int conn_sock, char *message);                  // send messages to client
int load_users_data(struct User *users);                     // load users data from file
void init_sessions(struct Session *sessions);                // initialize sessions
int create_session(struct Session *sessions, int conn_sock); // create a new session or connect to previously connected one
void handle_message(char *recv_msg, struct Session *sessions, int current_session, struct User *users, int *number_of_users, char *reply_message);
int detect_message(char *token);

int login_find_user_id(struct Session *sessions, int current_session, struct User *users, int number_of_users, char *user_id);
int check_user_status(struct User user);
void change_session_status_correct_userid(struct Session *sessions, int current_session, struct User user);

int login_check_password(struct Session *sessions, int current_session, struct User *users, int number_of_users, char *password);
void change_session_status_correct_password(struct Session *sessions, int current_session);
void block_user_id(struct Session *sessions, int current_session, struct User *users, int number_of_users);

int logout_find_user_id(struct Session *sessions, int current_session, char *user_id);
void change_session_status_log_out(struct Session *sessions, int current_session);
int logout_check_password(Session *sessions, int current_session, User *users, int number_of_users, char *password);

int signup_find_user_id(Session *sessions, int current_session, User *users, int number_of_users, char *user_id);
int signup_password(Session *sessions, int current_session, User *users, int *number_of_users, char *user_id, char *password);
void signup_correct_password(Session *sessions, int current_session, User user);

int save_users_data(struct User *users, int number_of_users);
