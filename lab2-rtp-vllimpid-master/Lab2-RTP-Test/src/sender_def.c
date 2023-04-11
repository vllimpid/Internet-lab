#include<stdio.h>   
#include<stdint.h> 
#include<stdlib.h>   
#include<time.h>   
#include<math.h>  
#include<stdlib.h>
#include <arpa/inet.h>
#include<netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#define START 0
#define END 1
#define DATA 2
#define ACK 3
#define PAYLOAD_SIZE 1461
typedef struct __attribute__ ((__packed__)) RTP_Header {
    uint8_t type;       // 0: START; 1: END; 2: DATA; 3: ACK
    uint16_t length;    // Length of data; 0 for ACK, START and END packets
    int seq_num;
    uint32_t checksum;  // 32-bit CRC
} rtp_header_t;
struct rtpmsg{
	rtp_header_t m_header;
	char payload[PAYLOAD_SIZE];
};
#define SEND_TIMEOUT 500//定义超时长度为500ms   
/**
 * @brief 用于建立RTP连接
 * @param receiver_ip receiver的IP地址
 * @param receiver_port receiver的端口
 * @param window_size window大小
 * @return -1表示连接失败，0表示连接成功
 **/
int min(int a,int b){
	if(a>b) return b;
	else return a;
}
int sockfd=0;
int seqtotal=0;
int gittime=2;
int windowsize=0;
int sending=0;
int acking=0;struct sockaddr_in addr;
int initSender(const char* receiver_ip, uint16_t receiver_port, uint32_t window_size){
	printf("----initsender----\n");
	sockfd=0;
	seqtotal=0;
	printf("windowsize:%d\n",window_size);
	sending=0;acking=0;
	sockfd=socket(PF_INET, SOCK_DGRAM, 0);
	struct timeval tv_out;
	tv_out.tv_sec = 1;//等待10秒
	tv_out.tv_usec = 0;
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));


	int opt=1;int len1 = sizeof(opt);
    //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, &len1);
	windowsize=window_size;
	//set up a socket
	printf("//ip:%s port:%d\n",receiver_ip,receiver_port);
	//地址信息赋值
	addr.sin_family = AF_INET;
	addr.sin_port = htons(receiver_port);//将主机字节序短整型型数据转化为网络字节序数据
	//addr.sin_addr.s_addr = INADDR_ANY;//将字符串IP地址转化为网络字节序IP地址
	printf("port1:%d port2:%d\n",addr.sin_port,ntohs(addr.sin_port));
    //addr.sin_addr.s_addr = inet_addr(receiver_ip);
	int len = sizeof(addr);
	inet_pton(AF_INET, receiver_ip, &addr.sin_addr.s_addr);
	struct rtpmsg msg,returnmsg;
	msg.m_header.type=START;
	msg.m_header.length=0;
	srand((unsigned)time(NULL));
	msg.m_header.seq_num=rand();
	msg.m_header.checksum=0;
	msg.m_header.checksum=compute_checksum(&msg.m_header, sizeof(msg.m_header));
	printf("//before sending!\n");
	printf("//sender:%d\n",msg.m_header.seq_num);
	int sendnum=sendto(sockfd,&msg.m_header,sizeof(msg.m_header),0, (struct sockaddr*)&addr,sizeof(addr));
	printf("//sending successfully! waiting for message!\n");
	printf("//sender:%d\n",sendnum);
	char recvbuff[2]={};
	recvfrom(sockfd,&returnmsg.m_header,sizeof(returnmsg.m_header),0,NULL,NULL);
	printf("//sender recv from receiver %s\n",recvbuff);
	if(returnmsg.m_header.type==ACK&&returnmsg.m_header.seq_num==msg.m_header.seq_num){
		//open succeed!
		printf("//open succeed\n");
		return 0;
		
	}
	else
		terminateSender();
	
	
}


/**
 * @brief 用于发送数据 
 * @param message 要发送的文件名
 * @return -1表示发送失败，0表示发送成功
 **/
