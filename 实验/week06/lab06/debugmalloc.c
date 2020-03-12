/*
* 此实验的目的在于检测内存区域，它给出了内存结构：header+size+footer，详情参照文档内的图片
* 实验目的一：实现该种内存的分配。原本malloc分配内存只会有size大小，现在要加上header和footer，简单起见header里就两个东西checksum和fence，footer里就一个东西fence。
* 实验目的二：记录内存分配情况。分配和释放都要做记录，记录存放在一个结构里，该结构可以是链表，也可以是数组，分配则添加记录，释放则删除记录，该记录存储内存的所有信息，具体见代码。
* 实验目的三：内存问题检测与追踪。检测的问题有五个，在myfree中实现的有四个。具体问题如下：
* The errors that can occur are: 
* •	Error #1: Writing past the beginning of the user's block (through the fence) 
* •	Error #2: Writing past the end of the user's block (through the fence) 
* •	Error #3: Corrupting the header information 
* •	Error #4: Attempting to free an unallocated or already-freed block 
* •	Error #5: Memory leak detection (user can use ALLOCATEDSIZE to check for leaks at the end of the program) 
* 错误检测的实现通过检查cheksum和fence，错误追踪实现通过其给出的函数实现，使用见代码。
* 实验操作：将所有相关的.c和.h导入到项目中去（有vc，可直接打开，因为它给的东西里有vc的项目文件）
* 实验运行：命令行参数中输入-t 1/2.../8(/表示或，意思-t 1、-t 2等等，一次输入一个)，输出结果见文档最后，correct output中，-t 1对应test case #1
*/
#include <stdlib.h>
#include <string.h>
#include "debugmalloc.h"
#include "dmhelper.h"
#include <stdio.h>

//下面这个函数是老师给的，可直接复制粘贴，作用是统计二进制形式中的1的个数，用于确定checksum中的
int bitCount(int x) {

    /* Sum 8 groups of 4 bits each */
    int m1 = 0x11 | (0x11 << 8);
    int mask = m1 | (m1 << 16);
    int s = x & mask;
    s += x>>1 & mask;
    s += x>>2 & mask;
    s += x>>3 & mask;
    /* Now combine high and low order sums */
    s = s + (s >> 16);
    //printf("s=%x\n",s);

    /* Low order 16 bits now consists of 4 sums,
    each ranging between 0 and 8.
    Split into two groups and sum */
    mask = 0xF | (0xF << 8);
    s = (s & mask) + ((s >> 4) & mask);
    return (s + (s>>8)) & 0x3F;
}

//以下是记录，用来记录内存的信息，自己定义的，不可复制粘贴，以下是定义为链表，可用数组来做
typedef struct record{
    //checkSum用来存储检验和，其值为fence中的1的个数
    int checkSum;
    //fileName是题目自带的filename变量，不用管含义，内存信息里包括它，所以结构体里要存它
    char * fileName;
    //错误的行数，对应自带的linenumber变量，内存信息包括它，指出错误所在位置
    int lineNumber;
    //malloc分配出的大小，对应自带的size变量，size_t是自带的已经定义好的，不用管它
    size_t size;
    //此处的fence就是header和footer的fence
    int fence;
    //原本malloc分配的内存的首地址，加了header和footer是通过自己手动开辟的所以要记录这块正确的数据空间；
    void *payload;
    //指向下一个节点，数组实现的方法不需要这个
    struct record * next;
} record;
//头结点
record * head;
//尾结点
record * tail;
//当前节点，指向当前节点的前一个位置，千万记住这一点！！！！指向当前节点的前一位置
record * curr;
//链表的长度
int length = 0;

//初始化链表，一个空的头结点
void initialHead() {
    if (length == 0) {
        head = (record *) malloc(sizeof(record));
        head->checkSum = 0;
        head->fileName = NULL;
        head->lineNumber = 0;
        head->size = 0;
        head->fence = 0;
        head->payload = NULL;
        head->next = NULL;
        tail = head;
    }
}

//在链表尾部添加一个数据，在mymalloc中调用
void append(int checkSum, char * fileName, int lineNumber, size_t size, void *payload) {
    record * newNode = (record *) malloc(sizeof(record));
    initialHead();
    newNode->checkSum = checkSum;
    newNode->fileName = fileName;
    newNode->lineNumber = lineNumber;
    newNode->size = size;
    newNode->fence = 0xCCDEADCC;
    newNode->payload = payload;
    newNode->next = NULL;
    tail->next = newNode;
    tail = newNode;
    length++;
}

