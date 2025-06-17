#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include <arpa/inet.h>

#define Range 100

long long Memory = 0;
int NodeNum = 1;

static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}


typedef struct bitmapTree{
	unsigned char bm;
	unsigned char pbm;
	struct bitmapTree* pt[8];
	struct bitmapTree* parent;	
} BT;


typedef struct IPv6{
	unsigned short ip[8];
	int len;
}IPv6;



BT* BuildNewNode(){
	BT*c = (BT*) malloc(sizeof(BT));
	
	Memory += sizeof(BT);
	NodeNum++;
	
	c->bm = 0;
	c->pbm = 0;
	c->parent = NULL;
	
	return c;
}

void BTInsert(BT* root, IPv6 p){
	BT*c = root;
	int pn;
	
	int index = 0;
	int len = 0;
	
	while(p.len > 2){
		pn = p.ip[index] & 7;
		
		//printf("pn:%d  == ", pn);
		
		if(c->pt[pn] == NULL){
			c->pt[pn] = BuildNewNode();
			c->pbm |= 1 << pn;
			c->pt[pn]->parent = c;
		}
		
		p.ip[index] >>= 2;
		p.len -= 2;
		len += 2;
		
		if(len % 8 == 0)  index++;
		
		c = c->pt[pn];
	}
	
	if(p.len == 0) c->bm |= 1;
	else if(p.len == 1){
		pn = p.ip[index] & 1;
		c->bm |= 1 << (1 + pn);
	}
	else if(p.len == 2){
		pn = p.ip[index] & 3;
		c->bm |= 1 << (3 + pn);
	}
}



BT* BTSearch(BT root, IPv6 ip){
	
	BT*c = &root;
	BT*Best = NULL;
	int pn;
	
	IPv6 rip;
	for(int i = 0; i < 8; i++)
		rip.ip[i] = 0;
	
	
	int index = 0;
	int len = 0;
	int n;
	while(c != NULL){
		pn = ip.ip[index] & 7;
		n = ip.ip[index] & 3;
		ip.ip[index] >>= 2;
		
		rip.ip[index] <<= 2;
		rip.ip[index] += n;
		
		len += 2;
		if(len % 8 == 0)
			index += 1;
		
		if(c->pt[pn])
			c = c->pt[pn];
		else
			break;
	}
	
	
	while(c != NULL){
		
		int CheckIP = rip.ip[index] & 3;
		
		rip.ip[index] >>= 2;
		len -= 2;
		if(len % 8 == 0)
			index -= 1;
		
		
		
		if(c->bm & (1 << (3 + (CheckIP & 3)))){
			Best = c;
			break;
		}
		else if(c->bm & (1 << (1 + (CheckIP & 1)))){
			Best = c;
			break;
		}
		else if(c->bm & 1){
			Best = c;
			break;
		}
		
		c = c->parent;
	}
	
	
	return Best;
}




unsigned short reverseBits(unsigned short num) {
    unsigned short reverse_num = 0;
    
    for (int i = 0; i < 16; i++) {
        reverse_num += num & 1;
        
        num >>= 1;
        if(i!=15) reverse_num <<= 1;
    }
    
    return reverse_num;
}



