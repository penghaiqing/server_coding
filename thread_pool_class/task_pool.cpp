#include"task_pool.h"

TaskPool::TaskPool(): m_bRunning(false){}

TaskPool::~TaskPool(){
    removeAllTasks();
}

void TaskPool::init(int threadNum){
    if(threadNum <= 0){
        threadNum = 10;
    }
    m_bRunning = true;

    for(int i=0; i<threadNum; ++i){
        std::shared_ptr<std::thread> spThread;
        spThread.reset(new std::thread(std::bind(&TaskPool::threadFunc, this))); //
        //spThread.reset(new std::thread(&TaskPool::threadFunc, this));
        m_threads.push_back(spThread);
    }
}

void TaskPool::stop(){
    m_bRunning = false;
    m_cv.notify_all();

    for(auto &iter : m_threads){
        if(iter->joinable())
            iter->join(); // 等待线程退出，前提是线程处于运行状态，即 可join的
    }
}

void TaskPool::addTask(Task* task){
    std::shared_ptr<Task> spTask;
    spTask.reset(task);
    {
        std::lock_guard<std::mutex> guard(m_mutex); // 获取锁, + suo
        m_taskList.push_back(spTask);
        std::cout << "add a task in tasklist." << std::endl;
    }

    m_cv.notify_one(); // 通知条件变量，等待方接收到通知条件变量，线程会被唤醒，并自动获取锁。
}

void TaskPool::removeAllTasks(){
    std::lock_guard<std::mutex> guard(m_mutex);
    for(auto& iter : m_taskList){
        iter.reset();
    }
    m_taskList.clear();
}

void TaskPool::threadFunc(){
    std::shared_ptr<Task> spTask;
    while(true){
        {   // guard 锁的作用范围，使用{} 也是为了减小锁的作用范围
            std::unique_lock<std::mutex> guard(m_mutex);
            while(m_taskList.empty()){
                if(!m_bRunning)
                    break;
                // 此时，获得了互斥锁，但是任务队列为空，
                // 调用wait()会释放锁，且挂起当前线程，不向下执行
                // 当发生变化，条件满足时，wait() 会唤醒挂起的线程且获得锁
                m_cv.wait(guard);
            }

            // 还要判一次 break
            if(!m_bRunning)
                break;
            // 此时，表示都满足了，可以进行执行线程了
            spTask = m_taskList.front();
            m_taskList.pop_front();
        }

        spTask->task_do_sth();
        spTask.reset();
    }
    std::cout << "exit thread ID :" << std::this_thread::get_id() << std::endl;
}