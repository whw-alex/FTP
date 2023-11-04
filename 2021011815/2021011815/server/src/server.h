#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>

#define PORT 21
#define DATA_PORT 20
#define BUFFER_SIZE 8192
#define PORT_MODE 0
#define PASV_MODE 1
#define NEED_USER 0
#define NEED_PASS 1
#define LOGGED_IN 2
#define COMMANDS_NUM 17

char *COMMANDS[256] = {"USER", "PASS", "PORT", "PASV", "RETR", "STOR", "SYST", "TYPE", "QUIT", "ABOR", "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO"};
int file_is_transferring = 0;

// information struct to store user information
struct User_info
{
    // socket to transfer data
    int file_socket;
    // data mode 0: port, 1: pasv
    int data_mode;
    int pasv_port;
    int status;
    char current_dir[256];
    char root_dir[256];
    char old_filename[256];
    char last_command[256];
    int h1,h2,h3,h4;
    struct sockaddr_in data_addr;
    pid_t child_pid;
    int child_status;
    pthread_mutex_t mutex;
};

struct ThreadArgs{
    int client_socket;
    char *sentence;
    struct User_info *user_info;
};

// init user info
void init_User_info(struct User_info *user_info)
{
    user_info->file_socket = -1;
    user_info->data_mode = -1;
    user_info->pasv_port = -1;
    user_info->status = NEED_USER;
    user_info->h1 = 0;
    user_info->h2 = 0;
    user_info->h3 = 0;
    user_info->h4 = 0;
    strcpy(user_info->root_dir, "/tmp");
    pthread_mutex_init(&user_info->mutex, NULL);
}


// function to handel user commands
void handle_user(int client_socket, char* sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strcmp(sentence, "USER anonymous") == 0)
    {
        strcpy(response, "331 Guest login ok, send your complete e-mail address as password.\r\n");
        user_info->status = NEED_PASS;
        send(client_socket, response, strlen(response), 0);
        strcpy(user_info->last_command, "USER");
    }
    else
    {
        strcpy(response, "530 This FTP server is anonymous only.\r\n");
        send(client_socket, response, strlen(response), 0);
        // close(client_socket);
        return;
    }
}

void handle_cwd(int client_socket, char *sentence, struct User_info *user_info);

// function to handel pass commands
void handle_pass(int client_socket, char* sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "PASS ", 5) != 0)
    {
        strcpy(response, "530 Login incorrect.\r\n");
        send(client_socket, response, strlen(response), 0);
        // close(client_socket);
        return;
       
    }
    // extract email from buffer
    // char *email = sentence + 5;
    // TODO: check if email is valid using a user table
    // strcpy(response, "230-\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-Welcome to\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-School of Software\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-FTP Archives at ftp.ssast.org\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-This site is provided as a public service by School of\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-Software. Use in violation of any applicable laws is strictly\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-prohibited. We make no guarantees, explicit or implicit, about the\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-contents of this site. Use at your own risk.\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230-\r\n");
    // send(client_socket, response, strlen(response), 0);
    // strcpy(response, "230 Guest login ok, access restrictions apply.\r\n");
    // send(client_socket, response, strlen(response), 0);

    strcpy(response, "230-\r\n230-Welcome to\r\n230-School of Software\r\n230-FTP Archives at ftp.ssast.org\r\n230-\r\n230-This site is provided as a public service by School of\r\n230-Software. Use in violation of any applicable laws is strictly\r\n230-prohibited. We make no guarantees, explicit or implicit, about the\r\n230-contents of this site. Use at your own risk.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n");
    send(client_socket, response, strlen(response), 0);
    user_info->status = LOGGED_IN;
    // change the current dir to root dir
    chdir(user_info->root_dir);
    char temp[256];
    getcwd(temp, 256);
    strcpy(user_info->root_dir, temp);
    strcpy(user_info->current_dir, user_info->root_dir);
    // printf("current_dir: %s\n", user_info->current_dir);
    chdir(user_info->current_dir);
    strcpy(user_info->last_command, "PASS");
}

// function to handle port commands
void handle_port(int client_socket, char *sentence, struct User_info *user_info)
{ 
    if (user_info->file_socket != -1)
    {
        close(user_info->file_socket);
    }
    user_info->file_socket = -1;
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "PORT ", 5) == 0)
    {
        // extract ip and port from buffer
        char *ip_port = sentence + 5;
        // extract h1,h2,h3,h4,p1,p2 from buffer
        int p1, p2;
        sscanf(ip_port, "%d,%d,%d,%d,%d,%d", &user_info->h1, &user_info->h2, &user_info->h3, &user_info->h4, &p1, &p2);


        // create address
        memset(&user_info->data_addr, 0, sizeof(user_info->data_addr));
        user_info->data_addr.sin_port = htons(p1 * 256 + p2);
        // printf("port: %hu\n", user_info->data_addr.sin_port);
        user_info->data_addr.sin_family = AF_INET;
        
        strcpy(response, "200 PORT command successful.\r\n");
        send(client_socket, response, strlen(response), 0);

        user_info->data_mode = PORT_MODE;
        strcpy(user_info->last_command, "PORT");
    }
}

