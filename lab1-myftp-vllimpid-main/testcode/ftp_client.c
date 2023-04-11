
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
// unsigned long int htons(unsigned short int hostshort);
#include <arpa/inet.h>
// in_addr_t inet_addr(const char *cp);
#include <errno.h>

#define DEFAULT_PORT 12345
#define DEFAULT_IP "127.0.0.1"
#define USERNAME "user"
#define PASSWORD "123123"
#define MAXLINE 1024

struct header
{
    char m_protocol[6]; /* protocol magic number (6 bytes) */
    uint8_t m_type;               /* type (1 byte) */
    uint8_t m_status;            /* status (1 byte) */
    uint32_t m_length;        /* length (4 bytes) in Big endian*/
} __attribute__((packed));

struct ftpmsg
{
    struct header m_header;
    char *payload;
};
void myprintf(struct header myheader){
    printf("protocol:%s\n",myheader.m_protocol);
    printf("like:%d\n",(char)0xA2);
    printf("type:%d\n",myheader.m_type);
    printf("status:%d\n",myheader.m_status);
}
char *Substr(char *s,int n,int len)
{
	char p[51200];//或者设置为静态变量 
	int i,j=0;
	while(n--){
		s++;//确定字符串的首位置 
	}
	for(i=n;i<n+len;i++){
		 p[j++]=*s;
		 s++;
	} 
	return p;
} 

int gettoken(char *token, char *line)
{
    int i = 0; // 记录token的位数
    // 移除line开头的空格
    while (*line == ' ')
    {
        strcpy(line, line + 1);
    }
    if (*line == '\"')
    {
        // 特殊情况：第一个单词由引号开头，则将第二个引号之前的内容移到token
        strcpy(line, line + 1);
        // 遇到第二个引号前，把每个字符移入token
        while (*line != '\"' && *line != '\0')
        {
            *token++ = *line;
            i++;
            strcpy(line, line + 1);
        }
        strcpy(line, line + 1); // 把第二个引号移除
    }
    else
    {
        // 将第一个词移入token
        while (*line != ' ' && *line != '\"' && *line != '\0')
        {
            *token++ = *line;
            i++;
            strcpy(line, line + 1);
        }
    }
    // 给token添加结束符号
    *token++ = '\0';
    if (i == 0)
    {
        return -1;
    }
    // 移除第一个词后的空格
    while (*line == ' ')
    {
        strcpy(line, line + 1);
    }
    return i;
}

