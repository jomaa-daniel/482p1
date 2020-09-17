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

using std::string;
using std::ifstream;
using std::vector;
using std::queue;
using std::cout;
using std::endl;
using std::list;
using std::begin;


int max_disk_queue = 0;
mutex mutex1;
cv cv1;
char ** argv = nullptr;
int argc = 0;

class OrderedList
{
    private:
    list<std::pair<int,int>> lst;

    public:
    void push(int requester, int track)
    {
        // mutex1.lock();
       // cout << "pushing " << requester << "," << track << endl;
        if (lst.empty())
        {
            lst.emplace_front(requester, track);
            return;
        }
        auto cursor = begin(lst);

        while (cursor != end(lst) and cursor->second < track)
        {
            ++cursor;
        }
        lst.emplace(cursor, requester, track);
        // mutex1.unlock();
    }

    std::pair<int, int> next(int track)
    {
        // mutex1.lock();
        auto cursor = begin(lst);

        while (cursor->second != track)
        {
            ++cursor;
        }
        auto closest = (abs(std::prev(cursor)->second - track) 
        < abs(std::next(cursor)->second - track)) ? std::next(cursor) : std::prev(cursor);

        auto ret = * closest;
        lst.erase(closest);
        return ret;
        // mutex1.unlock();
    }

    void print()
    {
        for (auto & pr : lst)
        {
            cout << pr.first << "," << pr.second << " ";
        }
    }

    int size()
    {
        // mutex1.lock();
        return lst.size();
        // mutex1.unlock();
    }

};

int unfinished_tasks = 0;
OrderedList q;



void consumer(void * a)
{
    char * filename = (char*) a;
    int len = strlen(filename);
    int requester = filename[len - 1] - '0';
    //cout << requester << " " << filename << endl;
    ifstream file(filename);
    string track;
    while (file >> track)
    {
        int track_i = std::stoi(track);
        mutex1.lock();
        while (q.size() >= max_disk_queue)
        {
            cv1.wait(mutex1);
        }
        // mutex1.lock();
        q.push(requester, track_i);
        print_request(requester, track_i);
        cv1.signal();
        // mutex1.unlock();
    }
    // mutex1.lock();
    unfinished_tasks--;
    // mutex1.unlock();
}


void servicer(void *a)
{
    vector<thread*> threads;
    // deploy consumer threads

    for (int i = 2; i < argc; ++i){
        thread *t1 = new thread((thread_startfunc_t) consumer, (void *) argv[i]);
        threads.push_back(t1);
    }
    int currentTrack = 0;
    // mutex1.lock();
    while (unfinished_tasks != 0)
    {
        // mutex1.unlock();
        // if the queue is not full go to sleep, wait for signal
        mutex1.lock();
        while (q.size() < max_disk_queue)
        {
            cv1.wait(mutex1);
        }
        // queue is now full, service a disk I/O
        // mutex1.lock();
        auto nextDiskIO = q.next(currentTrack);
        currentTrack = nextDiskIO.second;
        print_service(nextDiskIO.first, nextDiskIO.second);
        cv1.signal();
    }

    for (auto ptr : threads)
    {
        ptr->join();
    }
    q.print();
}

int main(int argc_, char ** argv_)
{
    argc = argc_;
    argv = argv_;  
    max_disk_queue = std::stoi(argv[1]);
    unfinished_tasks = argc - 2;
    cpu::boot((thread_startfunc_t) servicer, (void *) argv[1], 0);
    cout << endl;
}



