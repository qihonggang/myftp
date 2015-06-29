#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

static struct sockaddr_in si;
static char command[64], data[256], *arg[8];
static int sockfd, n;
static FILE *fp;

static void help(void)
{
	printf("put     	send file\n");
	printf("get     	receive file\n");
	printf("quit    	exit ftpc\n");
	printf("help    	print help information\n");
	printf("list    	print server file information\n");
	printf("?       	print help information\n");
}

static void list(void)
{
	if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	if(connect(sockfd, (const struct sockaddr *)&si, sizeof(si)) == -1)
	{
		perror("connect");
		exit(1);
	}

	strcpy(data, arg[0]);
	send(sockfd, data, sizeof(data), 0);

	while(recv(sockfd, data, sizeof(data), 0) > 0)
	{
		printf("%s ", data);
	}
	printf("\n");

	close(sockfd);
}

static void get(void)
{
	if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	if(connect(sockfd, (const struct sockaddr *)&si, sizeof(si)) == -1)
	{
		perror("connect");
		exit(1);
	}

	strcpy(data, arg[0]);
	strcat(data, " ");
	strcat(data, arg[1]);
	send(sockfd, data, sizeof(data), 0);
	recv(sockfd, data, sizeof(data), 0);

	if(strcmp(data, "yes") == 0)
	{
		if((fp=fopen(arg[1], "w")) == NULL)
		{
			perror("fopen");
			close(sockfd);
			return;
		}

		while((n=recv(sockfd, data, sizeof(data), 0)) > 0)
		{
			fwrite(data, 1, n, fp);
		}
		
		fclose(fp);
	}
	else
	{
		if(strcmp(data, "no") == 0)
		{
			printf("server not find %s\n", arg[1]);
		}
	}

	close(sockfd);
}

static void put(void)
{
	if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	if(connect(sockfd, (const struct sockaddr *)&si, sizeof(si)) == -1)
	{
		perror("connect");
		exit(1);
	}

	strcpy(data, arg[0]);
	strcat(data, " ");
	strcat(data, arg[1]);
	send(sockfd, data, sizeof(data), 0);
	recv(sockfd, data, sizeof(data), 0);

	if(strcmp(data, "no") == 0)
	{
		if((fp=fopen(arg[1], "r")) == NULL)
		{
			perror("fopen");
			close(sockfd);
			return;
		}

		while((n=fread(data, 1, sizeof(data), fp)) != 0)
		{
			send(sockfd, data, n, 0);
		}
		
		fclose(fp);
	}
	else
	{
		if(strcmp(data, "yes") == 0)
		{
			printf("server %s exist\n", arg[1]);
		}
	}

	close(sockfd);
}

int main(int argc, const char *argv[])
{
	struct servent *s;
	int i;

	if(argc != 2)
	{
		printf("help informatin: ftpc --help\n");
		exit(1);
	}

	if(strcmp(argv[1], "--help") == 0)
	{
		printf("ftp v1.0\n");
		printf("2015.03.29\n");
		printf("author niulibing\n");
		printf("Usage: ftp <serverip>\n");
		exit(1);
	}

	if((s=getservbyname("ftp", "tcp")) == NULL)
	{
		printf("getservbyname fail\n");
		exit(1);
	}

	si.sin_family = AF_INET;
	si.sin_port = s->s_port;
	si.sin_addr.s_addr = inet_addr(argv[1]);

	while(1)
	{
		printf("myftp> ");
		fgets(command, sizeof(command), stdin);
		command[strlen(command)-1] = '\0';

		arg[0] = strtok(command, " ");
		for(i=1; arg[i-1]!=NULL; i++)
		{
			arg[i] = strtok(NULL, " ");
		}

		if(strcmp(arg[0], "help")==0 || strcmp(arg[0], "?")==0)
		{
			help();
			continue;
		}

		if(strcmp(arg[0], "quit") == 0)
		{
			exit(0);
		}

		if(strcmp(arg[0], "list") == 0)
		{
			list();
			continue;
		}

		if(strcmp(arg[0], "get") == 0)
		{
			get();
			continue;
		}

		if(strcmp(arg[0], "put") == 0)
		{
			put();
			continue;
		}
	}
}