// function to connect to the client in port mode
void port_connect(int client_socket, struct User_info *user_info)
{
    // char response[BUFFER_SIZE];
        // create data socket
    user_info->file_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // int reuse = 1;
    // setsockopt(user_info->file_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
    if (user_info->file_socket < 0)
    {
        return;
    }
    
    char ip[256];
    memset(ip, 0, sizeof(ip));
    sprintf(ip, "%d.%d.%d.%d", user_info->h1, user_info->h2, user_info->h3, user_info->h4);
    // printf("ip: %s, port: %hu\n", ip, user_info->data_addr.sin_port);
    if (inet_pton(AF_INET, ip, &user_info->data_addr.sin_addr) <= 0)
    {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        user_info->file_socket = -1;
    }
    
    if (connect(user_info->file_socket, (struct sockaddr *)&user_info->data_addr, sizeof(user_info->data_addr)) >= 0)
    {
        // strcpy(response, "425 No TCP connection was established.\r\n");
        // send(client_socket, response, strlen(response), 0);
        return;
    }
    else
    {
        printf("connect error\n");
    }

    user_info->file_socket = -1;
    

}
// function to get the local server ip
char *get_local_ip()
{
    char host_name[128];
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host_name, sizeof(host_name));
    if (hostname == -1)
    {
        printf("Error while getting hostname\n");
        exit(1);
    }
    host_entry = gethostbyname(host_name);
    if (host_entry == NULL)
    {
        printf("Error while getting host entry\n");
        exit(1);
    }
    char *ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    // printf("Local IP: %s\n", ip);
    return ip;
}

