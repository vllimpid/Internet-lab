
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
    int i = 0; 
    // 移除开头的空格
    while (*line == ' ')
    {
        strcpy(line, line + 1);
    }
    if (*line == '\"')
    {

        strcpy(line, line + 1);

        while (*line != '\"' && *line != '\0')
        {
            *token++ = *line;
            i++;
            strcpy(line, line + 1);
        }
        strcpy(line, line + 1); 
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
    
    //printf("sending message!\n");
    int len=msg->m_header.m_length;
    msg->m_header.m_length=htonl(msg->m_header.m_length);
    send(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    //printf("len:%d\n",len);
    if(len == 12){
        return 0;//如果长度为12，说明只有报文头，没有负载
    }
    else {
        //如果长度大于12，说明有负载，根据长度计算出负载的长度进行发送
        //printf("There is payload\n");
        len=len-12;
        
        size_t ret = 0;
		while (ret < len) {
    		size_t b = send(sockfd, msg->payload + ret, len - ret, 0);
    		//if (b == 0) printf("socket Closed"); // 当连接断开
   			if (b < 0) printf("Error ?"); // 这里可能发生了一些意料之外的情况
    		ret += b; // 成功将b个byte塞进了缓冲区
		}
    }
    return 0;
}

int recv_msg(int sockfd, struct ftpmsg *msg)
{
    // printf(", data = %-30.30s\n", msg->data);
    
    //printf("recving message!\n");
    recv(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    
    msg->m_header.m_length=ntohl(msg->m_header.m_length);
    //myprintf(msg->m_header);
    //printf("recving header\n");
    //printf("length: %d\n",msg->m_header.m_length);
    if(msg->m_header.m_length == 12){
        return 0;//如果长度为12，说明只有报文头，没有负载
    }
    else {
        //如果长度大于12，说明有负载，根据长度计算出负载的长度进行发送
        int len=msg->m_header.m_length;
        msg->payload=malloc(msg->m_header.m_length-12);
        len=len-12;
        size_t ret = 0;
		while (ret < len) {
    		size_t b = recv(sockfd, msg->payload + ret, len - ret, 0);
    		//if (b == 0) printf("socket Closed"); // 当连接断开
   			if (b < 0) printf("Error ?"); // 这里可能发生了一些意料之外的情况
    		ret += b; // 成功将b个byte塞进了缓冲区
		}
        //printf("%s\n",msg->payload);return 0;
    }
    
}
void send_big_file(int sockfd,struct ftpmsg *msg,char path[MAXLINE]){
	//printf("sending big message!\n");
    int len=msg->m_header.m_length;
    msg->m_header.m_length=htonl(msg->m_header.m_length);
    send(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    FILE *fp = fopen(path, "rb");
    char send_buff[1024] = "";
    int read_size;
    int cnt=0;
    while ( (read_size = fread(send_buff, sizeof(char), 1024, fp)) > 0)
    {
    	printf("%s\n",send_buff);
    	cnt++;
        if (send(sockfd, send_buff, read_size, 0) < 0)
        {
            break;
        }
        memset(send_buff, '\0', 1024);
    }
    //printf("send %d times\n",cnt);
    fclose(fp);
}
void recv_big_file(int sockfd,struct ftpmsg *msg,char path[1024]){
	//printf("recving big message! %s\n",path);
    recv(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    
    msg->m_header.m_length=htonl(msg->m_header.m_length);
    int len=msg->m_header.m_length-13;
    FILE *fp = fopen(msg->payload, "ab");
    if(fp == NULL)
    {
        printf("file connot be open\n");
    } 
    else
    {
        char recv_buf[1024] = "";
        int recv_size = 0;
        int total_length = 0;
        while (1)
        {
            recv_size = recv(sockfd, recv_buf, 1024, 0);
            
            if (recv_size <= 0)
            {
                printf("recv error\n");
                break;
            }
            total_length += recv_size;
            int write_buf = fwrite(recv_buf, sizeof(char), recv_size, fp);
            if (write_buf < recv_size)
            {
                printf("file write error\n");
            }
            //printf("recv finish %d\n",total_length);
            /*接收完整个数据包之后，退出*/
            if (total_length >=len ) break;
            memset(recv_buf, '\0', 1024);
        }
        //printf("write finish\n");
        fclose(fp);
    }

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
    	recv_big_file(sockfd,&returnmsg,msg.payload);
        /*int i = 0, nwrite;
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
        */
        printf("File downloaded\n");
        //close(fd);
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
        if(nread>5000)
        	send_big_file(sockfd,&msgreturn,path);
        else
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
            inet_pton(AF_INET, IP, &address.sin_addr);
            result = connect(sockfd, (struct sockaddr *)&address, len);
            //printf("%d\n", result);
            if (result == -1)
            {
                printf("ERROR:Server connection not accepted. Please issue an server connection command.\n");
                continue;
            }
            struct ftpmsg msg;
            memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
            //myprintf(msg.m_header);
            msg.m_header.m_type = 0xA1;
            msg.m_header.m_length = 12;
            //printf("%d\n",sizeof(msg.m_header));
            send_msg(sockfd, &msg);
            //printf("send finish\n");
            recv_msg(sockfd, &msg);
            //printf("recv finish\n");
            //myprintf(msg.m_header);
            if(msg.m_header.m_type==0xA2&&msg.m_header.m_status==1){
                printf("Server connection accepted.\n");break;
            }
            else{
                printf("ERROR:Server connection not accepted. Please issue an server connection command.\n");
                continue;//return -1;
            }
        }
        else
        {
            printf("ERROR:Wrong option! Server connection not accepted. Please issue an server connection command.\n");
            continue;//return -1;
        }
    }

    
    // 请求连接
    
    while (1)
    {
        //printf("authready\n");
        printf("Client> "); //试图寻找auth
	char username[20]={};
        char password[20]={};
        
        scanf("%s %s %s", &option, &username, &password);
        //printf("%s\n", option);
        if (strcpy(option, "auth"))
        {
            //printf("%s\n", username);
            //printf("%s\n", password);
            struct ftpmsg msg;
            struct ftpmsg returnmsg;
            memcpy(msg.m_header.m_protocol, "\xe3myftp", sizeof(msg.m_header.m_protocol));
            msg.m_header.m_type = 0xA3;
            // msg.payload = username;
            // msg.payload[strlen(username)] = ' ';
            
            username[strlen(username)]=' ';//printf("show location\n");
            strcat(username,password);
            msg.payload = username;
            msg.m_header.m_length = 12 + strlen(msg.payload) + 1;
            //printf("socket %d\n", sockfd);
            printf("%s\n",msg.payload);
            int result = send_msg(sockfd, &msg); //向sockfd发送open SERVER_IP SERVER_PORT
            //printf("send finish %d\n", result);
            recv_msg(sockfd, &returnmsg);
            //printf("reveive finish\n");
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

