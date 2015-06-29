#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
	
static char data[256], *arg[8];
static int acceptfd, n;
static FILE *fp;

static void list(void)
{
	DIR *dir;
	struct dirent *dirent;

	if((dir=opendir((const char *)getcwd(NULL, 0))) == NULL)
	{
		perror("opendir");
		return;
	}

	while((dirent=readdir(dir)) != NULL)
	{
		if(dirent->d_name[0] == '.')
		{
			continue;
		}
		send(acceptfd, dirent->d_name, sizeof(dirent->d_name), 0);
	}

	closedir(dir);
}

static void get(void)
{
	if((fp=fopen(arg[1], "r")) == NULL)
	{
		strcpy(data, "no");
		send(acceptfd, data, sizeof(data), 0);
		return;
	}

	strcpy(data, "yes");
	send(acceptfd, data, sizeof(data), 0);

	while((n=fread(data, 1, sizeof(data), fp)) != 0)
	{
		send(acceptfd, data, n, 0);
	}
	
	fclose(fp);
}

static void put(void)
{
	if((fp=fopen(arg[1], "r")) == NULL)
	{
		strcpy(data, "no");
		send(acceptfd, data, sizeof(data), 0);

		if((fp=fopen(arg[1], "w")) == NULL)
		{
			perror("fopen");
			return;
		}

		while((n=recv(acceptfd, data, sizeof(data), 0)) > 0)
		{
			fwrite(data, 1, n, fp);
		}

		fclose(fp);

		return;
	}

	strcpy(data, "yes");
	send(acceptfd, data, sizeof(data), 0);
}

int main(int argc, const char *argv[])
{
	struct sockaddr_in si;
	socklen_t socklen;
	struct servent *s;
	int sockfd, i;

	signal(SIGCHLD, SIG_IGN);

	printf("ftps v1.0\n");
	printf("2015.03.29\n");
	printf("author niulibing\n");

	if((fp=fopen("/etc/ftp.conf", "r")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	fgets(data, sizeof(data), fp);
	data[strlen(data)-1] = '\0';
	chdir(data);

	if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	if((s=getservbyname("ftp", "tcp")) == NULL)
	{
		printf("getservbyname fail\n");
		exit(1);
	}

	si.sin_family = AF_INET;
	si.sin_port = s->s_port;
	si.sin_addr.s_addr = INADDR_ANY;
	if(bind(sockfd, (const struct sockaddr *)&si, sizeof(si)) == -1)
	{
		perror("bind");
		exit(1);
	}

	if(listen(sockfd, 5) == -1)
	{
		perror("listen");
		exit(1);
	}

	while(1)
	{
		socklen = sizeof(si);
		if((acceptfd=accept(sockfd, (struct sockaddr *)&si, &socklen)) == -1)
		{
			perror("accept");
			exit(1);
		}

		if(fork() == 0)
		{
			close(sockfd);
			
			while(recv(acceptfd, data, sizeof(data), 0) > 0)
			{
				arg[0] = strtok(data, " ");
				for(i=1; arg[i-1]!=NULL; i++)
				{
					arg[i] = strtok(NULL, " ");
				}

				if(strcmp(arg[0], "list") == 0)
				{
					list();
					break;
				}

				if(strcmp(arg[0], "get") == 0)
				{
					get();
					break;
				}

				if(strcmp(arg[0], "put") == 0)
				{
					put();
					break;
				}
			}

			close(acceptfd);
			exit(0);
		}

		close(acceptfd);
	}
}
