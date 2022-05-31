
#include<multitasking.h>
using namespace myos;
using namespace myos::common;

Thread::Thread(GlobalDescriptorTable* gdt,void entrypoint()){

    Thread_initialize(gdt,entrypoint); 
    
}
Thread::Thread(){
    
}
Thread::Thread(int id){
    this->id = id;
    
}


ThreadManager::ThreadManager(){
    currentThread = 0;
    numThread = 0;
}

bool ThreadManager::thread_add(Thread* thread){

    if(numThread >= 256)
        return false;
    
    this->thread[numThread++] = thread;
    return true;
}
void ThreadManager::thread_delete(Thread* thread1){ 
    thread1->setjoin(false);
    int k =0;

    Thread* thread[256];

    for(int i =0; i<256; i++ ){

        if(get_next_thread(i)->getid()!= thread1->getid()){
            thread[k] = get_next_thread(i);
            k++;
        }

    }

    for(int i =0; i<256; i++ ){
        set_thread(i,nullptr);
    }

    for(int i =0; i<256; i++ ){
        set_thread(i,thread[i]);
    }

    numThread--;
}
void ThreadManager::let_another_thread_run(Thread* thread){
    thread->setyield(true);
}
void ThreadManager::thread_join_function(Thread* thread1,Thread* thread2){
    thread1->setjoin(true);
    thread2->setjoin(true);
}


CPUState* ThreadManager::Schedule(CPUState* cpustate)
{
    
    

    if(numThread <= 0)
        return cpustate;
     
    if(currentThread >= 0)
        thread[currentThread]->cpustate = cpustate;

    if(++currentThread >= numThread)
        currentThread %= numThread;

    if(thread[currentThread]->getyield() == true && (thread[currentThread]->getid()==2)){

        thread[currentThread]->setyield(false);
        
        currentThread++;

    }

    if(thread[currentThread]->getyield() == true && (thread[currentThread]->getid()==3)){

        thread[currentThread]->setyield(false);
        
        currentThread--;

    }

    if(thread[currentThread]->getjoin() == true && thread[currentThread+1]->getjoin()== true){
        currentThread++;
    }

    return thread[currentThread]->cpustate;
}

void Thread::Thread_initialize(GlobalDescriptorTable *gdt,void (*f)()){
    cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
    
    cpustate -> eax = 0;
    cpustate -> ebx = 0;
    cpustate -> ecx = 0;
    cpustate -> edx = 0;

    cpustate -> esi = 0;
    cpustate -> edi = 0;
    cpustate -> ebp = 0;
    
    /*
    cpustate -> gs = 0;
    cpustate -> fs = 0;
    cpustate -> es = 0;
    cpustate -> ds = 0;
    */
    
    // cpustate -> error = 0;    
   
    // cpustate -> esp = ;
    cpustate -> eip = (uint32_t)f;
    cpustate -> cs = gdt->CodeSegmentSelector();
    // cpustate -> ss = ;
    cpustate -> eflags = 0x202;

    this->func = &f;
}

