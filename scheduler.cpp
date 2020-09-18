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
#include <unordered_set>

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
cv cv_requester;
cv cv_servicer;
char ** argv = nullptr;
int argc = 0;

class OrderedList
{
    private:
    list<std::pair<int,int>> lst;
    std::unordered_set<int> set;
    public:
    void push(int requester, int track)
    {
        set.insert(requester);
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
        set.erase(closest->first);
        return ret;
        // mutex1.unlock();
    }

    void print()
    {
        if (lst.empty()){cout << "Queue Empty\n"; return;}
        for (auto & pr : lst)
        {
            cout << pr.first << "," << pr.second << " ";
        }
        cout << endl;
    }

    int size()
    {
        // mutex1.lock();
        return lst.size();
        // mutex1.unlock();
    }

    bool contains(int requester)
    {
        return set.find(requester) != end(set);
    }

};

int unfinished_tasks = 0;
OrderedList q;


void requester(void * a)
{
    //cout << "hello";
    char * filename = (char*) a;
    int len = strlen(filename);
    int requester = filename[len - 1] - '0';
    ifstream file(filename);
    string track;
    while (file >> track)
    {
        int track_i = std::stoi(track);
        mutex1.lock();
        while (q.size() >= max_disk_queue or q.contains(requester))
        {
            cout << "Requester " << requester << " waiting " << endl;
            cv_requester.wait(mutex1);
            cout << "Requester woke up\n";
        }

        q.push(requester, track_i);
        q.print();
        print_request(requester, track_i);
        mutex1.unlock(); 
        cout << "servicer Signaled\n";
        cv_servicer.signal();
    }
    // TODO this could be wrong
    mutex1.lock();
    unfinished_tasks--; // signal the servicer, it might be waiting
    mutex1.unlock();
}


void servicer(void *a)
{
    vector<thread*> threads;
    // deploy consumer threads

    for (int i = 2; i < argc; ++i){
        thread *t1 = new thread((thread_startfunc_t) requester, (void *) argv[i]);
        threads.push_back(t1);
    }
    int currentTrack = 0;
    mutex1.lock();
    while (unfinished_tasks != 0)
    {
        // if the queue is not full go to sleep, wait for signal
        while (q.size() < max_disk_queue)
        {
            cout << "Servicer waiting" << endl; 
            cv_servicer.wait(mutex1);
            cout << "servicer woke up\n";
        }
        // TODO this will error if the servicer wakes up
        // queue is now full, service a disk I/O
        auto nextDiskIO = q.next(currentTrack);
        currentTrack = nextDiskIO.second;
        q.print();
        print_service(nextDiskIO.first, nextDiskIO.second);
        cout << "requester signaled\n";
        cv_requester.broadcast(); // should be broadcast (but it relies on the contains condition)
    }
    mutex1.unlock();

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
    cpu::boot((thread_startfunc_t) servicer, (void *) argv[1],1);
}



