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


int max_disk_queue = 0;
mutex mutex1;
cv cv_requester;
cv cv_servicer;
char ** argv = nullptr;
int argc = 0;
std::unordered_map<int, cv> condition_set;

class OrderedList
{
    private:
    // PROJECT UPDATES #1
    std::unordered_map<int, int> set;

    public:
    void push(int requester, int track)
    {
        set[requester] = track;
        // push a personal cv to the queue along with it
    }


    std::pair<int, int> next(int track)
    {
        if (set.empty())
        {
            cout << "Next called on empty Queue\n";
            return std::make_pair(-1, -1);
        }

        std::pair<int,int> closest_pair; 
        int minDist = std::numeric_limits<int>::max();
        for (auto& pair : set)
        {
            if (minDist > abs(pair.second - track))
            {
                minDist = abs(pair.second - track);
                closest_pair = pair;
            }
        }
        
        // MIDTERM CHANGE
        // add the erase to condition set so that it can be in parallel with
        // the current set
        set.erase(closest_pair.first);
        return closest_pair;
    }

    void print()
    {
        if (set.empty())
        {
            cout << "Empty Queue";
        }
        for (auto& pair :set)
        {
            cout << pair.first << "," << pair.second << " ";
        }
        cout << endl;
    }

    int size()
    {
        return set.size();
    }

    bool contains(int requester)
    {
        return set.find(requester) != end(set);
    }

};

int unfinished_tasks = 0;
OrderedList q;
int finished_tasks = 0;


void requester(void * a)
{

    char * filename = (char*) a;
    int len = strlen(filename);
    int requester = filename[len - 1] - '0';
    condition_set[requester] = cv();
    ifstream file(filename);
    string track;
    
    mutex1.lock();
    while (file >> track)
    {
        int track_i = std::stoi(track);
        // this will die if it contains again because it goes to sleep without signaling someone else
        while (q.size() >= max_disk_queue or q.contains(requester))
        {
            // MIDTERM CHANGE
            // instead of waiting on a global CV, wait on a single 
            // private CV so that this single thread can be woken up
            condition_set[requester].wait(mutex1);
        }

        q.push(requester, track_i);
        //q.print();
        print_request(requester, track_i);

        cv_servicer.signal();
    }

    while (q.contains(requester))
    {
        // MIDTERM CHANGE
        // instead of waiting on a global CV, wait on a single 
        // private CV so that this single thread can be woken up
        condition_set[requester].wait(mutex1);
    }
    unfinished_tasks--; // signal the servicer, it might be waiting
    if (unfinished_tasks < max_disk_queue)
    {
        max_disk_queue = unfinished_tasks;
    }
    cv_servicer.signal();
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
    if (unfinished_tasks < max_disk_queue)
    {
        max_disk_queue = unfinished_tasks;
    }
    int currentTrack = 0;
    mutex1.lock();
    while (unfinished_tasks != 0) 
    {
        // if the queue is not full go to sleep, wait for signal
        while (q.size() < max_disk_queue) 
        {
            cv_servicer.wait(mutex1);
        }
        if (unfinished_tasks == 0) {break;}

        // queue is now full, service a disk I/O
        auto nextDiskIO = q.next(currentTrack);
        currentTrack = nextDiskIO.second;
        print_service(nextDiskIO.first, nextDiskIO.second);

        // MIDTERM CHANGE
        // instead of broacasting to all of the threads, 
        // pinpoint the single thread that needs to wake up
        // and wake it up with a lookup
        condition_set[nextDiskIO.first].signal(); 
    }
    mutex1.unlock();

    for (auto ptr : threads)
    {
        ptr->join();
    }
    //q.print();
}

int main(int argc_, char ** argv_)
{
    argc = argc_;
    argv = argv_;  
    max_disk_queue = std::stoi(argv[1]);
    unfinished_tasks = argc - 2;
    cpu::boot((thread_startfunc_t) servicer, (void *) argv[1],0);
}



