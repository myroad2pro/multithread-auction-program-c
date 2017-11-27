#include "app.h"

char *recv_msg(int conn_sock)
{
    int ret, nLeft, index = 0;
    char recv_data[BUFF_SIZE], *data;
    int msg_len;

    // receive the length of message
    int bytes_received = recv(conn_sock, &msg_len, sizeof(int), MSG_WAITALL);
    if (bytes_received <= 0)
    {
        return NULL;
    }

    if (msg_len > 0)
    {
        nLeft = msg_len;
        data = (char *)malloc(msg_len * sizeof(char));
        memset(data, 0, msg_len);
        index = 0;

        //receives message from client
        while (nLeft > 0)
        {
            ret = recv(conn_sock, recv_data, BUFF_SIZE, 0);
            if (ret == -1)
            {
                return NULL;
            }
            memcpy(data + index, recv_data, ret);
            index += ret;
            nLeft -= ret;
        }
        data[msg_len] = '\0';
    }
    return data;
}

int send_msg(int conn_sock, char *message)
{
    int msg_len, bytes_sent;
    //send the length of the message to client
    msg_len = strlen(message);
    bytes_sent = send(conn_sock, &msg_len, sizeof(int), 0);
    if (bytes_sent <= 0)
    {
        return -1;
    }

    // send message to client
    bytes_sent = send(conn_sock, message, msg_len, 0);
    if (bytes_sent <= 0)
    {
        return -1;
    }
    return 0;
}

// load user data from account file
int load_users_data(struct User *users)
{
    FILE *fp = NULL;
    char line[100];
    char *user_id, *password;
    int status = NOT_FOUND, index = 0;
    if ((fp = fopen("account.txt", "r")) == NULL)
    {
        printf("Cannot open account file\n");
        return -1;
    }
    while (fgets(line, 100, fp))
    {
        // read user information from file
        user_id = strtok(line, " ");
        if(user_id == NULL) break;
        password = strtok(NULL, " ");
        if(password == NULL) break;
        status = atoi(strtok(NULL, " "));
        if(status == 0) break;
        // store them in users[] array
        strcpy(users[index].id, user_id);
        strcpy(users[index].pass, password);
        users[index].accStatus = status;
        //printf("USER ID: %s, PASS: %s, STATUS: %d\n", users[index].id, users[index].pass, users[index].accStatus );
        index++;
        memset(line, 0, 100);
    }
    return index;
}

// initialize sessions
void init_sessions(struct Session *sessions)
{
    int i = 0;
    for (i = 0; i < 100; i++)
    {
        sessions[i].sessStatus = NOT_CONNECTED;
        sessions[i].conn_sock = -1;
        sessions[i].number_of_retries = 0;
    }
}

// return index of connected socket or create a new session if socket not connected
int create_session(struct Session *sessions, int conn_sock)
{
    int i = 0;
    for (i = 0; i < 100; i++)
    {
        if (sessions[i].conn_sock == conn_sock)
        {
            printf("\nConnected socket\n");
            return i;
        }
    }

    for (i = 0; i < 100; i++)
    {
        if (sessions[i].sessStatus == NOT_CONNECTED)
        {
            strcpy(sessions[i].user.id, "");
            strcpy(sessions[i].user.pass, "");
            sessions[i].user.accStatus = NOT_FOUND;
            sessions[i].sessStatus = NOT_IDENTIFIED_USER;
            sessions[i].conn_sock = conn_sock;
            return i;
        }
    }
    return -1;
}

