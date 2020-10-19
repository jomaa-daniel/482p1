#include <iostream>
#include "thread.h"
#include <queue>
#include <vector>
#include <fstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <list>
#include "disk.h"
#include <unordered_map>
#include <limits>


using std::string;
using std::ifstream;
using std::vector;
using std::queue;
using std::cout;
using std::endl;
using std::list;
using std::begin;

semaphore sm(0);

void threader(void* a)
{
    printf("%i starts\n", a);
    sm.up();
    sm.down();
    sm.up();
    
}

void thread1
{
    print("1 start")
    up();
    down();
    up();
    print("1 ends\n");
}
void thread2
{
    down();
    print("2 starts");
}



void servicer(void *a)
{
    
}

int main(int argc_, char ** argv_)
{
    cpu::boot((thread_startfunc_t) servicer, (void *) 0,0);
}