// function to handle pasv commands
void handle_pasv(int client_socket, char *sentence, struct User_info *user_info)
{
    if (user_info->file_socket != -1)
    {
        close(user_info->file_socket);
    }
    user_info->file_socket = -1;
    char response[BUFFER_SIZE];
    if (strcmp(sentence, "PASV") == 0)
    {
        // create address
        struct sockaddr_in data_addr;
        memset(&data_addr, 0, sizeof(data_addr));
        // data_addr.sin_addr.s_addr = inet_addr(get_local_ip());
        data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        // random port number between 20000 and 65535
        int random_port = rand() % (65535 - 20000 + 1) + 20000;
        // printf("random_port: %d\n", random_port);
        data_addr.sin_port = htons(random_port);
        data_addr.sin_family = AF_INET;
        // int temp = 1;
        // printf("%d\n", temp);
        // create data socket
        int data_socket = -1;
        data_socket = socket(AF_INET, SOCK_STREAM, 0);
        // int reuse = 1;
        // setsockopt(data_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
        printf("data_socket: %d\n", data_socket);
        if (data_socket < 0)
        {
            strcpy(response, "425 connection attempts fails.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        // bind socket to address
        if (bind(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
        {
            strcpy(response, "425 connection attempts fails.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        // listen for connections
        if (listen(data_socket, 5) < 0)
        {
            strcpy(response, "425 connection attempts fails.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        // temp = 2;
        // printf("%d\n", temp);

        char *ip = get_local_ip();
        // printf("ip: %s\n", ip);
        int h1,h2,h3,h4,p1,p2;
        sscanf(ip, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
        // printf("%d.%d.%d.%d\n", h1, h2, h3, h4);
        p1 = random_port / 256;
        p2 = random_port % 256;
        // printf("%d %d\n", p1, p2);
        sprintf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", h1, h2, h3, h4, p1, p2);
        // printf("%s", response);
        send(client_socket, response, strlen(response), 0);

        user_info->data_mode = PASV_MODE;
        user_info->file_socket = data_socket;
        user_info->pasv_port = data_addr.sin_port;
        strcpy(user_info->last_command, "PASV");
    }
}

// function to handle retr_child commands
void handle_retr_child(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];

    if (strncmp(sentence, "RETR ", 5) == 0)
    {
        // extract file name from buffer
        char *file_name = sentence + 5;
        // first check ../ 是否在file_name中
        if (strstr(file_name, "../") != NULL)
        {
            strcpy(response, "550 Permission denied.\r\n");
            send(client_socket, response, strlen(response), 0);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
        // 绝对路径or相对路径
        if (file_name[0] != '/')
        {
            // 相对路径
            char *temp_file_name = (char *)malloc(sizeof(char) * 256);
            strcpy(temp_file_name, user_info->current_dir);
            if (user_info->current_dir[strlen(user_info->current_dir) - 1] == '/')
            {
                strcat(temp_file_name, file_name);
            }
            else
            {
                strcat(temp_file_name, "/");
                strcat(temp_file_name, file_name);
            }
            strcpy(file_name, temp_file_name);
            free(temp_file_name);
        }
        // 判断是否超过root_dir
        if (strstr(file_name, user_info->root_dir) == NULL)
        {
            strcpy(response, "550 File not found or permission denied.\r\n");
            send(client_socket, response, strlen(response), 0);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
        // printf("file_name: %s\n", file_name);
        // open file
        
        
        FILE *file = fopen(file_name, "rb");
        if (file == NULL)
        {
            char temp[256] = "550 File not found or permission denied.\r\n";
            // DIR *dir;
            // struct dirent *entry;
            // dir = opendir(user_info->current_dir);
            // if (dir == NULL) {
            //     // perror("Unable to open directory");
            //     strcat(temp, "unable to open directory ");
            //     strcat(temp, user_info->current_dir);
            //     strcat(temp, " ");
            //     strcat(temp, file_name);
            //     strcat(temp, "\r\n");
            // send(client_socket, temp, strlen(temp), 0);
            //     return;
            // }

            // // 读取目录中的文件
            // while ((entry = readdir(dir)) != NULL) {
            //     // printf("%s\n", entry->d_name);
            //     strcat(temp, entry->d_name);
            // }

            // // 关闭目录
            // closedir(dir);
            
            // strcat(temp, "\r\n");
            strcpy(response, temp);
            // strcpy(response, file_name);
            // printf("%d\n", errno);
            send(client_socket, response, strlen(response), 0);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }

        // get file size
        struct stat file_stat;
        int file_size = 0;
        if (stat(file_name, &file_stat) == 0) {
            file_size = file_stat.st_size;
            // printf("File size: %d bytes\n", file_size);
        } else {
            // printf("Failed to get file size.\n");
        }
        // fseek(file, 0, SEEK_END);
        // int file_size = ftell(file);
        // fseek(file, 0, SEEK_SET);
        // printf("file_size: %d\n", file_size);
        // send file
        // check if PORT or PASV is used
        if (user_info->data_mode == -1)
        {
            strcpy(response, "425 Use PORT or PASV first.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        
        int data_socket = -1;
        // if port mode
        if (user_info->data_mode == PORT_MODE)
        {
            port_connect(client_socket, user_info);
            data_socket = user_info->file_socket;
            // printf("port connect data_socket: %d\n", data_socket);
            if (data_socket < 0)
            {
                strcpy(response, "425 No connection was established.\r\n");
                send(client_socket, response, strlen(response), 0);
                fclose(file);
                close(data_socket);
                user_info->data_mode = -1;
                user_info->file_socket = -1;
                return;
            }
            
        }
        else if (user_info->data_mode == PASV_MODE)
        {
            // printf("pasv connect\n");
            data_socket = accept(user_info->file_socket, NULL, NULL);
            if (data_socket < 0)
            {
                strcpy(response, "425 No connection was established.\r\n");
                send(client_socket, response, strlen(response), 0);
                fclose(file);
                close(data_socket);
                user_info->data_mode = -1;
                user_info->file_socket = -1;
                return;
            }
            // close(user_info->file_socket);
        }

        // send file size
        sprintf(response, "150 Opening BINARY mode data connection for %s (%d bytes).\r\n", file_name, file_size);
        send(client_socket, response, strlen(response), 0);
        file_is_transferring = 1;
        // puts("start to transfer");
        // transfer the file
        char file_buffer[BUFFER_SIZE];
        int read_size;
        while ((read_size = fread(file_buffer, 1, BUFFER_SIZE, file)) > 0)
        {
            if (send(data_socket, file_buffer, read_size, 0) < 0)
            {
                strcpy(response, "426 Connection was established but broken by the client or by network failure.\r\n");
                send(client_socket, response, strlen(response), 0);
                fclose(file);
                close(data_socket);
                user_info->data_mode = -1;
                user_info->file_socket = -1;
                return;
            }
        }
        
        fclose(file);
        close(data_socket);
        strcpy(response, "226 Transfer complete.\r\n");
        send(client_socket, response, strlen(response), 0);
        user_info->file_socket = -1;
        user_info->data_mode = -1;
        strcpy(user_info->last_command, "RETR");
        file_is_transferring = 0;
        // puts("retr child exit");

    }
    file_is_transferring = 0;
}

// function to handle retr commands
void handle_retr(int client_socket, char *sentence, struct User_info *user_info)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Error fork()\n");
        exit(1);
    }
    else if (pid == 0)
    {
        // child process
        handle_retr_child(client_socket, sentence, user_info);
        close(client_socket);
        exit(0);
    }
    else
    {
        // parent process
        // file_is_transferring = 1;
        return;
    }
    
}


// function to handle stor_child commands
void handle_stor_child(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "STOR ", 5) == 0)
    {
        // extract file name from buffer
        char *file_name = sentence + 5;
        // first check ../ 是否在file_name中
        if (strstr(file_name, "../") != NULL)
        {
            strcpy(response, "550 Permission denied.\r\n");
            send(client_socket, response, strlen(response), 0);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
        // file_name是client上的文件名，需要转换成server上的文件名
        int i = strlen(file_name) - 1;
        while (i >= 0 && file_name[i] != '/' && file_name[i] != '\\')
        {
            i -= 1;
        }
        char *new_file_name = file_name + i + 1;
        // printf("new_file_name: %s\n", new_file_name);

        // open file
        FILE *file = fopen(new_file_name, "wb");
        if (file == NULL)
        {
            strcpy(response, "451 Can't save file to the disk.\r\n");
            send(client_socket, response, strlen(response), 0);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
        
        // check if PORT or PASV is used
        if (user_info->data_mode == -1)
        {
            strcpy(response, "425 Use PORT or PASV first.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        int data_socket = -1;
        // if port mode
        if (user_info->data_mode == PORT_MODE)
        {
            port_connect(client_socket, user_info);
            data_socket = user_info->file_socket;
            // printf("port connect data_socket: %d\n", data_socket);
            if (data_socket < 0)
            {
                strcpy(response, "425 No connection was established.\r\n");
                send(client_socket, response, strlen(response), 0);
                fclose(file);
                close(data_socket);
                user_info->data_mode = -1;
                user_info->file_socket = -1;
                return;
            }
            
        }
        else if (user_info->data_mode == PASV_MODE)
        {
            data_socket = accept(user_info->file_socket, NULL, NULL);
            if (data_socket < 0)
            {
                strcpy(response, "425 No connection was established.\r\n");
                send(client_socket, response, strlen(response), 0);
                fclose(file);
                close(data_socket);
                user_info->data_mode = -1;
                user_info->file_socket = -1;
                return;
            }
            close(user_info->file_socket);
        }
        

        // send file size
        sprintf(response, "150 Opening BINARY mode data connection for %s.\r\n", new_file_name);
        send(client_socket, response, strlen(response), 0);
        file_is_transferring = 1;
        // receive the file
        char file_buffer[BUFFER_SIZE];
        int read_size;
        while ((read_size = recv(data_socket, file_buffer, BUFFER_SIZE, 0)) > 0)
        {
            if (fwrite(file_buffer, 1, read_size, file) < read_size)
            {
                strcpy(response, "426 Connection was established but broken by the client or by network failure.\r\n");
                send(client_socket, response, strlen(response), 0);
                fclose(file);
                close(data_socket);
                user_info->data_mode = -1;
                user_info->file_socket = -1;
                return;
            }
        }

        fclose(file);
        close(data_socket);
        strcpy(response, "226 Transfer complete.\r\n");
        file_is_transferring = 0;
        send(client_socket, response, strlen(response), 0);
        user_info->file_socket = -1;
        user_info->data_mode = -1;
        strcpy(user_info->last_command, "STOR");

    }
    else
    {
        strcpy(response, "450 Invalid Command.\r\n");
        send(client_socket, response, strlen(response), 0);
    }
    file_is_transferring = 0;
    // puts("stor child exit");
}

// function to handle stor commands
void handle_stor(int client_socket, char *sentence, struct User_info *user_info)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Error fork()\n");
        exit(1);
    }
    else if (pid == 0)
    {
        // // child process
        // struct User_info *child_user_info = shmat(shmid, NULL, 0);
        // if (child_user_info == (void *)-1) {
        //     // 处理共享内存附加失败的情况
        //     perror("shmat");
        //     exit(1);
        // }
        handle_stor_child(client_socket, sentence, user_info);
        // shmdt(child_user_info);
        close(client_socket);
        exit(0);
    }
    else
    {
        // parent process
        // int shmid = shmget(IPC_PRIVATE, sizeof(struct User_info), IPC_CREAT | 0666);
        // if (shmid == -1) {
        //     // 处理共享内存创建失败的情况
        //     perror("shmget");
        //     exit(1);
        // }
        // struct User_info *shared_user_info = shmat(shmid, NULL, 0);
        // if (shared_user_info == (void *)-1) {
        //     // 处理共享内存附加失败的情况
        //     perror("shmat");
        //     exit(1);
        // }

        // // 在父进程中将用户信息复制到共享内存区域
        // memcpy(shared_user_info, user_info, sizeof(struct User_info));

        // shared_user_info->file_is_transferring = 1;
        // shared_user_info->child_pid = waitpid(pid, &shared_user_info->child_status, WNOHANG);
        // shmdt(shared_user_info);
        return;
    }
    
}

// function to handle syst commands
void handle_syst(int client_socket, char *sentence, struct User_info *user_info)
{
    // printf("syst\n");
    char response[BUFFER_SIZE];
    strcpy(response, "215 UNIX Type: L8\r\n");
    send(client_socket, response, strlen(response), 0);
    strcpy(user_info->last_command, "SYST");

}

// function to handle type commands
void handle_type(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strcmp(sentence, "TYPE I") == 0)
    {
        strcpy(response, "200 Type set to I.\r\n");
        send(client_socket, response, strlen(response), 0);
        strcpy(user_info->last_command, "TYPE");
    }
    else
    {
        strcpy(response, "504 Command not found.\r\n");
        send(client_socket, response, strlen(response), 0);
    }
}

// function to handle quit commands
void handle_quit(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strcmp(sentence, "QUIT") == 0)
    {
        strcpy(response, "221 Goodbye.\r\n");
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        close(user_info->file_socket);
    }
    
}

// function to handle abor commands
void handle_abor(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strcmp(sentence, "ABOR") == 0)
    {
        strcpy(response, "221 Goodbye.\r\n");
        send(client_socket, response, strlen(response), 0);
        close(user_info->file_socket);
        close(client_socket);
        user_info->file_socket = -1;
        user_info->data_mode = -1;
    }
}

// function to handle mkd commands
void handle_mkd(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "MKD ", 4) == 0)
    {
        // extract dir name from buffer
        char *dir_name = sentence + 4;
        // printf("dir_name: %s, len:%lu\n", dir_name, strlen(dir_name));
        // create dir
        if (mkdir(dir_name, 0777) < 0)
        {
            strcpy(response, "550 make directory operation failed.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        strcpy(response, "257 Directory created.\r\n");
        send(client_socket, response, strlen(response), 0);
        strcpy(user_info->last_command, "MKD");
    }
}

// function to handle cwd commands
void handle_cwd(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "CWD ", 4) == 0)
    {
        // 思路：系统先去，之后getcwd，然后判断是否在root_dir里面
        // extract dir name from buffer
        char *dir_name = sentence + 4;
        // 判断是否在root_dir里面
        // printf("dir_name: %s\n", dir_name);
        // if (chdir(dir_name) < 0)
        // {
        //     perror("chdir");
        //     strcpy(response, "550 change directory operation failed.\r\n");
        //     send(client_socket, response, strlen(response), 0);
        //     return;
        // }
        chdir(dir_name);
        char *temp_dir = (char *)malloc(sizeof(char) * 256);
        getcwd(temp_dir, 256);
        // printf("temp_dir: %s\n", temp_dir);
        if (strstr(temp_dir, user_info->root_dir) == NULL)
        {
            strcpy(response, "550 No permission.\r\n");
            chdir(user_info->current_dir);
            send(client_socket, response, strlen(response), 0);
            return;
        }
        else
        {
            strcpy(user_info->current_dir, temp_dir);
            strcpy(response, "250 Directory successfully changed. Current directory: ");
            strcat(response, user_info->current_dir);
            strcat(response, "\r\n");
            send(client_socket, response, strlen(response), 0);
            strcpy(user_info->last_command, "CWD");
            return;
        }
    }
    else
    {
        strcpy(response, "550 change directory operation failed.\r\n");
        send(client_socket, response, strlen(response), 0);
    }
}

// function to handle pwd commands
void handle_pwd(int client_socket, char *sentence, struct User_info *user_info)
{
    // printf("pwd\n");
    char response[BUFFER_SIZE];
    if (strcmp(sentence, "PWD") == 0)
    {
        sprintf(response, "257 \"%s\" is current directory.\r\n", user_info->current_dir);
        send(client_socket, response, strlen(response), 0);
        strcpy(user_info->last_command, "PWD");
    }
}

// function to handle list commands
void handle_list(int client_socket, char *sentence, struct User_info *user_info)
{
    // printf("list\n");
    // check if ../ is contained in the parameter
    if (strstr(sentence, "../") != NULL)
    {
        char response[BUFFER_SIZE];
        strcpy(response, "550 Permission denied.\r\n");
        send(client_socket, response, strlen(response), 0);
        return;
    }
    char response[BUFFER_SIZE];
    int data_socket = -1;
    // check if PORT or PASV is used
    if (user_info->data_mode == -1)
    {
        strcpy(response, "425 Use PORT or PASV first.\r\n");
        send(client_socket, response, strlen(response), 0);
        return;
    }
    // if port mode
    if (user_info->data_mode == PORT_MODE)
    {
        port_connect(client_socket, user_info);
        data_socket = user_info->file_socket;
        // printf("port connect data_socket: %d\n", data_socket);
        if (data_socket < 0)
        {
            strcpy(response, "425 No connection was established.\r\n");
            send(client_socket, response, strlen(response), 0);
            close(data_socket);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
    }
    else if (user_info->data_mode == PASV_MODE)
    {
        // printf("is pasv, %d\n", user_info->file_socket);
        data_socket = accept(user_info->file_socket, NULL, NULL);
        // printf("pasv connect data_socket: %d\n", data_socket);
        if (data_socket < 0)
        {
            strcpy(response, "425 No connection was established.\r\n");
            send(client_socket, response, strlen(response), 0);
            close(data_socket);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
        // close(user_info->file_socket);
    }

    sprintf(response, "150 Opening BINARY mode data connection.\r\n");
    send(client_socket, response, strlen(response), 0);

    // char file_buffer[BUFFER_SIZE];
    // int read_size;
    char command[256] = "ls -l";

    if (strcmp(sentence, "LIST") == 0)
    {
        strcat(command, " ");
        // strcat(command, "/private");
        strcat(command, user_info->current_dir);
    }
    else if (strncmp(sentence, "LIST ", 5) == 0)
    {
        char *path = sentence + 5;
        strcat(command, " ");
        strcat(command, path);
    }
    // printf("command: %s\n", command);
    FILE *fp = popen(command, "r");
    
    if (fp == NULL)
    {
        strcpy(response, "550 list operation failed.\r\n");
        send(client_socket, response, strlen(response), 0);
        return;
    }

    while (fgets(response, sizeof(response)-1, fp) != NULL) 
    {
        // printf("%s", response);
        if (send(data_socket, response, strlen(response), 0) < 0)
        {
            strcpy(response, "426 Connection was established but broken by the client or by network failure.\r\n");
            send(client_socket, response, strlen(response), 0);
            pclose(fp);
            close(data_socket);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
    }

    // 关闭管道
    pclose(fp);
    close(data_socket);
    strcpy(response, "226 List complete.\r\n");
    send(client_socket, response, strlen(response), 0);
    strcpy(user_info->last_command, "LIST");
    user_info->data_mode = -1;
    user_info->file_socket = -1;
}

// function to handle rmd commands
void handle_rmd(int client_socket, char *sentence, struct User_info *user_info)
{
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "RMD ", 4) == 0)
    {
        // extract dir name from buffer
        char *dir_name = sentence + 4;
        // if dir name contains current dir
        if (strstr(dir_name, user_info->current_dir) == NULL)
        {
            char *temp = (char *)malloc(sizeof(char) * 256);
            strcpy(temp, user_info->current_dir);
            strcat(temp, "/");
            strcat(temp, dir_name);
            strcpy(dir_name, temp);

        }
        // 检查是否超过root_dir
        chdir(dir_name);
        char *temp_dir = (char *)malloc(sizeof(char) * 256);
        getcwd(temp_dir, 256);
        if (strstr(temp_dir, user_info->root_dir) == NULL)
        {
            strcpy(response, "550 No permission.\r\n");
            chdir(user_info->current_dir);
            send(client_socket, response, strlen(response), 0);
            return;
        }   
        chdir(user_info->current_dir);
        
        // remove dir
        if (rmdir(dir_name) < 0)
        {
            strcpy(response, "550 remove directory operation failed.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        strcpy(response, "250 Directory successfully removed.\r\n");
        send(client_socket, response, strlen(response), 0);
        strcpy(user_info->last_command, "RMD");
    }
}

// funtion to handle rnfr commands
void handle_rnfr(int client_socket, struct User_info *user_info, char *sentence)
{
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "RNFR ", 5) == 0)
    {
        // extract file name from buffer
        char *file_name = sentence + 5;
        // 绝对路径or相对路径
        if (file_name[0] != '/')
        {
            // 相对路径
            char *temp_file_name = (char *)malloc(sizeof(char) * 256);
            strcpy(temp_file_name, user_info->current_dir);
            if (user_info->current_dir[strlen(user_info->current_dir) - 1] == '/')
            {
                strcat(temp_file_name, file_name);
            }
            else
            {
                strcat(temp_file_name, "/");
                strcat(temp_file_name, file_name);
            }
            strcpy(file_name, temp_file_name);
            free(temp_file_name);
        }
        // printf("file_name: %s\n", file_name);
        // 对绝对路径get parent dir
        char *parent_dir = (char *)malloc(sizeof(char) * 256);
        strcpy(parent_dir, file_name);
        int len = strlen(parent_dir);
        len -= 1;
        while (parent_dir[len] != '/')
        {
            len -= 1;
        }
        parent_dir[len] = '\0';
        // printf("parent_dir: %s\n", parent_dir);
        // 检查是否超过root_dir
        if (strstr(parent_dir, user_info->root_dir) == NULL)
        {
            strcpy(response, "550 No permission.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        
        // open file
        FILE *file = fopen(file_name, "rb");
        if (file == NULL)
        {
            strcpy(response, "550 File not found or permission denied.\r\n");
            send(client_socket, response, strlen(response), 0);
            user_info->data_mode = -1;
            user_info->file_socket = -1;
            return;
        }
        strcpy(user_info->old_filename, file_name);
        strcpy(response, "350 File exists, ready for destination name.\r\n");
        send(client_socket, response, strlen(response), 0);
        strcpy(user_info->last_command, "RNFR");    
    }
}

// function to handle rnto commands
void handle_rnto(int client_socket, struct User_info *user_info, char *sentence)
{
    if (strcmp(user_info->last_command, "RNFR") != 0)
    {
        char response[BUFFER_SIZE];
        strcpy(response, "503 RNTO commands should come immediately after RNFR.\r\n");
        send(client_socket, response, strlen(response), 0);
        return;
    }
    char response[BUFFER_SIZE];
    if (strncmp(sentence, "RNTO ", 5) == 0)
    {
        // extract file name from buffer
        char *file_name = sentence + 5;
        // 绝对路径or相对路径
        if (file_name[0] != '/')
        {
            // 相对路径
            char *temp_file_name = (char *)malloc(sizeof(char) * 256);
            strcpy(temp_file_name, user_info->current_dir);
            if (user_info->current_dir[strlen(user_info->current_dir) - 1] == '/')
            {
                strcat(temp_file_name, file_name);
            }
            else
            {
                strcat(temp_file_name, "/");
                strcat(temp_file_name, file_name);
            }
            strcpy(file_name, temp_file_name);
            free(temp_file_name);
        }
        // printf("old file name: %s\n", user_info->old_filename);
        // printf("new file name: %s\n", file_name);
        if (rename(user_info->old_filename, file_name) < 0)
        {
            strcpy(response, "550 Rename operation failed.\r\n");
            send(client_socket, response, strlen(response), 0);
            return;
        }
        else
        {
            strcpy(response, "250 Rename successful.\r\n");
            send(client_socket, response, strlen(response), 0);
            strcpy(user_info->last_command, "RNTO");
            return;
        }
        
    }
}
        

void handle_client(int client_socket, struct User_info *user_info, char *sentence, int len)
{
    char response[BUFFER_SIZE];
    if (file_is_transferring == 1)
    {
    //     if (user_info->child_pid > 0) {
    //         if (WIFEXITED(user_info->child_status)) 
    //         {
    //             printf("child ends\n");
    //         }
    //         else
    //         {
                strcpy(response, "451 A file is transferring currently, please try again later.\r\n");
                send(client_socket, response, strlen(response), 0);
    //         }
    //     }
    //     else
    //     {
    //         strcpy(response, "451 A file is transferring currently, please try again later.\r\n");
    //         send(client_socket, response, strlen(response), 0);
    //     }
        
        
    }
    // printf("handle client:%s\n", sentence);
    // 检查sentence每一个字符是不是\r 或者\n
    for (int i = 0; i < len; i++)
    {
        if (sentence[i] == '\r')
        {
            // printf("find \\r, %d\n", i);
            sentence[i] = '\0';
            len = i+1;
            break;
        }
        if (sentence[i] == '\n')
        {
            // printf("find \\n, %d\n", i);
        }
    }

    char buffer[8192];
    strcpy(buffer, sentence);
    char *cmd = strtok(buffer, " ");
    for (int i = 0; i < COMMANDS_NUM; i++)
    {
        if (strcmp(cmd, COMMANDS[i]) == 0)
        {
            break;
        }
        if (i == COMMANDS_NUM - 1)
        {
            
            strcpy(response, "404 Please enter valid commands.\r\n");
            send(client_socket, response, strlen(response), 0);
            // printf("invalid command\n");
            // close(client_socket);
            return;
        }
    }
    if (user_info->status == NEED_USER)
    {
        if (strncmp(cmd, "USER", 4) == 0)
        {
            handle_user(client_socket, sentence, user_info);
        }
        else
        {
            char response[BUFFER_SIZE];
            strcpy(response, "530 Please login with USER and PASS.\r\n");
            send(client_socket, response, strlen(response), 0);
            close(client_socket);
            return;
        }
    }
    else if (user_info->status == NEED_PASS)
    {
        if (strncmp(cmd, "PASS", 4) == 0)
        {
            handle_pass(client_socket, sentence, user_info);
        }
        else
        {
            char response[BUFFER_SIZE];
            strcpy(response, "530 Please login with USER and PASS.\r\n");
            send(client_socket, response, strlen(response), 0);
            close(client_socket);
            return;
        }
    }
    else if (user_info->status == LOGGED_IN)
    {
        if (strncmp(cmd, "RETR", 4) == 0) handle_retr(client_socket, sentence, user_info);
        if (strncmp(cmd, "STOR", 4) == 0) handle_stor(client_socket, sentence, user_info);
        if (strncmp(cmd, "SYST", 4) == 0) handle_syst(client_socket, sentence, user_info);
        if (strncmp(cmd, "TYPE", 4) == 0) handle_type(client_socket, sentence, user_info);
        if (strncmp(cmd, "QUIT", 4) == 0) handle_quit(client_socket, sentence, user_info);
        if (strncmp(cmd, "ABOR", 4) == 0) handle_abor(client_socket, sentence, user_info);
        if (strncmp(cmd, "PORT", 4) == 0) handle_port(client_socket, sentence, user_info);
        if (strncmp(cmd, "PASV", 4) == 0) handle_pasv(client_socket, sentence, user_info);
        if (strncmp(cmd, "MKD", 3) == 0) handle_mkd(client_socket, sentence, user_info);
        if (strncmp(cmd, "CWD", 3) == 0) handle_cwd(client_socket, sentence, user_info);
        if (strncmp(cmd, "PWD", 3) == 0) handle_pwd(client_socket, sentence, user_info);
        if (strncmp(cmd, "LIST", 4) == 0) handle_list(client_socket, sentence, user_info);
        if (strncmp(cmd, "RMD", 3) == 0) handle_rmd(client_socket, sentence, user_info);
        if (strncmp(cmd, "RNFR", 4) == 0) handle_rnfr(client_socket, user_info, sentence);
        if (strncmp(cmd, "RNTO", 4) == 0) handle_rnto(client_socket, user_info, sentence);
        

    }
    else
    {
        char response[BUFFER_SIZE];
        strcpy(response, "404 Please enter valid commands.\r\n");
        send(client_socket, response, strlen(response), 0);
        // printf("invalid command\n");
        close(client_socket);
        return;
    }

}

// int main()
// {
//     int server_socket;
//     int client_socket;
//     struct sockaddr_in server_address;
//     struct sockaddr_in client_address;
//     socklen_t client_address_length = sizeof(client_address);

//     // create socket
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0)
//     {
//         printf("Error while creating socket\n");
//         exit(1);
//     }

//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(PORT);
//     server_address.sin_addr.s_addr = INADDR_ANY;

//     // bind socket to address
//     if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
//     {
//         printf("Error while binding socket\n");
//         exit(1);
//     }
//     // listen for connections
//     if (listen(server_socket, 5) < 0)
//     {
//         printf("Error while listening\n");
//         exit(1);
//     }
//     printf('server is running on port %d\n', PORT);
//     while (1)
//     {
//         // accept connection
//         client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
//         if (client_socket < 0)
//         {
//             printf("Error while accepting connection\n");
//             exit(1);
//         }
//         // fork a new process to handle client
//         pid_t pid = fork();
//         if (pid == 0)
//         {
//             // child process
//             close(server_socket);
//             // close the server socket in order to avoid conflictsd
//             handle_client(client_socket);
//             exit(0);
//         }
//         else if (pid < 0)
//         {
//             printf("Error while forking\n");
//             exit(1);
//         }
//         else
//         {
//             // parent process
//             close(client_socket);
//             // close the client socket in order to listen to other clients
//         }

//     }
//     close(server_socket);
//     return 0;
// }