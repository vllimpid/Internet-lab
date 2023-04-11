
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
#define DEFAULT_PORT 12323
#define DEFAULT_IP "127.0.0.1"
#define USERNAME "user"
#define PASSWORD "123123"
#define MAXLINE 1024
#define MAX_ARGC 20
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
void myprintf(struct header myheader){
    printf("protocol:%s\n",myheader.m_protocol);
    //printf("like:%d\n",(char)0xA2);
    printf("type:%d\n",myheader.m_type);
    printf("status:%d\n",myheader.m_status);
}


int send_msg(int sockfd, struct ftpmsg *msg)
{
    
    printf("sending message!\n");
    int len=msg->m_header.m_length;
    msg->m_header.m_length=htonl(msg->m_header.m_length);
    send(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    printf("len:%d\n",len);
    if(len == 12){
        return 0;//如果长度为12，说明只有报文头，没有负载
    }
    else {
        //如果长度大于12，说明有负载，根据长度计算出负载的长度进行发送
        
        if(len-12-1>51200){
        
            len=len-12;
            //buf=msg->payload;
            //printf("data:%s\n",msg->payload);
            int times=len/51200+1;
            for(int i=0;i<times;i++){
                if(i==times-1){
                    send(sockfd,Substr(msg->payload,i*51200,(i+1)*51200),51200,0);
                }
                else{
                    send(sockfd,Substr(msg->payload,i*51200,len-i*51200),len-i*51200,0);
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
    // printf(", data = %-30.30s\n", msg->data);
    
    printf("recving message!\n");
    recv(sockfd,&msg->m_header,sizeof(msg->m_header),0);
    
    msg->m_header.m_length=ntohl(msg->m_header.m_length);
    myprintf(msg->m_header);
    printf("recving header\n");
    printf("length: %d\n",msg->m_header.m_length);
    if(msg->m_header.m_length == 12){
        return 0;//如果长度为12，说明只有报文头，没有负载
    }
    else {
        //如果长度大于12，说明有负载，根据长度计算出负载的长度进行发送
        if(msg->m_header.m_length-12>51200){
        
            msg->payload=malloc(msg->m_header.m_length-12);
        
            int len=msg->m_header.m_length-12;
        
            //printf("data:%s\n",msg->payload);
            int times=len/51200+1;
            printf("len:%d times: %d\n",len,times);
            for(int i=0;i<times;i++){
            	char *thisrecv;
            	printf("recving message %d\n",i+1);
            	if(i<times-1){
            		thisrecv=malloc(51201);
            		printf("pause\n");
             		recv(sockfd,thisrecv,51201,0);
            	}
            	else{
            		printf("pause\n");
            		thisrecv=malloc(len-i*51200+1);
                	recv(sockfd,thisrecv,len-i*51200+1,0);
            	}
            	printf("recv succeed!\n");
            	strcat(msg->payload,thisrecv);
        	}
        
    	}
    	else{
    		printf("recv small\n");
    		msg->payload=malloc(msg->m_header.m_length-12);
    		recv(sockfd,msg->payload,msg->m_header.m_length-12,0);//
    		printf("%s\n",msg->payload);
    		
    	}
        return 0;
    }
    
}
int r_open(int sockfd, struct ftpmsg msg)
{
    struct ftpmsg msgreturn;
    memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
    msgreturn.m_header.m_type = 0xA2;
    msgreturn.m_header.m_length = 12;
    msgreturn.m_header.m_status = 1;
    send_msg(sockfd,&msgreturn);
    return 0;
}

int r_ls(int sockfd, struct ftpmsg msg)
{
    //printf("r_ls\n");
    struct ftpmsg msgreturn;
    memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
    msgreturn.m_header.m_type = 0xA6;
    char buf[1024*1024]={};
    FILE *fp = NULL;
    
    fp=NULL;
    fp = popen("ls", "r");
        
    //memset(buf, 0, sizeof(buf));
        
    fread(buf,1024*1024,1,fp);
    printf("buf:%s %d\n",buf,sizeof(buf));
    msgreturn.payload=buf;
    //printf("fp\n");
    int len=strlen(msgreturn.payload);
    msgreturn.payload[len] = '\0';
    pclose(fp);
    msgreturn.m_header.m_length = 12+len+1;
    send_msg(sockfd, &msgreturn);
    return 0;
}

int r_auth(int sockfd, struct ftpmsg msg)
{

    struct ftpmsg msgreturn;
    printf("payload:%s\n",msg.payload);
    if (!strcmp(msg.payload , "user 123123"))
    {
        msgreturn.m_header.m_status = 1;
    }
    else
    {
        msgreturn.m_header.m_status = 0;
    }
    memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
    msgreturn.m_header.m_type = 0xA4;
    msgreturn.m_header.m_length = 12;

    send_msg(sockfd,&msgreturn);
    return 0;
}

int r_get(int sockfd, struct ftpmsg msg)
{
    struct ftpmsg msgreturn;
    memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
    msgreturn.m_header.m_type = 0xA8;
    msgreturn.m_header.m_length = 12;
    int fp=0;
    //printf("filename: %s\n",msg.payload);
    fp=open(msg.payload,O_RDONLY);
    //printf("fp %d\n",fp);
    if (fp!=-1)
    {
        msgreturn.m_header.m_status = 1;
        send_msg(sockfd, &msgreturn);
        int nread;char buf[1024*1024+10];
        struct stat statbuf;
        if (stat(msg.payload, &statbuf) == -1)
        {
            close(fp);
            return -1;
        }
        //msgreturn.payload = malloc(sizeof(msg.payload));
        //sprintf(msgreturn.payload, "%ld", statbuf.st_size);
        msgreturn.m_header.m_type = 0xFF;
        memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
        //先发送第一条指令：文件的大小
        /*sprintf(msgreturn.payload, "%ld", statbuf.st_size);
        
        msgreturn.m_length=12+strlen(msgreturn.payload);
        send_msg(sockfd,&msgreturn);//发送文件的大小，给接收方提供malloc的数据
        //再逐行发送文件，由于文件内容过大，需要分成多个部分发送
        */
        nread = read(fp, buf, sizeof(buf));
        buf[nread]='\0';
        printf("len:%d\n",nread);
        close(fp);
        //printf("data:%s\n",buf);
        msgreturn.m_header.m_length = 12 + nread;
        msgreturn.payload = buf;
        send_msg(sockfd,&msgreturn);
        /*while ((nread = read(fp, buf, sizeof(buf))) != 0)
        {
            if (nread == -1)
            {
                fprintf(stderr, "read函数出错，读取源文件失败\n");
                return -1;
            }
            msgreturn.m_length = 12 + nread;
            msgreturn.payload = buf;
            
            send_msg(sockfd,&msgreturn);
        }*/

        
    }
    else
    {
        msgreturn.m_header.m_status = 0;
        close(fp);
        send_msg(sockfd, &msgreturn);
    }

    return 0;
}

int r_put(int sockfd, struct ftpmsg msg)
{
    int fd=0;
    fd = open(msg.payload,O_WRONLY|O_CREAT,0600);
    struct ftpmsg msgreturn;
    memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
    msgreturn.m_header.m_type = 0xAA;
    msgreturn.m_header.m_length = 12;
    if(fd==-1){
        close(fd);
        return -1;
    }
    else {

        send_msg(sockfd, &msgreturn);
        recv_msg(sockfd, &msg);
        //printf("get data:%s\n",msg.payload);
        printf("len: %d\n",msg.m_header.m_length-12);
        write(fd, msg.payload, msg.m_header.m_length-12);
        close(fd);
        return 0;
    }
        
    
}

int r_quit(int sockfd, struct ftpmsg msg)
{
    struct ftpmsg msgreturn;
    memcpy(msgreturn.m_header.m_protocol, "\xe3myftp", sizeof(msgreturn.m_header.m_protocol));
    msgreturn.m_header.m_type = 0xAC;
    msgreturn.m_header.m_length = 12;
    send_msg(sockfd, &msgreturn);
    return 0;
}

int main(int argc, char const *argv[])
{
    char option[20];
    char IP[20] = DEFAULT_IP;int port = DEFAULT_PORT;
    
    if (argc == 2) // 只有一个参数时
    {
        port = atoi(argv[1]); // 将参数作为端口号
    }
    else if (argc > 2) // 有两个（或更多）参数时
    {
        strcpy(IP, argv[1]);  // 将第一个参数作为IP地址
        port = atoi(argv[2]); // 将第二个参数作为端口号
    }
    //printf("%s\n",IP);
    //printf("%d\n",port);
    int server_sockfd, client_sockfd, server_len, client_len, result;
    struct sockaddr_in server_address, client_address;
    server_sockfd = socket(AF_INET, SOCK_STREAM , 0);
    server_address.sin_family = AF_INET;            // 域
    //server_address.sin_addr.s_addr = inet_addr(IP); // IP地址
    server_address.sin_port = htons(port);          // 端口号
    server_len = sizeof(server_address);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr); 
    result = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    
    if (result == -1)
    {
        fprintf(stderr, "bind函数出错，服务器命名套接字失败\n");
        return -1;
    }
    result = listen(server_sockfd, 128);
    if (result == -1)
    {
        fprintf(stderr, "listen函数出错，服务器创建套接字队列失败\n");
        return -1;
    }
    client_len = sizeof(client_address);

    //int client = accept(server_sockfd , ((void*)0),  ((void*)0));
    int tfd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
    if (tfd == -1)
    {
        //fprintf(stderr, "open函数出错（/dev/tty）\n");
        return -1;
    }
    int connected = 0; // 1表示已连接，0表示未连接
    
    char line[MAXLINE];
    char *cmd[MAX_ARGC];
    //printf("%d\n",connected);
    //printf("---\n");
    while (1)
    {
        if (connected==0)
        {
            client_sockfd = accept(server_sockfd,  (struct sockaddr *)&client_address, (socklen_t *)&client_len);
            if(client_sockfd!=-1) connected = 1;
            //printf("%d %d\n",client_sockfd,connected);
        }
        else
        {
            struct ftpmsg msg;
            printf("waiting...\n");
            recv_msg(client_sockfd, &msg);
            //printf("after recv \n");
            printf("receive type %d\n",msg.m_header.m_type);
            switch (msg.m_header.m_type)
            {
            case 0xA1:
                r_open(client_sockfd, msg);
                break;
            case 0xA3:
                r_auth(client_sockfd, msg);
                break;
            case 0xA5:
                r_ls(client_sockfd, msg);
                break;
            case 0xA7:
                r_get(client_sockfd, msg);
                break;
            case 0xA9:
                r_put(client_sockfd, msg);
                break;
            case 0xAB:
                r_quit(client_sockfd, msg);
                break;
            default:break;
            }
        }
    }
    return 0;
}

