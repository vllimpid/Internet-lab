#include<stdio.h>   
#include<stdint.h>  
#include<stdlib.h>   
#include<time.h>   
#include<math.h>  
#include<stdlib.h>
#include <arpa/inet.h>

#define START 0
#define END 1
#define DATA 2
#define ACK 3


#define PAYLOAD_SIZE 1461
#define SEND_TIMEOUT 500//定义超时长度为500ms   
/**
 * @brief 用于建立RTP连接
 * @param receiver_ip receiver的IP地址
 * @param receiver_port receiver的端口
 * @param window_size window大小
 * @return -1表示连接失败，0表示连接成功
 **/


typedef struct __attribute__ ((__packed__)) RTP_header {
    uint8_t type;       // 0: START; 1: END; 2: DATA; 3: ACK
    uint16_t length;    // Length of data; 0 for ACK, START and END packets
    uint32_t seq_num;
    uint32_t checksum;  // 32-bit CRC
} rtp_header_t;


typedef struct __attribute__ ((__packed__)) RTP_packet {
    rtp_header_t rtp;
    char payload[PAYLOAD_SIZE];
} rtp_packet_t;

typedef struct __attribute__ ((__packed__)) rtpmsg{
	rtp_header_t rtp;
	char payload[PAYLOAD_SIZE];
};

struct data{
	int num;
	int ok;
	char detail[PAYLOAD_SIZE];
	int length;
};
/**
 * @brief 开启receiver并在所有IP的port端口监听等待连接
 * 
 * @param port receiver监听的port
 * @param window_size window大小
 * @return -1表示连接失败，0表示连接成功
 */



int recvwindows=0;
int connected=0;

int server_sockfd, client_sockfd, server_len, client_len, result;
struct sockaddr_in server_address, client_address;
int initReceiver(uint16_t port, uint32_t window_size){
	printf("----initreceiver----\n");
	printf("server port :%d\n",port);
	recvwindows=window_size;
	connected=0;
	server_sockfd=0;
	client_sockfd=0;
	server_len=0;client_len=0;
	memset(&server_address,0,sizeof(server_address));
	memset(&client_address,0,sizeof(client_address));
    server_sockfd = socket(PF_INET, SOCK_DGRAM , 0);
    server_address.sin_port = htons(port);          // 端口号
    server_address.sin_family = AF_INET;            // 域
    server_address.sin_addr.s_addr = INADDR_ANY;  //自动获取IP地址
    server_len = sizeof(server_address);
    //inet_pton(AF_INET, IP, &server_address.sin_addr); 
    int ret = bind(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address));//bind address
	printf("ret:%d\n",ret);
	struct timeval tv_out;
	tv_out.tv_sec = 1;//等待10秒
	tv_out.tv_usec = 0;
	setsockopt(server_sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));

	if(ret==-1)
		return -1;//if bind fail, return -
	struct RTP_packet msg;
	printf("recviver:\n");char ipBuf[16];socklen_t size = sizeof(client_address);
	int recvnum=recvfrom(server_sockfd,&msg.rtp,sizeof(msg.rtp), 0, (struct sockaddr*)&client_address, &size);printf("recviver:%d\n",recvnum);
	printf("msg.type=%d msg.seq=%d checksum%d\n",msg.rtp.type,msg.rtp.seq_num,msg.rtp.checksum);
	printf("ip=%s,port=%d\n",inet_ntop(AF_INET, &client_address.sin_addr.s_addr, ipBuf, sizeof(ipBuf)),ntohs(client_address.sin_port));
	uint32_t testchecksum;
	testchecksum=0;
	uint32_t recv_checksum=msg.rtp.checksum;
	msg.rtp.checksum=0;
	testchecksum=compute_checksum(&msg, sizeof(msg.rtp)+msg.rtp.length);
	if(testchecksum!=recv_checksum){
		terminateReceiver();
		return -1;
	}
	if(msg.rtp.type==START){
		struct RTP_packet returnmsg;
		returnmsg.rtp.type=ACK;
		returnmsg.rtp.length=0;
		returnmsg.rtp.seq_num=msg.rtp.seq_num;
		returnmsg.rtp.checksum=0;
		returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
		memset(returnmsg.payload,0,sizeof(returnmsg.payload));
		sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
		
	}
	printf("come to end \n");
	return 0;
}

