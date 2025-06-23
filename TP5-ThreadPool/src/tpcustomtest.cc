/**
 * File: tpcustomtest.cc
 * ---------------------
 * Unit tests *you* write to exercise the ThreadPool in a variety
 * of ways.
 */

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <functional>
#include <cstring>
#include <mutex>
#include <sys/types.h> // used to count the number of threads
#include <unistd.h>    // used to count the number of threads
#include <dirent.h>    // for opendir, readdir, closedir

#include "thread-pool.h"


using namespace std;

void sleep_for(int slp){
    this_thread::sleep_for(chrono::milliseconds(slp));
}

static mutex oslock;

static const size_t kNumThreads = 4;
static const size_t kNumFunctions = 10;
static void simpleTest() {
  ThreadPool pool(kNumThreads);
  for (size_t id = 0; id < kNumFunctions; id++) {
    pool.schedule([id] {
      oslock.lock();
      cout << "Thread (ID: " << id << ") has started." << endl;
      oslock.unlock();
      size_t sleepTime = (id % 3) * 10;
      sleep_for(sleepTime);
      oslock.lock();
      cout <<  "Thread (ID: " << id << ") has finished." << endl ;
      oslock.unlock();
    });
  }

  pool.wait();
}


static void singleThreadNoWaitTest() {
    ThreadPool pool(4);

    pool.schedule([&] {
        oslock.lock();
        cout << "This is a test." << endl;
        oslock.unlock();
    });
    sleep_for(1000); // emulate wait without actually calling wait (that's a different test)
}

static void singleThreadSingleWaitTest() {
    ThreadPool pool(4);
    pool.schedule([] {
        oslock.lock();
        cout << "This is a test." << endl;
        oslock.unlock();
        sleep_for(1000);
    });
}

static void noThreadsDoubleWaitTest() {
    ThreadPool pool(4);
    pool.wait();
    pool.wait();
}

static void reuseThreadPoolTest() {
    ThreadPool pool(4);
    for (size_t i = 0; i < 16; i++) {
        pool.schedule([] {
            oslock.lock();
            cout << "This is a test." << endl;
            oslock.unlock();
            sleep_for(50);
        });
    }
    pool.wait();
    pool.schedule([] {
        oslock.lock();
        cout << "This is a code." << endl;
        oslock.unlock();
        sleep_for(1000);
    }); 
    pool.wait();
}

// mis test

#include <atomic>
static void concurrentSchedulingTest() {
    ThreadPool pool(4);
    const int numThreads = 10;
    const int tasksPerThread = 100;
    atomic<int> counter(0); 

    vector<thread> threads;

    for (int t = 0; t < numThreads; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < tasksPerThread; i++) {
                pool.schedule([&counter]() {  // capture counter by reference
                    counter++;
                });
            }
        });
    }

    for (auto& th : threads) th.join();
    pool.wait();

    cout << "Total scheduled: " << counter.load() << endl;
    if (counter != numThreads * tasksPerThread) {
        cout << "Error: Race condition in schedule()" << endl;
    }
}
static void scheduleAfterDestroyTest() {
    ThreadPool* pool = new ThreadPool(2);
    pool->schedule([]() { cout << "Task A\n"; });
    pool->wait();
    delete pool;

    try {
        pool->schedule([]() { cout << "Task B\n"; });
        cout << " Error: Se pudo hacer schedule luego del destroy.\n";
    } catch (const exception& e) {
        cout << " Correctamente falló al hacer schedule: " << e.what() << endl;
    }
}
static void mixedTaskTest() {
    ThreadPool pool(4);
    atomic<int> fast(0), slow(0); // direct initialization

    for (int i = 0; i < 5; ++i) {
        pool.schedule([&fast]() { fast++; });         // capture by reference
        pool.schedule([&slow]() { 
            sleep_for(500);
            slow++; 
        });
    }
    pool.wait();
    cout << "Fast: " << fast << ", Slow: " << slow << endl;
}