int sendMessage(const char* message){
	printf("----sendmessage----\n");
	printf("// we will send message %s\n",message);
	FILE *fd=fopen(message,"rb");
	if(fd==NULL){
		printf("//open file failed\n");
		return -1;
	}
	//return 0;
	int low=0;
	int high=windowsize;
	int len = sizeof(addr);
	int cnt=0;
	int readid[100000]={};int sendid[100000]={};
	int recvid[100000]={};
	memset(recvid,0,sizeof(recvid));
	int totaltimes=100000;
	char send_buff[1030][1500] = {};int read_size=0;
	int lenofdata[1030]={};
	while(1){
		int resend=0;
		int senttimes=0;
		
		
		//memset(send_buff,0,sizeof(send_buff));
		for(int i=cnt;i<cnt+windowsize;i++){
			int j=i-cnt;
			if(recvid[i]==1) continue;
			if(readid[i]!=1){
				read_size = fread(send_buff[j], sizeof(char), 1461, fd);
				lenofdata[j]=read_size;
				if(read_size<=0){
					printf("//read failed\n");
					totaltimes=i-1;
					seqtotal=i-1;//return 0;
					break;
					
				}
				//printf("read_size:%d\n",read_size);
				//printf("//read No.%d\n",i);
				readid[i]=1;
				
			}
			
			struct rtpmsg msg;

			msg.m_header.length=0;
			//printf("//strlen:%d %d\n",strlen(send_buff[j]),read_size);
			msg.m_header.seq_num=0;
			memset(msg.payload,0,sizeof(msg.payload));
			msg.m_header.type=DATA;
			msg.m_header.length=lenofdata[j];
			//printf("//strlen:%d %d\n",strlen(send_buff[j]),read_size);
			msg.m_header.seq_num=i;
			//seqtotal++;
			//sending++;
			sendid[i]=1;
			msg.m_header.checksum=0;
			memcpy(msg.payload,send_buff[j],msg.m_header.length);
			
			msg.m_header.checksum=compute_checksum(&msg,sizeof(msg.m_header)+msg.m_header.length);
			//printf("//detail: %s\n",msg.payload);
			//send_msg(sockfd,&msg,addr);
			
			sendto(sockfd,&msg,sizeof(msg.m_header)+msg.m_header.length,0, (struct sockaddr*)&addr,sizeof(addr));
			//printf("send seq:%d with len %d\n",msg.m_header.seq_num,msg.m_header.length);
			senttimes++;
		}//发送和窗口大小一样的报文数量
		//printf("//totaltimes:%d\n",totaltimes);
		//printf("//senttimes:%d\n",senttimes);
		//printf("//cnt:%d\n",cnt);
		//printf("//cnt+senttimes:%d\n",cnt+senttimes);
		clock_t start,end;
		start=clock();
		//printf("senttimes%d\n",senttimes);
		if(cnt>=totaltimes) break;
		int firstnum=-1,secondnum=-1,thirdnum=-1;
		for(int i=cnt;i<cnt+senttimes;i++){
			//printf("//goto check %d\n",i);
			end=clock();
			if(end-start>=80){
				resend=1;
				//printf("timeout----------------------------------------\n");
				break;
			}
			struct rtpmsg returnmsg;
			//recv_msg(sockfd,&returnmsg,addr);
			returnmsg.m_header.seq_num=0;
			
			recvfrom(sockfd,&returnmsg.m_header,sizeof(returnmsg.m_header),0,NULL,NULL);
			//printf("//ACK?%d\n",returnmsg.m_header.type);
			
			if(returnmsg.m_header.type==ACK){
				//printf("//we receive ack %d\n",returnmsg.m_header.seq_num-1);
				uint32_t testchecksum;
				testchecksum=0;
				uint32_t recv_checksum=returnmsg.m_header.checksum;
				returnmsg.m_header.checksum=0;
				testchecksum=compute_checksum(&returnmsg.m_header, sizeof(returnmsg.m_header));
				if(testchecksum==recv_checksum){
					
					recvid[returnmsg.m_header.seq_num-1]=1;
				}
				//else printf("checksum not ok\n");
				//if(recvid[returnmsg.m_header.seq_num-2]==0&&recvid[returnmsg.m_header.seq_num-1]==1){
					for(int k=returnmsg.m_header.seq_num-2;k>=0;k--){
						if(recvid[k]==0){
							recvid[k]=1;//printf("we ingore ack %d\n",k);
						}
						else break;
					}
					
					
				
				if(returnmsg.m_header.seq_num==cnt+senttimes){
					for(int l=cnt;l<returnmsg.m_header.seq_num;l++){
						recvid[l]=1;
					}
					break;	
				}
				if(returnmsg.m_header.seq_num>=cnt+senttimes) break;
				if(firstnum==-1){
					firstnum=returnmsg.m_header.seq_num-1;
				}
				else if(secondnum==-1){
					secondnum=firstnum;
					firstnum=returnmsg.m_header.seq_num-1;
				}
				else {
					thirdnum=secondnum;
					secondnum=firstnum;
					firstnum=returnmsg.m_header.seq_num-1;
				}
				if(firstnum==secondnum&&secondnum==thirdnum&&firstnum!=-1){
					//printf("lost data\n");
					break;
				}
			}
			
		}//尝试接收报文，超时则取消
		//printf("//beforecnt:%d\n",cnt);
		if(resend==1) continue;
		//printf("we not timeout\n");
		int recvok=0;
		for(int i=cnt;i<cnt+senttimes;i++){
			if(recvid[i]==1){
				recvok++;
			}
			else break;
			
		}
		//printf("revock:%d\n",recvok);
		
		if(recvok==0){
			continue;
			
		}
		
		for(int j=cnt;j<cnt+windowsize;j++){
				
			memset(send_buff[j-cnt],0,lenofdata[j-cnt]);
			memcpy(send_buff[j-cnt],send_buff[j+recvok-cnt],lenofdata[j-cnt+recvok]);
			lenofdata[j-cnt]=lenofdata[j+recvok-cnt];
			memset(send_buff[j-cnt+recvok],0,lenofdata[j-cnt+recvok]);
			lenofdata[j-cnt+recvok]=0;
				//将整个data记录的窗口左移
		}
		cnt=cnt+recvok;
		//printf("//cnt:%d\n",cnt);
		if(cnt>=totaltimes) break;
		//cnt=cnt+senttimes;

		
		
		
		
		//return 0;
	}
	fclose(fd);
	printf("//send end\n");
	return 0;
}




