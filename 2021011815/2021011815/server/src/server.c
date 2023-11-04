#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include "server.h"

int main(int argc, char **argv) {
	int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
	struct sockaddr_in addr;
	char sentence[8192];
	// int p;
	int len;
	int port = 21;
	char root_dir[256] = "/tmp";
	// 处理argc和argv
	if (argc != 1)
	{
		if (argc == 5)
		{
			if (strcmp(argv[1], "-root") == 0)
			{
				strcpy(root_dir, argv[2]);
				if (strcmp(argv[3], "-port") == 0)
				{
					port = atoi(argv[4]);
				}
				
			}
			else if (strcmp(argv[1], "-port") == 0)
			{
				port = atoi(argv[2]);
				if (strcmp(argv[3], "-root") == 0)
				{
					strcpy(root_dir, argv[4]);
				}
			}

		}
		else if (argc == 3)
		{
			printf("argc == 3\n");
			printf("argv[1]: %s\n", argv[1]);
			printf("argv[2]: %s\n", argv[2]);
			if (strcmp(argv[1], "-root") == 0)
			{
				strcpy(root_dir, argv[2]);
			}
			else if (strcmp(argv[1], "-port") == 0)
			{
				port = atoi(argv[2]);
			}
		}
	}

	//创建socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//设置本机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	// addr.sin_port = htons(32834);
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"
	printf("Server is running on port %d, the root is %s\n", port, root_dir);
	int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
	//将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//开始监听socket
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//持续监听连接请求
	while (1) {
		//等待client的连接 阻塞函数
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
		// fork() handle multiple clients
		pid_t pid = fork();
		if (pid < 0)
		{
			close(connfd);
			close(listenfd);
			printf("Error fork()\n");
			exit(1);
		}
		else if (pid == 0)
		{
			// child process
			// printf("New client connected.\n");
			struct User_info *user_info = malloc(sizeof(struct User_info));
			// init user informatioin
			init_User_info(user_info);
			strcpy(user_info->root_dir, root_dir);
			close(listenfd);
			send(connfd, "220 ftp.ssast.org FTP server ready.\r\n", 37, 0);
			while((len = recv(connfd, sentence, 8192, 0)) > 0)
			{
				sentence[len] = '\0';
				handle_client(connfd, user_info, sentence, len+1);
			}
			close(connfd);
			close(user_info->file_socket);
			free(user_info);
			exit(0);
		
		}
		else
		{
			// parent process
			close(connfd);
		}
	}

	close(listenfd);
}

