// /**
//  * File: thread-pool.h
//  * -------------------
//  * This class defines the ThreadPool class, which accepts a collection
//  * of thunks (which are zero-argument functions that don't return a value)
//  * and schedules them in a FIFO manner to be executed by a constant number
//  * of child threads that exist solely to invoke previously scheduled thunks.
//  */
// #ifndef _thread_pool_
// #define _thread_pool_

// #include <cstddef>
// #include <functional>
// #include <thread>
// #include <vector>
// #include <mutex>
// #include "Semaphore.h"
// #include <atomic>

// using namespace std;



/**
 * @brief Represents a worker in the thread pool.
 * 
 * The `worker_t` struct contains information about a worker 
 * thread in the thread pool. Should be includes the thread object, 
 * availability status, the task to be executed, and a semaphore 
 * (or condition variable) to signal when work is ready for the 
 * worker to process.
 */
// typedef struct worker {
//     thread ts;
//     function<void(void)> thunk;
//     /**
//      * Complete the definition of the worker_t struct here...
//      **/
// } worker_t;

// class ThreadPool {
//   public:

//   /**
//   * Constructs a ThreadPool configured to spawn up to the specified
//   * number of threads.
//   */
//     ThreadPool(size_t numThreads);

//   /**
//   * Schedules the provided thunk (which is something that can
//   * be invoked as a zero-argument function without a return value)
//   * to be executed by one of the ThreadPool's threads as soon as
//   * all previously scheduled thunks have been handled.
//   */
//     void schedule(const function<void(void)>& thunk);

//   /**
//   * Blocks and waits until all previously scheduled thunks
//   * have been executed in full.
//   */
//     void wait();

//   /**
//   * Waits for all previously scheduled thunks to execute, and then
//   * properly brings down the ThreadPool and any resources tapped
//   * over the course of its lifetime.
//   */
//     ~ThreadPool();
    
//   private:

//     void worker(int id);
//     void dispatcher();
//     thread dt;                              // dispatcher thread handle
//     vector<worker_t> wts;                   // worker thread handles. you may want to change/remove this
//     bool done;                              // flag to indicate the pool is being destroyed

//     /* It is incomplete, there should be more private variables to manage the structures... 
//     * *
//     */
  
//     /* ThreadPools are the type of thing that shouldn't be cloneable, since it's
//     * not clear what it means to clone a ThreadPool (should copies of all outstanding
//     * functions to be executed be copied?).
//     *
//     * In order to prevent cloning, we remove the copy constructor and the
//     * assignment operator.  By doing so, the compiler will ensure we never clone
//     * a ThreadPool. */
//     ThreadPool(const ThreadPool& original) = delete;
//     ThreadPool& operator=(const ThreadPool& rhs) = delete;
// };
// #endif

// simplifique la estructura para leerlo bien y que no se complique hacer los struct
// typedef struct worker {
//     thread ts;                      // Hilo del worker
//     function<void(void)> thunk;     // Tarea asignada
//     Semaphore ready{0};            // Semáforo para despertar al worker
//     bool available = true;          // Flag de disponibilidad
// } worker_t;

// class ThreadPool {
//   public:
//     ThreadPool(size_t numThreads);
//     void schedule(const function<void(void)>& thunk);
//     void wait();
//     ~ThreadPool();

//   private:
//     void worker(int id);
//     void dispatcher(size_t numThreads);

//     thread dt;                     // Dispatcher
//     vector<worker_t> wts;          // Workers
//     atomic<bool> done;             // Señal de finalización
//     mutex queueLock;               // Protección extra si necesitás

//     ThreadPool(const ThreadPool& original) = delete;
//     ThreadPool& operator=(const ThreadPool& rhs) = delete;
// };

// #endif

//resumido para que se entienda mejor y poder manipular las clases

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include "Semaphore.h"
#include <atomic>
#include <condition_variable>

using namespace std;

typedef struct worker {
    thread ts;                      // Hilo del worker
    function<void(void)> thunk;     // Tarea asignada
    Semaphore ready{0};             // Semáforo para despertar al worker
    bool available = true;          // Flag de disponibilidad
    mutex wmutex;            // Mutex para proteger el acceso a la tarea
} worker_t;

class ThreadPool {
  public:
    ThreadPool(size_t numThreads);
    void schedule(const function<void(void)>& thunk);
    void wait();
    ~ThreadPool();

  private:
    void worker(int id);
    void dispatcher(size_t numThreads);

    thread dt;                     // dispatcher thread handle
    vector<worker_t> wts;          // workers thread handles. you may want to change/remove this
    atomic<bool> done;             //  flag to indicate the pool is being destroyed

    // Variables
    queue<function<void(void)>> taskQueue;
    mutex taskQueueMutex;
    Semaphore tasksAvailable{0};
    mutex waitMutex;
    condition_variable_any waitCV;
    size_t tasksInProgress = 0;

    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
