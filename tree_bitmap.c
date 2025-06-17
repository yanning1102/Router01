#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

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

BT* BuildNewNode(){
	BT*c = (BT*) malloc(sizeof(BT));
	
	Memory += sizeof(BT);
	NodeNum++;
	
	c->bm = 0;
	c->pbm = 0;
	c->parent = NULL;

    for (int i = 0; i < 8; i++) {
        c->pt[i] = NULL;  // ✅ 初始化
    }
	
	return c;
}

void BTInsert(BT* root, int rip, int len){
	BT*c = root;
	int pn;
		
	while(len > 2){
		pn = rip & 7;
		
		if(c->pt[pn] == NULL){
			c->pt[pn] = BuildNewNode();
			c->pbm |= 1 << pn;
			c->pt[pn]->parent = c;
		}
		
		rip >>= 2;
		len-=2;
		c = c->pt[pn];
	}
	
	if(len == 0) c->bm |= 1;
	else if(len == 1){
		pn = rip & 1;
		c->bm |= 1 << (1 + pn);
	}
	else if(len == 2){
		pn = rip & 3;
		c->bm |= 1 << (3 + pn);
	}
}


BT* BTSearch(BT* root, int rip) {
    BT* c = root;
    BT* Best = NULL;
    int pn;
    unsigned int ip = 0;
    int n;

    while (c != NULL) {
        pn = rip & 7;
        n = rip & 3;
        rip >>= 2;

        ip <<= 2;
        ip += n;

        if (c->pt[pn])
            c = c->pt[pn];
        else
            break;
    }

    // ✅ 注意：必須先檢查 c != NULL
    while (c != NULL) {
        if (c->bm & (1 << (3 + (ip & 3)))) {
            Best = c;
            break;
        } else if (c->bm & (1 << (1 + (ip & 1)))) {
            Best = c;
            break;
        } else if (c->bm & 1) {
            Best = c;
            break;
        }

        ip >>= 2;
        c = c->parent;
    }

    return Best;
}




unsigned int reverseBits(unsigned int num) {
    unsigned int reverse_num = 0;
    
    for (int i = 0; i < 32; i++) {
        reverse_num += num & 1;
        
        num >>= 1;
        if(i!=31) reverse_num <<= 1;
    }
    
    return reverse_num;
}


int main(int argc, char**argv){
	
	BT* root = BuildNewNode();

	root->parent = NULL;
	
	long long Set, End;
	
	
	if(argc != 3){
		printf("input error!!!\n");
		exit(0);
	}
	
	FILE *fp;
	unsigned int ip;
	unsigned int cp[4];
	int len;
	char string[100];
	
	int InIndex = 0;
	long long InClock = 0;
	
	
	/////////////////////////////////////----Build----/////////////////////////////////////
	
	fp = fopen(argv[1],"r");
	while(fgets(string,50,fp)!=NULL){
		//if (InIndex >= 10000) break; 
		InIndex++;
		
		sscanf(string, "%d.%d.%d.%d/%d", &cp[0], &cp[1], &cp[2], &cp[3], &len);
		//ip = cp[0]<<24 + cpcp[1]<<16 + cp[2]<<8 + cp[3];
		
		ip = cp[0];
		ip <<= 8;
		ip += cp[1];
		ip <<= 8;
		ip += cp[2];
		ip <<= 8;
		ip += cp[3];
		
		//printf("%u.%u.%u.%u/%d  |  %u/%d\n", cp[0], cp[1], cp[2], cp[3], len, ip, len);
		
		unsigned int rip = reverseBits(ip);
		
		//printf("Reverse: %u / %d\n\n", rip, len);
		Set = rdtsc();
		
		BTInsert(root, rip, len);
		
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
		//if (SearchIndex >= 10000) break;
		SearchIndex++;
		
		sscanf(string, "%d.%d.%d.%d/%d", &cp[0], &cp[1], &cp[2], &cp[3], &len);
		//ip = cp[0]<<24 + cp[1]<<16 + cp[2]<<8 + cp[3];
		
		ip = cp[0];
		ip <<= 8;
		ip += cp[1];
		ip <<= 8;
		ip += cp[2];
		ip <<= 8;
		ip += cp[3];
		
		//printf("%u.%u.%u.%u/%d  |  %u/%d\n", cp[0], cp[1], cp[2], cp[3], len, ip, len);
		
		unsigned int rip = reverseBits(ip);
		
		//printf("Reverse: %u / %d\n\n", rip, len);
		Set = rdtsc();
	
		BT* result = BTSearch(root, rip);

		
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
		//if (InIndex >= 10000) break;
		InIndex++;
		
		sscanf(string, "%d.%d.%d.%d/%d", &cp[0], &cp[1], &cp[2], &cp[3], &len);
		//ip = cp[0]<<24 + cp[1]<<16 + cp[2]<<8 + cp[3];
		
		ip = cp[0];
		ip <<= 8;
		ip += cp[1];
		ip <<= 8;
		ip += cp[2];
		ip <<= 8;
		ip += cp[3];
		
		//printf("%u.%u.%u.%u/%d  |  %u/%d\n", cp[0], cp[1], cp[2], cp[3], len, ip, len);
		
		unsigned int rip = reverseBits(ip);
		
		//printf("Reverse: %u / %d\n\n", rip, len);
		Set = rdtsc();
		
		BTInsert(root, rip, len);
		
		End = rdtsc();
		
		InClock += End - Set;
		
		//printf("BT: %d, %d\n", root.bm, root.pbm);
		
		
		printf("\rInput:%d, BuildAvgTime:%lf, Memory:%lf, NodeNum:%d ", InIndex, (double)InClock/InIndex, (double)Memory/1000000, NodeNum);
		
	}
	
	printf("\n");
	fclose(fp);
	
	
	
	
	
	printf("\n");
	return 0;
}