//找到malloc分配的内存信息，payload存储的是malloc分配（未加header和footer）的地址
int find(void *payload) {
    initialHead();
    curr = head;
    while (curr->next != NULL) {
        if (curr->next->payload == payload) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

//删除一个节点，在myfree中调用
void removeNode(void *payload) {
    find(payload);
    record * tmp = curr->next;
    curr->next = curr->next->next;
    free(tmp);
    length--;
}

//判断头部信息是否错误，比较checksum是否一样，记录中的checksum和内存分配时的header内部的checksum
int headerCorrect(record * r) {
    //payload指向未加header时的首地址，那么其地址-2就是checksum的值
    int * tmp = (int *) r->payload;
    if (r->checkSum != *(tmp - 2)) {
        return 0;
    }
    return 1;
}

//判断header中的fence是否受损
int headerFenceCorrect(record * r) {
    int * tmp = (int *) r->payload;
    if (r->fence != *(tmp - 1)) {
        return 0;
    }
    return 1;
}


//判断footer中的fence是否受损，每次加一个单位，int类型就是加4所以除4
int footerFenceCorrect(record * r) {
    int * tmp = (int *) r->payload;
        if (r->fence != *(tmp + r->size / 4)) {
            return 0;
        }
        return 1;
}


/* Wrappers for malloc and free */

void *MyMalloc(size_t size, char *filename, int linenumber) {
    int * newptr = NULL;
    //fence中的值固定，可以和我的一样
    int checkSum = bitCount(0xCCDEADCC);
    //initial字符可以不用，之后size/4
    char * initial = NULL;    
    //未修改前内存分配调用malloc直接就是size的大小，但现在在头和尾加了东西，简单起见，头部有checksum和fence信息，尾部有fence信息，所以多开辟三个int大小的空间
    newptr = (int *) malloc(size + 3 * sizeof(int));
    //可以不要这个if判断，是为了检测内存分配是否成功
    if (newptr == NULL) {
        printf("failed");
    }
    //现在的newptr指向的就是checksum，并不是指向未加头和尾之前的位置
    *newptr = checkSum;
    //加了个1使其指向checksum的下一个位置，即fence
    newptr = newptr + 1;
    *newptr = 0xCCDEADCC;
    //再次加一，使得newptr指向了原本该指向的位置，即直接未加头部时的位置，这也正是用户存储数据的位置
    newptr = newptr + 1;
    /*
    * 先转换为一个字节的char类型指针，在加size就是移动了size个字节，然后就指向了尾部
    * 如果不这么做，即不转型，可以直接写成newptr = newptr + size / 4，之后再改成减号，这样就不需要initial变量了
    * 加了又减是因为使其回到malloc原本分配的位置
    */
    initial = (char *) newptr;
    initial = initial + size;
    newptr = (int *) initial;
    *newptr = 0xCCDEADCC;
    initial = initial - size;
    append(checkSum, filename, linenumber, size, initial);
    return initial;
}

//以下函数是释放内存空间，释放时，进行错误检测
void MyFree(void *ptr, char *filename, int linenumber) {
    //查找ptr是否存在，这个ptr是malloc原本分配空间的首地址，find是返回0/1用于判断是否已经分配了该空间
    int b = find(ptr);
    if (!b) {
        //error给出的函数，第一个参数是错误类型代码，第二个参数filename，第三个linenumber，调用它会打印出错误信息
        error(4, filename, linenumber);
    } else if (!headerCorrect(curr->next)) {
        //errorf1给出的函数，多了两个参数，是结构中存储的该内存信息的filename和linenumber，，调用它会打印出错误信息
        errorfl(3, curr->next->fileName, curr->next->lineNumber, filename, linenumber);
    } else if (!headerFenceCorrect(curr->next)) {
        errorfl(1, curr->next->fileName, curr->next->lineNumber, filename, linenumber);
    } else if (!footerFenceCorrect(curr->next)) {
        errorfl(2, curr->next->fileName, curr->next->lineNumber, filename, linenumber);
    }
    //移除结构中的该内存信息的节点
    removeNode(ptr);
    //之所以减2，是将指针移到头部
	free((void *) ((int *) ptr - 2));
}

/* returns number of bytes allocated using MyMalloc/MyFree:
	used as a debugging tool to test for memory leaks */
int AllocatedSize() {
    //长度为0，说明分配空间为0
    if (length == 0) {
        return 0;
    }
    curr = head;
    int size = 0;
    //计算整个已分配的空间大小
    while (curr->next != NULL) {
        size += curr->next->size;
        curr = curr->next;
    }
	return size;
}



/* Optional functions */

/* Prints a list of all allocated blocks with the
	filename/line number when they were MALLOC'd */
void PrintAllocatedBlocks() {
    if (length == 0) {
        return;
    }
    curr = head;
    printf("Currently allocated block: \n");
    while (curr->next != NULL) {
        //下面这个函数是其自带的，用于打印出块的信息，接受size、filename和linenumber
        PRINTBLOCK(curr->next->size, curr->next->fileName, curr->next->lineNumber);
        curr = curr->next;
    }
	return;
}

/* Goes through the currently allocated blocks and checks
	to see if they are all valid.
	Returns -1 if it receives an error, 0 if all blocks are
	okay.
*/
int HeapCheck() {
    if (length == 0) {
        return 0;
    }
    curr = head;
    while (curr->next != NULL) {
        if (!headerCorrect(curr->next)) {
            //自带的函数，用于答应堆的错误信息，第一个参数是错误类型
            PRINTERROR(3, curr->next->fileName, curr->next->lineNumber);
            return -1;
        } else if (!headerFenceCorrect(curr->next)) {
            PRINTERROR(1, curr->next->fileName, curr->next->lineNumber);
            return -1;
        } else if (!footerFenceCorrect(curr->next)) {
            PRINTERROR(2, curr->next->fileName, curr->next->lineNumber);
            return -1;
        }
        curr = curr->next;
    }
	return 0;
}