int sendMessageOpt(const char* message){
	printf("----sendmessageopt----\n");
	//return 0;
	printf("// we will send message %s\n",message);
	FILE *fd=fopen(message,"rb");
	if(fd==NULL){
		printf("//open file failed\n");
		return -1;
	}
	//return 0;
	int low=0;
	int high=windowsize;
	int len = sizeof(addr);
	int cnt=0;
	int readid[100000]={};int sendid[100000]={};
	int recvid[100000]={};
	memset(recvid,0,100000);
	int totaltimes=100000;
	char send_buff[1030][1500] = {};int read_size=0;
	int lenofdata[1030]={};
	clock_t start,end;
	while(1){
		int resend=0;
		int senttimes=0;
		
		
		//memset(send_buff,0,sizeof(send_buff));
		for(int i=cnt;i<min(totaltimes+1,cnt+windowsize);i++){
			int j=i-cnt;
			if(recvid[i]==1) continue;
			if(readid[i]!=1){
				read_size = fread(send_buff[j], sizeof(char), 1461, fd);
				lenofdata[j]=read_size;
				if(read_size<=0){
					//printf("//read failed\n");
					totaltimes=i-1;
					seqtotal=i-1;
					break;
					
				}readid[i]=1;
				//printf("read_size:%d\n",read_size);
				//printf("//read No.%d\n",i);
				
			}
			struct rtpmsg msg;
			memset(msg.payload,0,sizeof(msg.payload));
			msg.m_header.type=DATA;
			msg.m_header.length=lenofdata[j];
			msg.m_header.seq_num=i;
			sendid[i]=1;
			msg.m_header.checksum=0;
			memcpy(msg.payload,send_buff[j],msg.m_header.length);
			msg.m_header.checksum=compute_checksum(&msg,sizeof(msg.m_header)+msg.m_header.length);
			sendto(sockfd,&msg,sizeof(msg.m_header)+msg.m_header.length,0, (struct sockaddr*)&addr,sizeof(addr));
			//printf("send seq:%d with len %d\n",msg.m_header.seq_num,msg.m_header.length);
			senttimes++;
		}//发送和窗口大小一样的报文数量
		//printf("//totaltimes:%d\n",totaltimes);
		//printf("//senttimes:%d\n",senttimes);
		//printf("//cnt:%d\n",cnt);
		//printf("//cnt+senttimes:%d\n",cnt+senttimes);
		
		//printf("senttimes%d\n",senttimes);
		start=clock();
		
		for(int i=0;i<senttimes;i++){
			//printf("//goto check %d\n",i);
			if(clock()-start>100){
			
				//printf("timeout _______________________________\n");
				break;
			}
			struct rtpmsg returnmsg;
			//recv_msg(sockfd,&returnmsg,addr);
			returnmsg.m_header.seq_num=0;
			returnmsg.m_header.type=0;
			recvfrom(sockfd,&returnmsg.m_header,sizeof(returnmsg.m_header),0,NULL,NULL);
			//printf("//ACK?%d\n",returnmsg.m_header.type);
			
			if(returnmsg.m_header.type==0) continue;
			if(returnmsg.m_header.type==ACK){
				
				//printf("%d to %d\n",cnt,cnt+windowsize);
				/*if(returnmsg.m_header.seq_num<cnt||returnmsg.m_header.seq_num>=cnt+windowsize||returnmsg.m_header.seq_num>totaltimes){
					printf("out of size %d\n",returnmsg.m_header.seq_num);
					continue;
					
				}*/
				uint32_t testchecksum;
				testchecksum=0;
				uint32_t recv_checksum=returnmsg.m_header.checksum;
				returnmsg.m_header.checksum=0;
				testchecksum=compute_checksum(&returnmsg.m_header, sizeof(returnmsg.m_header));
				if(testchecksum!=recv_checksum){
					//printf("wrong checksum\n");
					continue;
				}
				//else printf("checksum not ok\n");
				recvid[returnmsg.m_header.seq_num]=1;
				//printf("//we receive ack %d\n",returnmsg.m_header.seq_num);
				//if(returnmsg.m_header.seq_num==cnt+windowsize-1) break;
				
			}
			//else break;
			
		}//尝试接收报文，超时则取消
		//printf("//beforecnt:%d\n",cnt);
		
		if(cnt>totaltimes){
		
			//printf("%d %d\n",totaltimes,recvid[totaltimes]);
			break;
		}
		int recvok=0;
		for(int i=cnt;i<min(cnt+windowsize,totaltimes+1);i++){
			if(recvid[i]==1){
				recvok++;
			}
			else break;
			
		}
		//printf("revock:%d\n",recvok);
		if(recvok==0){
			continue;
			
		}
		
		for(int j=cnt;j<min(cnt+windowsize,totaltimes+1);j++){
				
			memset(send_buff[j-cnt],0,lenofdata[j-cnt]);
			memcpy(send_buff[j-cnt],send_buff[j+recvok-cnt],lenofdata[j-cnt+recvok]);
			lenofdata[j-cnt]=lenofdata[j+recvok-cnt];
			memset(send_buff[j-cnt+recvok],0,lenofdata[j-cnt+recvok]);
			lenofdata[j-cnt+recvok]=0;
			//start[j-cnt]=start[j+recvok-cnt];
				//将整个data记录的窗口左移
		}
		cnt=cnt+recvok;
		//printf("//cnt:%d\n",cnt);
		
		
		if(cnt>totaltimes){
		
			//printf("%d %d\n",totaltimes,recvid[totaltimes]);
			break;
		}
		
		
		//cnt=cnt+senttimes;

		
		
		
		
		//return 0;
	}
	fclose(fd);
	printf("//send end\n");
	return 0;
}