int main(int argc, char** argv){
	
	
	BT root = {0, 0};
	root.parent = NULL;
	
	long long Set, End;
	
	
	if(argc != 3){
		printf("input error!!!\n");
		exit(0);
	}
	
	FILE *fp;
	
	
	IPv6 p;
	struct in6_addr ipv6_addr;
	char string[100];
	char* token;
	
	
	int InIndex = 0;
	long long InClock = 0;
	
	fp = fopen(argv[1],"r");
	while(fgets(string,50,fp)!=NULL){
		
		InIndex++;
		
		//sscanf(string, "%d:%d:%d:%d:%d%d:%d:%d/%d", &p.ip[0], &p.ip[1], &p.ip[2], &p.ip[3], &p.ip[4], &p.ip[5], &p.ip[6], &p.ip[7], &p.len);
		
		token = strtok(string, "/");
		
		
		if (inet_pton(AF_INET6, string, &ipv6_addr) <= 0) {
	   		printf("IPv6 Read Error\n");
        		return 1;
    		}
		
		for (int i = 0; i < 8; i++) {
        		p.ip[i] = ntohs(*(unsigned short *)(ipv6_addr.s6_addr + i * 2));
    		}
		
		token = strtok(NULL, "/");
		
		p.len = atoi(token);
		
		for(int i = 0; i < 8; i++){
			p.ip[i] = reverseBits(p.ip[i]);
		}
		
		//printf("%d\n", p.len);
		//printf("%u:%u:%u:%u:%u:%u:%u:%u/%d\n", p.ip[0], p.ip[1], p.ip[2], p.ip[3], p.ip[4], p.ip[5], p.ip[6], p.ip[7], p.len);
		
		
		Set = rdtsc();
		
		BTInsert(&root, p);
		
		End = rdtsc();
		
		InClock += End - Set;
		
		//printf("BT: %d, %d\n", root.bm, root.pbm);
		
		printf("\rInput:%d, BuildAvgTime:%lf, Memory:%lf, NodeNum:%d ", InIndex, (double)InClock/InIndex, (double)Memory/1000000, NodeNum);
		
	}
	
	printf("\n");
	fclose(fp);
	
	
	
	/////////////////////////////////////----Search----/////////////////////////////////////
	
	int SearchIndex = 0;
	long long SearchClock = 0;
	
	int SearchRange[50];
	for(int i = 0; i < 50; i++) SearchRange[i] = 0;
	
	fp = fopen(argv[1],"r");
	while(fgets(string,50,fp)!=NULL){
		
		SearchIndex++;
		
		token = strtok(string, "/");
		
		
		if (inet_pton(AF_INET6, string, &ipv6_addr) <= 0) {
	   		printf("IPv6 Read Error\n");
        		return 1;
    		}
		
		for (int i = 0; i < 8; i++) {
        		p.ip[i] = ntohs(*(unsigned short *)(ipv6_addr.s6_addr + i * 2));
    		}
		
		token = strtok(NULL, "/");
		
		p.len = atoi(token);
		
		for(int i = 0; i < 8; i++){
			p.ip[i] = reverseBits(p.ip[i]);
		}
		
		
		//printf("Reverse: %u / %d\n\n", rip, len);
		Set = rdtsc();
		
		BTSearch(root, p);
		
		End = rdtsc();
		
		int Time = End - Set;
		SearchClock += Time;
		
		
		if(Time > Range*49) Time = Range*49;
		SearchRange[Time / Range] += 1;
		
		//printf("BT: %d, %d\n", root.bm, root.pbm);
		
		
		printf("\rSearch:%d, SearchAvgTime:%lf", SearchIndex, (double)SearchClock/SearchIndex);
		
	}
	
	printf("\n");
	fclose(fp);
	
	
	for(int i = 0 ; i < 50 ; i++)
		printf("%d\n", SearchRange[i]);
	
	
	
	
	
	/////////////////////////////////////----Insert----/////////////////////////////////////
		
	InIndex = 0;
	InClock = 0;
	
	fp = fopen(argv[2],"r");
	while(fgets(string,50,fp)!=NULL){
		
		InIndex++;
		
		//sscanf(string, "%d:%d:%d:%d:%d%d:%d:%d/%d", &p.ip[0], &p.ip[1], &p.ip[2], &p.ip[3], &p.ip[4], &p.ip[5], &p.ip[6], &p.ip[7], &p.len);
		
		token = strtok(string, "/");
		
		
		if (inet_pton(AF_INET6, string, &ipv6_addr) <= 0) {
	   		printf("IPv6 Read Error\n");
        		return 1;
    		}
		
		for (int i = 0; i < 8; i++) {
        		p.ip[i] = ntohs(*(unsigned short *)(ipv6_addr.s6_addr + i * 2));
    		}
		
		token = strtok(NULL, "/");
		
		p.len = atoi(token);
		
		for(int i = 0; i < 8; i++){
			p.ip[i] = reverseBits(p.ip[i]);
		}
		
		//printf("%d\n", p.len);
		//printf("%u:%u:%u:%u:%u:%u:%u:%u/%d\n", p.ip[0], p.ip[1], p.ip[2], p.ip[3], p.ip[4], p.ip[5], p.ip[6], p.ip[7], p.len);
		
		
		Set = rdtsc();
		
		BTInsert(&root, p);
		
		End = rdtsc();
		
		InClock += End - Set;
		
		//printf("BT: %d, %d\n", root.bm, root.pbm);
		
		printf("\rInput:%d, BuildAvgTime:%lf, Memory:%lf, NodeNum:%d ", InIndex, (double)InClock/InIndex, (double)Memory/1000000, NodeNum);
		
	}
	
	printf("\n");
	fclose(fp);
	
	
	return 0;
}