/**
 * @brief 用于接收数据并在接收完后断开RTP连接
 * @param filename 用于接收数据的文件名
 * @return >0表示接收完成后到数据的字节数 -1表示出现其他错误
 */


int recvMessage(char* filename){
	printf("----recvmessage----\n");
	printf("we will recv message %s\n",filename);
	FILE *fd=fopen(filename,"wb");
	if(fd==NULL){
		printf("open file failed\n");
		return -1;
	}
	else
		printf("open succeed\n");
	
	int low=0;
	int high=recvwindows;
	int sendid[100000]={};
	int recvid[100000]={};
	int cnt=0;
	int readid[100000]={};
	//printf("set mydata\n");
	struct data mydata[1030];
	memset(recvid,0,sizeof(recvid));
	int totaltimes=0;
	int lastseq=-1;
	int len[100000]={};
	int load[100000]={};

	while(1){
		int randtimes=0;
		
		//printf("cnt:%d\n",cnt);
		for(int i=cnt;i<cnt+recvwindows;i++){
			struct rtpmsg msg;
			socklen_t size = sizeof(client_address);
			clock_t recvtime=clock();
			recvfrom(server_sockfd,&msg,sizeof(msg), 0, (struct sockaddr*)&client_address, &size);
			/*if(clock()-recvtime>10000){
				terminateReceiver();
				return -1;
			}*/
			//recv_msg1(client_sockfd,&msg,client_address)
			if(msg.rtp.type==END){
				//printf("randtimes:%d\n",randtimes);
				printf("turn to end function!\n");
				//return 0;
				fclose(fd);
				struct rtpmsg returnmsg;
				returnmsg.rtp.type=ACK;
				returnmsg.rtp.length=0;
				
				returnmsg.rtp.seq_num=msg.rtp.seq_num;
				returnmsg.rtp.checksum=0;
				returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
				//send_msg1(client_sockfd,&returnmsg,client_address);
				sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
				connected=0;//return 0;
				//close(client_sockfd);
				close(server_sockfd);
				return 0;
			}
			int number=msg.rtp.seq_num;
			//printf("number:%d\n",number);
			//printf("checksum %d\n",msg.rtp.checksum);
			struct rtpmsg returnmsg;
			returnmsg.rtp.type=ACK;
			returnmsg.rtp.length=0;
			int right=0;
			uint32_t testchecksum;
			testchecksum=0;
			uint32_t recv_checksum=msg.rtp.checksum;
			msg.rtp.checksum=0;
			testchecksum=compute_checksum(&msg, sizeof(msg.rtp)+msg.rtp.length);
			if(recv_checksum!=testchecksum)
				continue;
			if(lastseq+recvwindows<=msg.rtp.seq_num){
				continue;
			}
			if(lastseq+1!=msg.rtp.seq_num){
				//如果我们现在收到的报文不是我们想要的，我们依然接收，但是我们会对他们进行缓存
				if(recvid[number]==0){
					mydata[number-cnt].num=number;
					memcpy(mydata[number-cnt].detail,msg.payload,msg.rtp.length);
					mydata[number-cnt].length=msg.rtp.length;
					mydata[number-cnt].ok=2;
					randtimes++;
					//recvid[number]=1;
					returnmsg.rtp.seq_num=lastseq+1;
					returnmsg.rtp.checksum=0;
					returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
				//send_msg1(client_sockfd,&returnmsg,client_address);
					//printf("wrong! we load it and wait for the right udp! seq we send%d\n",returnmsg.rtp.seq_num);
					sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
				//printf("reserve %d\n",number-cnt);
				}
				
				
				if(number-cnt>1000){
					return 0;
				}
				
				
				right=0;
				//continue;
			}
			else {//此时保证报文的序号是我们需要的
				
				//printf("testchecksum:%d\n",testchecksum);
					mydata[number-cnt].num=number;
					memcpy(mydata[number-cnt].detail,msg.payload,msg.rtp.length);
					mydata[number-cnt].length=msg.rtp.length;
					mydata[number-cnt].ok=1;
					recvid[number]=1;
					totaltimes++;
					randtimes++;
				
				//lastseq=msg.rtp.seq_num;
				int k=0;
				for(k=number+1;k<cnt+recvwindows;k++){
					int k1=k-cnt;
					if(mydata[k1].ok==2){
						mydata[k1].ok=1;
						recvid[k]=1;
						totaltimes++;//randtimes++;
						//printf("we reload number %d with len %d\n",k,mydata[k1].length);
						
						
					}
					else{
						break;
					}
						
				}
				int num;
				int cal=0;
				for(num=0;;num++){
					if(recvid[num]!=0){
						cal++;
						//printf("%d\n",num);
					}
					else break;
				}
				lastseq=cal-1;
				returnmsg.rtp.seq_num=cal;
				returnmsg.rtp.checksum=0;
				returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
				//send_msg1(client_sockfd,&returnmsg,client_address);
				//printf("seq we send%d\n",returnmsg.rtp.seq_num);
				sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
				
				//printf("lastseq%d\n",lastseq);
			}
			int move=0;
			for(int j=cnt;;j++){
				if(mydata[j-cnt].ok==1){
					move++;//记录一下有多少个可以写入
				}
				else break;
			}
			
			for(int j=cnt;j<cnt+move;j++){
				//printf("write %d\n",mydata[j-cnt].num);
				fwrite(mydata[j-cnt].detail,sizeof(char),mydata[j-cnt].length,fd);
			}
			if(move==0)continue;
			for(int j=cnt;j<cnt+recvwindows;j++){
				
				mydata[j-cnt].num=mydata[j+move-cnt].num;
				mydata[j-cnt].length=mydata[j+move-cnt].length;
				mydata[j-cnt].ok=mydata[j+move-cnt].ok;
				memset(mydata[j-cnt].detail,0,mydata[j-cnt].length);
				memcpy(mydata[j-cnt].detail,mydata[j+move-cnt].detail,mydata[j+move-cnt].length);
				
				memset(mydata[j+move-cnt].detail,0,mydata[j+move-cnt].length);
				mydata[j+move-cnt].num=0;
				mydata[j+move-cnt].length=0;
				mydata[j+move-cnt].ok=0;
				
				//将整个data记录的窗口左移
			}
			cnt=cnt+move;
			//printf("cnt:%d\n",cnt);
			//printf("move %d seqlast%d\n",move,lastseq);
			if(move>0) break;
			
		}
		
		
		
		
		
	}
	printf("recv end\n");
	return 0;
}


