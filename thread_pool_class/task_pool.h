#include<thread>
#include<mutex>
#include<list>
#include<vector>
#include<memory>
#include<iostream>
#include<condition_variable>
#include<functional>

class Task{
public:
    virtual void task_do_sth(){
        std::cout << "handle a task ..." << std::endl;
    }
    virtual ~Task(){
        std::cout << "a task destructed ..." << std::endl;
    }

};

class TaskPool{
public:
    TaskPool();
    ~TaskPool();
    TaskPool(const TaskPool& t) = delete;
    TaskPool& operator=(const TaskPool& t) = delete; // 拷贝构造和赋值运算符都被禁止使用
    void init(int threadNum = 10); // 初始化线程池，默认为10个线程
    void stop();
    void addTask(Task* task);
    void removeAllTasks();

private:
    void threadFunc(); // 线程函数，使用thread类时，需要显示的将线程函数所属的类对象实例指针作为构造函数的参数传递给thread
    std::list<std::shared_ptr<Task>> m_taskList; // 任务列表
    std::vector<std::shared_ptr<std::thread>> m_threads;
    bool m_bRunning;
    std::mutex  m_mutex; // 共享变量，配合condition_vatiable使用。避免虚假唤醒问题
    std::condition_variable m_cv; // 使用条件变量实现同步机制

};