int send_msg(int sockfd, struct ftpmsg *msg)
{
    
    printf("sending message!\n");
    int len=msg->m_header.m_length;
    msg->m_header.m_length=ntohl(msg->m_header.m_length);
    send(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    printf("len:%d\n",len);
    if(len == 12){
        return 0;//如果长度为12，说明只有报文头，没有负载
    }
    else {
        //如果长度大于12，说明有负载，根据长度计算出负载的长度进行发送
        //char buf[1024*1024+10]={};
        if(len-12>51200){
        
            len=len-12;
            //buf=msg->payload;
            //printf("data:%s\n",msg->payload);
            
            int times=len/51200+1;
            printf("len:%d times: %d\n",len,times);
            for(int i=0;i<times;i++){
            	printf("send message %d\n",i+1);
                if(i<times-1){
                	char *buf=Substr(msg->payload,i*51200,(i+1)*51200);
                	buf[51200]='\0';
                    send(sockfd,buf,51201,0);
                }
                else{
                	char *buf=Substr(msg->payload,i*51200,len-i*51200);
                	buf[len-i*51200]='\0';
                    send(sockfd,buf,len-i*51200+1,0);
                }
            }
        }
        else{
        	printf("send small\n");
            send(sockfd,msg->payload,len-12,0);
        }
        
    }
    return 0;
}

int recv_msg(int sockfd, struct ftpmsg *msg)
{
    printf("recving message!\n");
    recv(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    msg->m_header.m_length=htonl(msg->m_header.m_length);
    myprintf(msg->m_header);
    printf("recving header\n");
    printf("length: %d\n",msg->m_header.m_length);
    if(msg->m_header.m_length == 12){
        return 0;//如果长度为12，说明只有报文头，没有负载
    }
    else {
        //如果长度大于12，说明有负载，根据长度计算出负载的长度进行发送
        if(msg->m_header.m_length-12-1>51200){
        
            msg->payload=malloc(msg->m_header.m_length-12);
        
            int len=msg->m_header.m_length-12;
        
            //printf("data:%s\n",msg->payload);
            int times=len/51200+1;
            for(int i=0;i<times;i++){
            char *thisrecv;
            	if(i==times-1){
             	   recv(sockfd,thisrecv,51200,0);
            	}
            else{
                recv(sockfd,thisrecv,len-i*51200,0);
            }
            	strcat(msg->payload,thisrecv);
        	}
        
    	}
    	else{
    		printf("recv small\n");
    		msg->payload=malloc(msg->m_header.m_length-12);
    		recv(sockfd,msg->payload,msg->m_header.m_length-12,0);
    	}
    }
    return 0;
}
int c_ls(int sockfd)
{
    struct ftpmsg msg;
    memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
    msg.m_header.m_type = 0xA5;
    msg.m_header.m_length = 12;
    int result = send_msg(sockfd, &msg); //向sockfd发送open SERVER_IP SERVER_PORT
    //printf("%d\n", result);
    
    struct ftpmsg returnmsg;
    recv_msg(sockfd, &returnmsg);
    printf("---- file list start ----\n");
    printf("%s\n",returnmsg.payload);
    printf("---- file list end ----\n");
    return 0;
}
int c_get(int sockfd, char filename[MAXLINE])
{

    struct ftpmsg msg,returnmsg;
    memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
    msg.m_header.m_type = 0xA7;

    msg.payload = filename;
    msg.m_header.m_length = 12 + strlen(filename) + 1;
    send_msg(sockfd, &msg); //向sockfd发送open SERVER_IP SERVER_PORT
    recv_msg(sockfd, &returnmsg);
    if (returnmsg.m_header.m_status == 1)
    {
        int i = 0, nwrite;
        int fd=0;
        long size, recvd = 0;
        //printf("filename:%s\n",filename);
        fd = open(filename, O_WRONLY|O_CREAT,0600);//printf("pause %d\n",fd);
        if (fd == -1)
        {
            close(fd);
            return -1;
        }
        recv_msg(sockfd,&returnmsg);
        //printf("get data:%s\n",returnmsg.payload);
        printf("length:%d\n",returnmsg.m_header.m_length-12);
        write(fd, returnmsg.payload, returnmsg.m_header.m_length-12);
        printf("File downloaded\n");
        close(fd);
        return 0;
    }
    else
    {
        printf("File downloaded failed\n");
        return -1;
    }
}
int c_put(int sockfd, char path[MAXLINE])
{
    int fd=0;
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("This file not exist!\n");
        return -1;
    }
    struct ftpmsg msg;
    memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
    msg.m_header.m_type = 0xA9;
    msg.payload = path;
    msg.m_header.m_length = 12 + strlen(path) + 1;
    send_msg(sockfd, &msg); //向sockfd发送open SERVER_IP SERVER_PORT
    recv_msg(sockfd, &msg);
    if (msg.m_header.m_type == 0xAA)
    { //收到PUT_REPLY信号
        int nread;
        char buf[1024*1024+10];
        struct ftpmsg msgreturn;
        memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
        msgreturn.m_header.m_type = 0xFF;
        nread = read(fd, buf, sizeof(buf));
        
        buf[nread]='\0';
        //printf("send data:%s\n",buf);
        printf("%d\n",nread);
        msgreturn.m_header.m_length = 12 + nread;
        msgreturn.payload = buf;
        send_msg(sockfd, &msgreturn);
        close(fd);
        printf("File uploaded\n");
        return 0;
    }
    else
    {
        close(fd);
        return -1;
    }
}
int c_quit(int sockfd)
{
    struct ftpmsg msg;
    memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
    msg.m_header.m_type = 0xAB;
    msg.m_header.m_length = 12;
    send_msg(sockfd, &msg); //向sockfd发送QUIT_REQUEST
    recv_msg(sockfd,&msg);
    if(msg.m_header.m_type==0xAC){
        close(sockfd);
        
    }
    printf("Thank you.\n");
    return 0;
}
int main()
{

    char option[20];
    char IP[20] = DEFAULT_IP;
    int port = DEFAULT_PORT;
    int sockfd, len, result;
    struct sockaddr_in address;
    char username[20];
    char password[20];
    while (1)
    {
        printf("Client> ");                     //开始运行程序的时候输出
        scanf("%s %s %d", &option, &IP, &port); //读取第一个操作符
        // if(IP==)
        //printf("%s\n", option);
        //printf("%s\n", IP);

        if (strcpy(option, "open"))
        {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            //printf("%d\n", sockfd);
            address.sin_family = AF_INET; // 域
            //.address.sin_addr.s_addr = inet_addr(IP); // IP地址
            address.sin_port = htons(port); // 端口号
            // 计算长度
            len = sizeof(address);
            inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
            result = connect(sockfd, (struct sockaddr *)&address, len);
            printf("%d\n", result);
            if (result == -1)
            {
                printf("ERROR:Server connection not accepted. Please issue an server connection command.\n");
                continue;
            }
            struct ftpmsg msg;
            memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
            myprintf(msg.m_header);
            msg.m_header.m_type = 0xA1;
            msg.m_header.m_length = 12;
            printf("%d\n",sizeof(msg.m_header));
            send_msg(sockfd, &msg);
            printf("send finish\n");
            recv_msg(sockfd, &msg);
            printf("recv finish\n");
            myprintf(msg.m_header);
            if(msg.m_header.m_type==0xA2&&msg.m_header.m_status==1){
                printf("Server connection accepted.\n");break;
            }
            else{
                printf("ERROR:Server connection not accepted. Please issue an server connection command.\n");
                return -1;
            }
        }
        else
        {
            printf("ERROR:Wrong option! Server connection not accepted. Please issue an server connection command.\n");
            return -1;
        }
    }

    
    // 请求连接
    
    while (1)
    {
        //printf("authready\n");
        printf("Client> "); //试图寻找auth

        scanf("%s %s %s", &option, &username, &password);
        //printf("%s\n", option);
        if (strcpy(option, "auth"))
        {
            printf("%s\n", username);
            printf("%s\n", password);
            struct ftpmsg msg;
            struct ftpmsg returnmsg;
            memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
            msg.m_header.m_type = 0xA3;
            // msg.payload = username;
            // msg.payload[strlen(username)] = ' ';
            
            username[strlen(username)]=' ';printf("show location\n");
            strcat(username,password);
            msg.payload = username;
            msg.m_header.m_length = 12 + strlen(msg.payload) + 1;
            //printf("socket %d\n", sockfd);
            printf("%s\n",msg.payload);
            int result = send_msg(sockfd, &msg); //向sockfd发送open SERVER_IP SERVER_PORT
            printf("send finish %d\n", result);
            recv_msg(sockfd, &returnmsg);
            printf("reveive finish\n");
            if (returnmsg.m_header.m_type == 0xA4 && returnmsg.m_header.m_status == 1)
            {
                printf("Authentication granted.\n");
                break;
            }
            else if (returnmsg.m_header.m_type == 0xA4 && returnmsg.m_header.m_status == 0)
            {
                printf("ERROR: authentication rejected. Connection closed.\n");
            }
            else
            {
                printf("ERROR: authentication rejected. Connection closed.\n");
            }
        }
        else
        {
            printf("ERROR: authentication not started. Please issue an authentication commane.\n");
            continue;
        }
    }
    getchar();
    while (1)
    {
        printf("Client> ");
        //
        char line[100];
        char cmd[100];
        
        while (gets(line) == NULL)
        {
        }
        
        gettoken(cmd, line);
        printf("cmd:%s\n", cmd);printf("line:%s\n", line);
        if (cmd == NULL)
        {
            continue;
        }
        else if (!strcmp(cmd, "ls"))
        {
            //printf("ls in\n");
            c_ls(sockfd);continue;
        }
        else if (!strcmp(cmd, "get"))
        {
            //printf("get in\n");
            c_get(sockfd, line);continue;
        }
        else if (!strcmp(cmd, "put"))
        {
            //printf("put in\n");
            c_put(sockfd, line);continue;
        }
        else if (!strcmp(cmd, "quit"))
        {
            c_quit(sockfd);
            break;
        }
        else
        {
            printf("Wrong option!\n");continue;
        }
    }

    return 0;
}

