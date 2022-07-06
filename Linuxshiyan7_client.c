//采用UDP通信的客户端程序
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>

#define MAXSIZE 300
#define SERV_PORT 6666
int main(int argc, char *argv[])
{

	struct sockaddr_in servaddr;
	char message_send[MAXSIZE]; //向服务端发送的消息
	char message_recv[MAXSIZE]; //从服务端接收的消息
	int sockfd;					//套接字文件描述符，类似管道。（套接字对象）
	if (argc != 2)
	{
		printf("程序启动缺少用户名!\n");
		exit(-1);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //创建套接字对象，打开网络通信端口
	//设置服务器地址
	bzero(&servaddr, sizeof(servaddr));					 //结构体置零
	servaddr.sin_family = AF_INET;						 //设置协议
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr); //设置IP地址
	servaddr.sin_port = htons(SERV_PORT);				 //设置端口号

	char password[10];
	char username[30];
	printf("请输入密码\n");
	int i=0;                                              //计数器
	while(1)              
	{
	fgets(password, sizeof(password), stdin);
	strcpy(username,argv[1]);
	strcat(username, " "); //在用户名和密码之间插入一个间隔符号（空格）
	char *login_info = strcat(username, password);
	//登陆验证
	sendto(sockfd, login_info, strlen(login_info) + 1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)); //向服务端请求登陆验证

	recvfrom(sockfd, message_recv, sizeof(message_recv), 0, NULL, 0); //接受服务端发送的信息。
	printf("%s", message_recv); //服务端发送的信息中包含\n和\0.
	if(strcmp(message_recv,"登陆失败，密码错误或该用户已登陆\n")!=0)       //如果登陆成功
	{
		break;
	}
	if(i == 2)                                   //密码最多输入三次，三次错误后退出
	{
		printf("连续三次输入错误，退出程序!\n");
		exit(-1);
	}
	printf("请重新输入您的密码\n");
	i++;
	}
	//通信阶段
	//双进程，一个进程全负责接受，一个进程负责发送
	pid_t pid;
	pid = fork();
	if(pid == -1)
	{		
		printf("子进程创建失败,程序终止\n");
		exit(-1);
	}
	//接受、发送分离
	if(pid == 0)           //子进程入口
	{
		while(1)
		{
			recvfrom(sockfd, message_recv, sizeof(message_recv), 0, NULL, 0); //接受服务端发送的信息
			printf("%s", message_recv);
			if(strcmp(message_recv,"拜拜，欢迎下次再来\n")==0)//程序唯一出口
				break;
		}
		kill(getppid(),SIGKILL);//子进程直接发送2号信号给父进程，父进程直接终止
	}
	if(pid > 0)            //父进程入口
	{
		//while (fgets(message_send, sizeof(message_send), stdin) != NULL）&& strncmp(message_send, "quit", 4) != 0)
		while(1)
		{
			/* 存在一个问题，就是必须输入一个回车才能结束程序，以为父进程会阻塞在fgets（）那里
			pid_t p = waitpid(pid,NULL,WNOHANG);//以非阻塞的方式试探子进程是否终止。子进程终止，父进程给子进程收尸后，紧跟着也终止。
			//相当于向服务端发送logout后，服务端发送终止客户端的消息，客户端终止。反映到客户端程序上即服务端发送消息给客户端，子进程受到后，子进程终止。父进程探测到子进程终止后，立马给子进程收尸，随后也终止，客户端结束.
			if(p>0)          //如果子进程终止，即输入logout后，服务端发送退出消息给子进程。
			{
				break;
			}
			*/
			fgets(message_send,sizeof(message_send),stdin);
			sendto(sockfd, message_send, strlen(message_send) + 1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)); //将需求发送给服务端
		}
		
	}
	close(sockfd);
	return 0;
}
