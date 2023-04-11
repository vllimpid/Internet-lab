#ifndef COMPNET_LAB4_SRC_SWITCH_H
#define COMPNET_LAB4_SRC_SWITCH_H

#include "types.h"
#include"stdio.h"
struct trans{
  int port;
  char mac[6];
  int time;
};
struct info{
  ether_header_t myheader;
  char* payload;

};
class SwitchBase {
 public:
  SwitchBase() = default;
  ~SwitchBase() = default;

  virtual void InitSwitch(int numPorts) = 0;//进行交换机状态的初始化,numPorts表示此交换机的端口的数量，保证numPorts大于0
  virtual int ProcessFrame(int inPort, char* framePtr) = 0;//交换机接收到的帧进行处理与转发,inPort表示收到的帧的入端口号,framePrt是一个指针，表示收到的帧;返回值表示的是这个帧应当被转发到的端口号
};

class EthernetSwitch : public SwitchBase {
private:
  int numports=0;
  struct trans tran[1000];
  int total=0;
public:
  EthernetSwitch(){
  	total=0;
  }
  void InitSwitch(int numPorts) override {
   /* your implementation ... */ 
     numports=numPorts;
     total=0;
     for(int i=0;i<1000;i++){
       tran[i].port=0;
       memset(tran[i].mac,0,sizeof(tran[i].mac));
       tran[i].time=0;
     }
  }
    int ProcessFrame(int inPort, char* framePtr) override {
     printf("calculatoring%d\n",inPort);
     /* your implementation ... */ 
     char dest[6]={};char src[6]={};char type[2]={};
     int returnport=0;
     memcpy(dest,framePtr,6);
     memcpy(src,framePtr+6,6);
     memcpy(type,framePtr+12,2);
     
     //printf("get finish %s %s\n",dest,src);
     //printf("type:%d\n",*(uint16_t*)type);
     if(*(uint16_t*)type!=0){
     	printf("aging\n");
     	for(int i=0;i<total;i++){
     		tran[i].time--;
     	}
     	return -1;
     }
     int i=0;
     int used=0;
     for(int k=0;k<total;k++){
     	if(tran[k].port==inPort&&!memcmp(src,tran[k].mac,6)){
     		tran[k].time=10;used=1;
     		break;
     	}
     	if(tran[k].time<=0){
     		tran[k].port=inPort;
     		memcpy(tran[k].mac,src,6);
     		tran[k].time=10;used=1;
     		break;
     	}
     }
     if(used==0){
     	tran[total].port=inPort;
     	memcpy(tran[total].mac,src,6);
     	tran[total].time=10;;
     	total++;
     }
     //tran[inPort]=src;
     //memcpy(tran[inPort],src,6);
     for(i=0;i<total;i++){
       if(!memcmp(dest,tran[i].mac,6)&&tran[i].time>0){
         returnport=tran[i].port;
       }
       
     }
     if(returnport==0){
     	tran[total].port=inPort;
     	memcpy(tran[total].mac,src,6);
     	tran[total].time=10;;
     	total++;
     }
     if(returnport==inPort){
     	return -1;
     }
     
     return returnport;
   }
     
};
extern SwitchBase* CreateSwitchObject();

// TODO : Implement your switch class.

#endif  // ! COMPNET_LAB4_SRC_SWITCH_H
