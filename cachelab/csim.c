#include "cachelab.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>

int word_size = 64;

void printHelp();
int readingInitParams(int argc, char* argv[], int hvsEb[5], char** tracefile);
int isNums(char* str);

void analyzeTypeAddress(char* buffer, int buffer_len, char* ans);
int binaryChar2int(char* hex, int len);
void simulator(FILE* file, char* cache, int tagBitsNum, int hvsEb[5], int hme[3]);

void printTrace(int hitLine, int evictLine, int invalidLine, char* pos, int isHit, int hvsEb[5], int setNumber, int* timeSteps);
int main(int argc, char* argv[]) {
    int hvsEb[5] = {0, 0, 0, 0, 0};
    char* tracefile = NULL;
    if(!readingInitParams(argc, argv, hvsEb, &tracefile)) {
        return 0;
    }

    FILE* file = fopen(tracefile, "r");
    if(file == NULL) {
        printf("%s: No such file or directory\n", tracefile);
        return 0;
    }

    /* initilize chche abstraction by params. */
    int tagBitsNum = word_size - hvsEb[2] - hvsEb[4];
    int cacheCapacity = (1<<hvsEb[2]) * hvsEb[3] * (tagBitsNum+1);
    char* cache = (char*) malloc(cacheCapacity);
    // char cache[1<<hvsEb[2]][hvsEb[3]][tagBitsNum+1];
    memset(cache, 0, cacheCapacity );

    int hme[3] = {0, 0, 0};
    simulator(file, cache, tagBitsNum, hvsEb, hme);
    
    printSummary(hme[0], hme[1], hme[2]);
    fclose(file);
    return 0;
}


void printHelp() {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h          Print this help message.\n"
           "  -v          Optional verbose flag.\n"
           "  -s <num>    Number of set index bits.\n"
           "  -E <num>    Number of lines per set.\n"
           "  -b <num>    Number of block offset bits.\n"
           "  -t <file>   Trace file.\n\n"
           "Examples:\n"
           "  linux> ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux> ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.traces\n"
           );
    return;
}


int isNums(char* str){
    int len = strlen(str);
    int ans = 1;
    for (int i = 0; i < len; ++i) {
        ans = ans && (str[i] >= '0' && str[i] <= '9');
    }
    return ans;
}


int readingInitParams(int argc, char* argv[], int hvsEb[5], char** tracefile) {
    if(argc > 1){
        int len = strlen(argv[1]);
        for(int i = 1; i < len; ++i) {
            hvsEb[0] = (argv[1][i] == 'h');
            hvsEb[1] = (argv[1][i] == 'v');
        }
    }
    int index = (hvsEb[0] || hvsEb[1]) ? 2 : 1;
    int num = 0;
    while(index < argc) {
        switch(argv[index][1]) {
            case 's':
                num = 2;
                break;
            case 'E':
                num = 3;
                break;
            case 'b':
                num = 4;
                break;
            case 't':
                *tracefile = argv[index+1];
                index += 1;
            default:
                num = 0;
        }
        if (num && index+1 < argc && isNums(argv[index + 1])) {
            sscanf(argv[index+1], "%d", &hvsEb[num]);
            index += 2;
        }
        else {
            ++index;
        }
    }
    
    int ans = 1;
    if(hvsEb[0]) {
        printHelp();
        ans = 0;
    } else if (!(hvsEb[2] && hvsEb[3] && hvsEb[4])) {
        printf("Missing required command line argument\n");
        printHelp();
        ans = 0;
    }
    return ans;
}


int binaryChar2int(char* bin, int len) {
    int ans = 0;
    int base = 0;
    len -= 1;
    while(len >= 0) {
        ans += ((bin[len] - '0') << base);
        ++base; --len;
    }
    return ans;
}


void analyzeTypeAddress(char* buffer, int buffer_len, char* ans) { 
    int i = buffer_len - 1;
    while(buffer[i--] != ',');
    int j = 61;
    while(buffer[i] != ' '){
        switch(buffer[i--]) {
            case 'f' : memcpy(ans+j, "1111", 4); break;
            case 'e' : memcpy(ans+j, "1110", 4); break;
            case 'd' : memcpy(ans+j, "1101", 4); break;
            case 'c' : memcpy(ans+j, "1100", 4); break;
            case 'b' : memcpy(ans+j, "1011", 4); break;
            case 'a' : memcpy(ans+j, "1010", 4); break;
            case '9' : memcpy(ans+j, "1001", 4); break;
            case '8' : memcpy(ans+j, "1000", 4); break;
            case '7' : memcpy(ans+j, "0111", 4); break;
            case '6' : memcpy(ans+j, "0110", 4); break;
            case '5' : memcpy(ans+j, "0101", 4); break;
            case '4' : memcpy(ans+j, "0100", 4); break;
            case '3' : memcpy(ans+j, "0011", 4); break;
            case '2' : memcpy(ans+j, "0010", 4); break;
            case '1' : memcpy(ans+j, "0001", 4); break;
            case '0' : memcpy(ans+j, "0000", 4); break;
        }
        j -= 4; 
    }
    ans[0] = buffer[i-1];
    return;
}


