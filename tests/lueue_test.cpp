#ifdef XHBLIB_LIST_QUEUE_UNIT_TEST

#include "src/lueue.h"


#include <cassert>

#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

void test_member(){
    {
        lueue<int> l1;
        l1.push_back(1);
        assert(l1.front() == 1 && l1.back() == 1);
        
        for(int i = 0; i != 200; i++)
            l1.push_back(i);
        
        assert(l1.size() == 201 && l1.back() == 199);
        
        int i = 100;
        while(i--){
            l1.pop_front();
        }
        
        assert(l1.size() == 101 && l1.back() == 199 && l1.front() == 99);
        
        l1.clear();
        assert(l1.size() == 0);
        
    }
    {
        lueue<string> l2;
        l2.reserve(1023);
        
        assert(l2.capacity() == 1024);
        
        int n = 2049;
        while(n--){
            stringstream ss;
            ss<<n;
            l2.emplace_back(ss.str());
        }
        assert(l2.back() == "0" && l2.size() == 2049);
        
        n = 2048;
        while(n--){
            l2.pop_front();
        }
        assert(l2.size() == 1 && l2.front() == "0");
    }
   }

void test_memory_cycle()
{
    lueue<string> l;
    
    int cycle = 0;
    while(1){
        cycle++;
        int n = 2049;
        while(n--){
            stringstream ss;
            ss<<n;
            l.emplace_back(ss.str());
        }
        
        while(!l.empty()){
            l.pop_front();
        }
        if(cycle == 1000){
            cycle = 0;
            this_thread::sleep_for(chrono::milliseconds(100));
            cout<<l.size()<<" "<<l.capacity()<<endl;
        }
       
    }
}
int main(){
    test_member();
//    test_memory_cycle();
}
#endif //XHBLIB_LIST_QUEUE_UNIT_TEST_H_