/**
 * @brief 用于断开RTP连接以及关闭UDP socket
 **/
void terminateSender(){
	printf("----terminatesender----\n");
	printf("seq:%d\n",seqtotal);
	if(sockfd!=-1){
		struct rtpmsg msg,returnmsg;
		msg.m_header.type=END;
		msg.m_header.length=0;
		
		msg.m_header.seq_num=seqtotal+1;
		//msg.payload=0;
		//int len=sizeof(addr);
		msg.m_header.checksum=0;
		msg.m_header.checksum=compute_checksum(&msg.m_header,sizeof(msg.m_header));
		printf("before terminal send\n");
		sendto(sockfd,&msg.m_header,sizeof(msg.m_header),0, (struct sockaddr*)&addr,sizeof(addr));
		//send_msg(sockfd,&msg,addr);
		printf("after terminal send\n");
		clock_t start,end;
		start=clock();
		//recv_msg(sockfd,&returnmsg,addr);
		recvfrom(sockfd,&returnmsg.m_header,sizeof(returnmsg.m_header),0,NULL,NULL);
		if(returnmsg.m_header.type==ACK&&returnmsg.m_header.seq_num==msg.m_header.seq_num){
		//open succeed!
			printf("//close succeed\n");
			//shutdown(sockfd,SHUT_RDWR);
			close(sockfd);
		
		}
		else if(clock()-start>100){
			close(sockfd);
		}
		
		
		
	}
	return 0;
}