void simulator(FILE* file, char* cache, int tagBitsNum, int hvsEb[5], int hme[3]) {
    char buffer[30];
    char ans[66]; ans[65] = '\0';

    char* info[3] = {" miss", " hit", " eviction"};

    /* initialize timeSteps numbers of every cache line. */
    int timeSteps[1 << hvsEb[2]][hvsEb[3]];
    memset(timeSteps, 0, (1<<hvsEb[2])*hvsEb[3]*sizeof(int) );

    int setCapacity = (hvsEb[3] * (tagBitsNum+1)), lineStep = tagBitsNum+1;
    
    while( fgets(buffer, 30, file) ) {
        int buffer_len = strlen(buffer);
        memset(ans, '0', 65);
        if(buffer[0] == 'I') {
            continue;
        }

        analyzeTypeAddress(buffer, buffer_len, ans);
        int s = binaryChar2int(ans+1+tagBitsNum, hvsEb[2]);
        int offset = s*setCapacity;

        int hitLine=0, isHit=lineStep; int invalidLine = -1; int evictLine = 0;
        int maxTimeStep = timeSteps[s][0];

        for(int line = 0; line < hvsEb[3]; ++line, offset += isHit) {
            if(isHit && strncmp(ans+1, cache+offset, tagBitsNum) == 0)
                isHit = 0, hitLine = line;
            invalidLine = cache[offset+tagBitsNum] ? invalidLine : line;
            if(maxTimeStep < timeSteps[s][line]) {
                maxTimeStep = timeSteps[s][line];
                evictLine = line;
            }
            ++timeSteps[s][line];
        }
        
        char* h = ""; char* m = ""; char* e = "";
        char* targetPos = NULL;
        if(isHit == 0 && cache[offset+tagBitsNum]) { 
            hme[0] += 1, h = info[1]; // hit
            timeSteps[s][hitLine] = 1;
        } else if (invalidLine == -1) { 
            // cache replacement policy.
            m = info[0], ++hme[1]; e = info[2], ++hme[2];
            targetPos = cache + s*setCapacity + evictLine*lineStep;
            timeSteps[s][evictLine] = 1;
        } else {
            // cold miss.
            m = info[0], ++hme[1];
            targetPos = cache + s*setCapacity + invalidLine*lineStep;
            timeSteps[s][invalidLine] = 1;
        }
        
        /* If cache miss, load the block in cache. */
        if(targetPos) {
            memcpy(targetPos, ans + 1, tagBitsNum);
            targetPos[tagBitsNum] = 1;
        }

        hme[0] += (ans[0] == 'M');
        buffer[buffer_len-1] = '\0';
        if(hvsEb[1]) {
            char* modifyHit = ans[0] == 'M' ? info[1] : "";
            printf("%s%s%s%s%s\n", buffer, m, e, h, modifyHit);
        }
        // printTrace(hitLine, evictLine, invalidLine, cache+offset, isHit, hvsEb, s, (int*)timeSteps);
    }
    return;
}


void printTrace(int hitLine, int evictLine, int invalidLine, char* pos, int isHit, int hvsEb[5], int setNumber, int* timeSteps) {
    int tagBitsNum = word_size - hvsEb[2] - hvsEb[4];
    static int line = 1;
    printf("%4d\t", line);
    if(isHit == 0 && pos[tagBitsNum]) { 
        printf("Hit _: set=%d, line=%d\t", setNumber, hitLine);
    } else if (invalidLine == -1) { 
        printf("Mis r: set=%d, elne=%d\t", setNumber, evictLine);
    } else {
        printf("Mis c: set=%d, rlne=%d\t", setNumber, invalidLine);
    }
    printf("time setp info: ");
    for(int i = 0; i < hvsEb[3]; ++i) {
        printf("%4d ", timeSteps[setNumber*hvsEb[3] + i]);
    }
    printf("\n");
    ++line;
}