void handle_message(char *recv_msg, struct Session *sessions, int current_session, struct User *users, int *number_of_users, char *reply_message)
{
    char *token1, *token2, *token3, *token4;
    int cmd = 0;
    token1 = strtok(recv_msg, " ");
    token2 = strtok(NULL, " ");
    token3 = strtok(NULL, " ");
    token4 = strtok(NULL, " ");
    cmd = detect_message(token1);
    int result = 0;
    switch (cmd)
    {
    case LOGIN:
        //printf("Handling user login\n");
        result = login_find_user_id(sessions, current_session, users, *number_of_users, token2);
        switch (result)
        {
        case NOT_FOUND:
            strcpy(reply_message, "01");
            break;
        case SUCCESS:
            result = login_check_password(sessions, current_session, users, *number_of_users, token3);
            switch (result)
            {
            case NOT_FOUND:
                strcpy(reply_message, "F0");
                break;
            case SUCCESS:
                strcpy(reply_message, "00");
                break;
            case BLOCKED:
                strcpy(reply_message, "F1");
                break;
            case INVALID_COMMAND:
                strcpy(reply_message, "F2");
                break;
            default:
                break;
            }
            break;
        case BLOCKED:
            strcpy(reply_message, "F1");
            break;
        case INVALID_COMMAND:
            strcpy(reply_message, "F2");
            break;
        default:
            break;
        }
        break;
    case LOUT:
        //printf("Handling user logout\n\n");
        result = logout_find_user_id(sessions, current_session, token2);
        switch (result)
        {
        case NOT_FOUND:
            strcpy(reply_message, "11");
            break;
        case SUCCESS:
            result = logout_check_password(sessions, current_session, users, *number_of_users, token3);
            switch (result)
            {
            case NOT_FOUND:
                strcpy(reply_message, "F0");
                break;
            case SUCCESS:
                strcpy(reply_message, "10");
                break;
            case INVALID_COMMAND:
                strcpy(reply_message, "F2");
                break;
            default:
                break;
            }
            break;
        case INVALID_COMMAND:
            strcpy(reply_message, "F2");
            break;
        default:
            break;
        }
        break;
    case SIGNUP:
        result = signup_find_user_id(sessions, current_session, users, *number_of_users, token2);
        switch (result)
        {
        case NOT_FOUND:
            result = signup_password(sessions, current_session, users, number_of_users, token2, token3);
            switch (result)
            {
            case NOT_FOUND:
                strcpy(reply_message, "22");
                break;
            case SUCCESS:
                strcpy(reply_message, "20");
                break;
            case INVALID_COMMAND:
                strcpy(reply_message, "F2");
                break;
            default:
                break;
            }
            break;
        case SUCCESS:
            strcpy(reply_message, "21");
            break;
        case INVALID_COMMAND:
            strcpy(reply_message, "F2");
            break;
        default:
            break;
        }
        break;
    default:
        strcpy(reply_message, "F2");
        break;
    }
}

int detect_message(char *token)
{
    if (strcmp(token, "LOGIN") == 0)
    {
        return LOGIN;
    }
    else if (strcmp(token, "SIGNUP") == 0)
    {
        return SIGNUP;
    }
    else if (strcmp(token, "LOUT") == 0)
    {
        return LOUT;
    }
    else if (strcmp(token, "CRRM") == 0)
    {
        return CRRM;
    }
    else if (strcmp(token, "JOIN") == 0)
    {
        return JOIN;
    }
    else if (strcmp(token, "CRPD") == 0)
    {
        return CRPD;
    }
    else if (strcmp(token, "AUCP") == 0)
    {
        return AUCP;
    }
    else if (strcmp(token, "LEAVE") == 0)
    {
        return LEAVE;
    }
    else if (strcmp(token, "SBUY") == 0)
    {
        return SBUY;
    }
    else if (strcmp(token, "DELR") == 0)
    {
        return DELR;
    }
    else if (strcmp(token, "DELP") == 0)
    {
        return DELP;
    }
    return INVALID_COMMAND;
}

int login_find_user_id(Session *sessions, int current_session, User *users, int number_of_users, char *user_id)
{
    int i = 0;
    if ((sessions[current_session].sessStatus == NOT_IDENTIFIED_USER) && (user_id != NULL))
    {
        for (i = 0; i < number_of_users; i++)
        { // find user id
            if (strcmp(users[i].id, user_id) == 0)
            {
                if (check_user_status(users[i]) == ACTIVE)
                {                                                                              // user active
                    change_session_status_correct_userid(sessions, current_session, users[i]); //Not authenticated
                    return SUCCESS;
                }
                else
                { // user blocked
                    return BLOCKED;
                }
            }
        }
        return NOT_FOUND; // user not found
    }
    else
        return INVALID_COMMAND;
}

int check_user_status(User user)
{
    return user.accStatus;
}

void change_session_status_correct_userid(Session *sessions, int current_session, User user)
{
    sessions[current_session].sessStatus = NOT_AUTHENTICATED;
    (sessions[current_session].user).accStatus = user.accStatus;
    strcpy((sessions[current_session].user).id, user.id);
    strcpy((sessions[current_session].user).pass, user.pass);
}

