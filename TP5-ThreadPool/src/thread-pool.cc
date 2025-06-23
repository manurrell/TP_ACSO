

#include "thread-pool.h"

using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    // Inicio los threads y el dispatcher
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].ts = thread([this, i]() { this->worker(i); });
    }
    dt = thread([this, numThreads]() { this->dispatcher(numThreads); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    // Si el pool ya está cerrado o destruido no se puede llamar a schedule
    if (done.load()) {
        throw runtime_error("ThreadPool already destroyed or closed");
    }
    if (!thunk) return;
    {
        // Agrego la tarea a la cola
        lock_guard<mutex> lock(taskQueueMutex);
        taskQueue.push(thunk);
        tasksAvailable.signal();
    }
}

void ThreadPool::dispatcher(size_t numThreads) {
    while (true) {
        // Espera a que haya tareas disponibles
        tasksAvailable.wait();

        if (done.load()) break;

        function<void(void)> task;
        {
            // Toma una tarea de la cola
            lock_guard<mutex> lock(taskQueueMutex);
            if (!taskQueue.empty()) {
                task = taskQueue.front();
                taskQueue.pop();
            } else {
                continue;
            }
        }

        if (numThreads == 1) {
            {
                // Si solo hay un thread, se ejecuta directamente, dispatcher = worker
                unique_lock<mutex> lock(waitMutex);
                ++tasksInProgress;
            }
            task();
            {
                unique_lock<mutex> lock(waitMutex);
                --tasksInProgress;
                waitCV.notify_all();
            }
        } else {
            bool assigned = false;
            while (!assigned) {
                for (size_t i = 0; i < wts.size(); ++i) {
                    // Busca un worker disponible
                    //el mutex protege el worker y la tarea asignada
                    lock_guard<mutex> guard(wts[i].wmutex);
                    if (wts[i].available) {
                        wts[i].available = false;
                        wts[i].thunk = task;
                        wts[i].ready.signal();
                        assigned = true;
                        break;
                    }
                }
                // Si no se encontró un worker disponible, espera sin consumir recursos asi terminan los workers
                if (!assigned) this_thread::yield();
            }
        }
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        // Espera a que haya una tarea asignada
        wts[id].ready.wait();
        // Si el pool está cerrado o destruido, sale del loop y mata el thread
        if (done.load()) break;

        {
            // Incrementa el contador de tareas y bloquea el mutex para evitar que otros threads accedan
            unique_lock<mutex> lock(waitMutex);
            ++tasksInProgress;
        }
        // corre la task
        wts[id].thunk();
        {
            //indica que termino la tarea y notifica a wait()
            unique_lock<mutex> lock(waitMutex);
            --tasksInProgress;
            waitCV.notify_all();
        }
        // Marca el worker como disponible para recibir nuevas tareas
        lock_guard<mutex> guard(wts[id].wmutex);
        wts[id].available = true;
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(waitMutex);
    waitCV.wait(lock, [this] {
        lock_guard<mutex> qlock(taskQueueMutex);
        return taskQueue.empty() && tasksInProgress == 0;
    });
}


ThreadPool::~ThreadPool() {
    wait();
    //done indica  que el pool termino su tarea y se va a destruir
    done.store(true);
    // Despierta a todos los workers para que lean el done y salgan del loop matando el thread
    for (auto& w : wts) {
        w.ready.signal();
    }
    tasksAvailable.signal();
    // Espera a que todos los threads terminen asi no termina el programa antes de que terminen
    for (auto& w : wts) {
        if (w.ts.joinable()) w.ts.join();
    }
    if (dt.joinable()) dt.join();
}
