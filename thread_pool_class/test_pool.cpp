#include"task_pool.h"
#include<chrono>

int main(){
    TaskPool threadPool;
    threadPool.init();

    Task* task = NULL;

    for(int i=0; i<16; ++i){
        task = new Task();
        threadPool.addTask(task);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    threadPool.stop();
    
    return 0;
}