int recvMessageOpt(char* filename){
	printf("----recvmessageopt----\n");
	printf("we will recv message %s\n",filename);
	FILE *fd=fopen(filename,"ab");
	if(fd==NULL){
		printf("open file failed\n");
		return -1;
	}
	else
		printf("open succeed\n");
	
	int low=0;
	int high=recvwindows;
	int recvid[100000]={};
	int cnt=0;
	//printf("set mydata\n");
	struct data mydata[1025];
	memset(recvid,0,100000);
	int totaltimes=0;
	int lastseq=-1;
	int len[10000]={};
	while(1){
		int randtimes=0;
		//printf("cnt:%d\n",cnt);
		for(int i=cnt;i<cnt+recvwindows;i++){
			struct rtpmsg msg;
			socklen_t size = sizeof(client_address);
			recvfrom(server_sockfd,&msg,sizeof(msg), 0, (struct sockaddr*)&client_address, &size);
			/*if(clock()-recvtime>10000){
				terminateReceiver();
				return -1;
			}*/
			//recv_msg1(client_sockfd,&msg,client_address);
			
			if(msg.rtp.type==END){
				//printf("randtimes:%d\n",randtimes);
				
				//printf("turn to end function!\n");
				//return 0;
				fclose(fd);
				struct rtpmsg returnmsg;
				returnmsg.rtp.type=ACK;
				returnmsg.rtp.length=0;
				
				returnmsg.rtp.seq_num=msg.rtp.seq_num;
				returnmsg.rtp.checksum=0;
				returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
				//send_msg1(client_sockfd,&returnmsg,client_address);
				sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
				connected=0;//return 0;
				//close(client_sockfd);
				close(server_sockfd);
				return 0;
			}
			int number=msg.rtp.seq_num;
			//printf("number:%d\n",number);
			//printf("checksum %d\n",msg.rtp.checksum);
			struct rtpmsg returnmsg;
			returnmsg.rtp.type=ACK;
			returnmsg.rtp.length=0;
			int right=0;
			uint32_t testchecksum;
			testchecksum=0;
			uint32_t recv_checksum=msg.rtp.checksum;
			msg.rtp.checksum=0;
			testchecksum=compute_checksum(&msg, sizeof(msg.rtp)+msg.rtp.length);
			//printf("testchecksum:%d\n",testchecksum);
			if(recvid[number]!=0) continue;
			if(recv_checksum!=testchecksum)
				continue;
			if(cnt+recvwindows<=msg.rtp.seq_num){
				continue;
			}
			else if(msg.rtp.seq_num<cnt){
			continue;
				printf("lower\n");
				returnmsg.rtp.seq_num=number;
				returnmsg.rtp.checksum=0;

				returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
				//send_msg1(client_sockfd,&returnmsg,client_address);
				//printf("77wrong! we load it and wait for the right udp! seq we send%d\n",returnmsg.rtp.seq_num);
				sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
				
			}
			else {//此时保证报文的序号是我们需要的
				
				returnmsg.rtp.seq_num=msg.rtp.seq_num;
				returnmsg.rtp.checksum=0;
				returnmsg.rtp.checksum=compute_checksum(&returnmsg.rtp, sizeof(returnmsg.rtp));
				//send_msg1(client_sockfd,&returnmsg,client_address);
				//printf("seq we send%d\n",returnmsg.rtp.seq_num);
				sendto(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
				memset(mydata[number-cnt].detail,0,sizeof(mydata[number-cnt].detail));
				mydata[number-cnt].num=number;
				memcpy(mydata[number-cnt].detail,msg.payload,msg.rtp.length);
				mydata[number-cnt].length=msg.rtp.length;
				mydata[number-cnt].ok=1;
				recvid[number]=1;
				totaltimes++;
				randtimes++;
				//lastseq=msg.rtp.seq_num;
				//lastseq=number+cal-1;
				//printf("lastseq%d\n",lastseq);
			}
			int move=0;
			for(int j=cnt;;j++){
				if(mydata[j-cnt].ok==1){
					move++;//记录一下有多少个可以写入
					fwrite(mydata[j-cnt].detail,sizeof(char),mydata[j-cnt].length,fd);
				}
				else break;
			}
			
			
			if(move==0)continue;
			for(int j=cnt;j<cnt+recvwindows;j++){
				
				mydata[j-cnt].num=mydata[j+move-cnt].num;
				mydata[j-cnt].length=mydata[j+move-cnt].length;
				mydata[j-cnt].ok=mydata[j+move-cnt].ok;
				memset(mydata[j-cnt].detail,0,mydata[j-cnt].length);
				memcpy(mydata[j-cnt].detail,mydata[j+move-cnt].detail,mydata[j+move-cnt].length);
				
				memset(mydata[j+move-cnt].detail,0,mydata[j+move-cnt].length);
				mydata[j+move-cnt].num=0;
				mydata[j+move-cnt].length=0;
				mydata[j+move-cnt].ok=0;
				
				//将整个data记录的窗口左移
			}
			cnt=cnt+move;
			//printf("cnt:%d\n",cnt);
			//printf("move %d seqlast%d\n",move,lastseq);
			if(move>0) break;
			
		}
		
		
		
		
		
	}
	printf("recv end\n");
	return 0;
}


/**
 * @brief 用于接收数据并在接收完后断开RTP连接 (优化版本的RTP)
 * @param filename 用于接收数据的文件名
 * @return >0表示接收完成后到数据的字节数 -1表示出现其他错误
 */

/**
 * @brief 用于接收数据失败时断开RTP连接以及关闭UDP socket
 */
void terminateReceiver(){
	printf("----terminatereceiver----\n");
	if(connected==1){
		printf("here close sockfd\n");
		struct rtpmsg msg,returnmsg;
		socklen_t size = sizeof(client_address);
		recvfrom(server_sockfd,&returnmsg.rtp,sizeof(returnmsg.rtp), 0, (struct sockaddr*)&client_address, &size);
		if(returnmsg.rtp.type==END){
			msg.rtp.type=ACK;
			msg.rtp.length=0;
		
			msg.rtp.seq_num=returnmsg.rtp.seq_num+1;
			msg.rtp.checksum=0;
			msg.rtp.checksum=compute_checksum(&msg.rtp, sizeof(msg.rtp));
			sendto(server_sockfd,&msg.rtp,sizeof(msg.rtp), 0, (struct sockaddr*)&client_address, sizeof(client_address));
		}
		connected=0;
		close(client_sockfd);
		//shutdown(client_sockfd,SHUT_RDWR);
		//shutdown(server_sockfd,SHUT_RDWR);
		//close(server_sockfd);
	}
	printf("close succeed\n");
	return 0;
}