int login_check_password(Session *sessions, int current_session, User *users, int number_of_users, char *password)
{
    if (sessions[current_session].sessStatus == NOT_AUTHENTICATED && (password != NULL))
    {
        if (strcmp((sessions[current_session].user).pass, password) == 0)
        {
            change_session_status_correct_password(sessions, current_session);
            sessions[current_session].number_of_retries = 0;
            return SUCCESS;
        }
        else
        {
            sessions[current_session].number_of_retries += 1;
            if (sessions[current_session].number_of_retries == 3)
            {
                block_user_id(sessions, current_session, users, number_of_users);
                return BLOCKED;
            }
            else
                return NOT_FOUND;
        }
    }
    else
        return INVALID_COMMAND;
}

void change_session_status_correct_password(Session *sessions, int current_session)
{
    sessions[current_session].sessStatus = AUTHENTICATED;
}

void block_user_id(Session *sessions, int current_session, User *users, int number_of_users)
{
    int i = 0;
    sessions[current_session].user.accStatus = BLOCKED;
    sessions[current_session].sessStatus = NOT_IDENTIFIED_USER;
    for (i = 0; i < number_of_users; i++)
    {
        if (strcmp(users[i].id, sessions[current_session].user.id) == 0)
        {
            users[i].accStatus = BLOCKED;
        }
    }
}

int logout_find_user_id(Session *sessions, int current_session, char *user_id)
{
    if (sessions[current_session].sessStatus == AUTHENTICATED)
    {
        if (strcmp(sessions[current_session].user.id, user_id) == 0)
        {
            return SUCCESS;
        }
        else
        {
            return NOT_FOUND;
        }
    }
    else
        return INVALID_COMMAND;
}

void change_session_status_log_out(Session *sessions, int current_session)
{
    sessions[current_session].sessStatus = NOT_IDENTIFIED_USER;
    strcpy(sessions[current_session].user.id, "");
    strcpy(sessions[current_session].user.pass, "");
    sessions[current_session].user.accStatus = NOT_FOUND;
}

int logout_check_password(Session *sessions, int current_session, User *users, int number_of_users, char *password)
{
    if (sessions[current_session].sessStatus == AUTHENTICATED && (password != NULL))
    {
        if (strcmp((sessions[current_session].user).pass, password) == 0)
        {
            change_session_status_log_out(sessions, current_session);
            sessions[current_session].number_of_retries = 0;
            return SUCCESS;
        }
        else
        {
            return NOT_FOUND;
        }
    }
    else
        return INVALID_COMMAND;
}

int signup_find_user_id(Session *sessions, int current_session, User *users, int number_of_users, char *user_id)
{
    int i = 0;
    if ((sessions[current_session].sessStatus == NOT_IDENTIFIED_USER) && (user_id != NULL))
    {
        for (i = 0; i < number_of_users; i++)
        { // find user id
            if (strcmp(users[i].id, user_id) == 0)
            {
                    return SUCCESS;
            }
        }
        return NOT_FOUND; // user not found
    }
    else
        return INVALID_COMMAND;
}

int signup_password(Session *sessions, int current_session, User *users, int *number_of_users, char *user_id, char *password)
{
    if (sessions[current_session].sessStatus == NOT_IDENTIFIED_USER && (password != NULL))
    {
        if(strlen(password) >= 8){
            strcpy(users[*number_of_users].id, user_id);
            strcpy(users[*number_of_users].pass, password);
            users[*number_of_users].accStatus = ACTIVE;
            signup_correct_password(sessions, current_session, users[*number_of_users]);
            sessions[current_session].number_of_retries = 0;
            (*number_of_users)++;
            return SUCCESS;
        }
        else
        {
            return NOT_FOUND;
        }
    }
    else
        return INVALID_COMMAND;
}

void signup_correct_password(Session *sessions, int current_session, User user)
{
    sessions[current_session].sessStatus = AUTHENTICATED;
    (sessions[current_session].user).accStatus = user.accStatus;
    strcpy((sessions[current_session].user).id, user.id);
    strcpy((sessions[current_session].user).pass, user.pass);
}


int save_users_data(User *users, int number_of_users)
{
    FILE *fp = NULL;
    int i = 0;
    if ((fp = fopen("account.txt", "w")) == NULL)
    {
        printf("Cannot open account file\n");
        return -1;
    }
    printf("Number of users: %d\n", number_of_users);
    for (i = 0; i < number_of_users; i++)
    {
        fprintf(fp, "%s ", users[i].id);
        printf("%s ", users[i].id);
        fprintf(fp, "%s ", users[i].pass);
        printf("%s ", users[i].pass);
        fprintf(fp, "%d\n", users[i].accStatus);
        printf("%d\n", users[i].accStatus);
    }
    fclose(fp);
    return 0;
}