static void countTasksTest() {
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    const int totalTasks = 100;

    for (int i = 0; i < totalTasks; i++) {
        pool.schedule([&counter] {
            sleep_for(10);
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.wait();

    oslock.lock();
    cout << "Total tasks completed: " << counter.load() << endl;
    oslock.unlock();

    if (counter != totalTasks) {
        oslock.lock();
        cout << "Error: Not all tasks completed!" << endl;
        oslock.unlock();
    }
}

static void stressTest() {
    ThreadPool pool(8);
    const int totalTasks = 1000;
    std::atomic<int> completed{0};

    for (int i = 0; i < totalTasks; i++) {
        pool.schedule([&completed] {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.wait();

    oslock.lock();
    cout << "Stress test tasks completed: " << completed.load() << "/" << totalTasks << endl;
    oslock.unlock();
}

static void noWaitCancellationTest() {
    ThreadPool pool(4);

    for (int i = 0; i < 20; i++) {
        pool.schedule([] {
            sleep_for(50);
            oslock.lock();
            cout << "Task executed." << endl;
            oslock.unlock();
        });
    }
    // No hacemos pool.wait() intencionalmente.
    sleep_for(1000); // dejamos tiempo a que corran algunas tareas
    oslock.lock();
    cout << "No wait test completed without crash." << endl;
    oslock.unlock();
}

static void mutexSynchronizationTest() {
    ThreadPool pool(4);
    int sharedCounter = 0;
    mutex mtx;
    const int totalTasks = 50;

    for (int i = 0; i < totalTasks; i++) {
        pool.schedule([&] {
            std::lock_guard<mutex> lock(mtx);
            int local = sharedCounter;
            sleep_for(5);  // simula trabajo
            sharedCounter = local + 1;
        });
    }

    pool.wait();

    oslock.lock();
    cout << "Shared counter value: " << sharedCounter << " (expected " << totalTasks << ")" << endl;
    oslock.unlock();

    if (sharedCounter != totalTasks) {
        oslock.lock();
        cout << "Error: Data race or lost updates detected!" << endl;
        oslock.unlock();
    }
}
struct testEntry {
    string flag;
    function<void(void)> testfn;
};

static void buildMap(map<string, function<void(void)>>& testFunctionMap) {
    testEntry entries[] = {
        {"--single-thread-no-wait", singleThreadNoWaitTest},
        {"--single-thread-single-wait", singleThreadSingleWaitTest},
        {"--no-threads-double-wait", noThreadsDoubleWaitTest},
        {"--reuse-thread-pool", reuseThreadPoolTest},
        {"--s", simpleTest},
        {"--count-tasks", countTasksTest},               // mis test
        {"--stress", stressTest},                        
        {"--no-wait-cancel", noWaitCancellationTest},   
        {"--mutex-sync", mutexSynchronizationTest},
        {"--concurrent-scheduling", concurrentSchedulingTest},
        {"--schedule-after-destroy", scheduleAfterDestroyTest},
        {"--mixed-tasks", mixedTaskTest},
    };

    for (const testEntry& entry: entries) {
        testFunctionMap[entry.flag] = entry.testfn;
    }
}

static void executeAll(const map<string, function<void(void)>>& testFunctionMap) {
    for (const auto& entry: testFunctionMap) {
        cout << entry.first << ":" << endl;
        entry.second();
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Ouch! I need exactly two arguments." << endl;
        return 0;
    }

    map<string, function<void(void)>> testFunctionMap;
    buildMap(testFunctionMap);
    string flag = argv[1];
    if (flag == "--all") {
        executeAll(testFunctionMap);
        return 0;
    }
    auto found = testFunctionMap.find(argv[1]);
    if (found == testFunctionMap.end()) {
        cout << "Oops... we don't recognize the flag \"" << argv[1] << "\"." << endl;
        return 0;
    }

    found->second();
    return 0;
}


