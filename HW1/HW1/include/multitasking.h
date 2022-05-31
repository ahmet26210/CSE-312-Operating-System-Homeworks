 
#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>


namespace myos
{
    class Task;

    
    struct CPUState
    {
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t ecx;
        common::uint32_t edx;

        common::uint32_t esi;
        common::uint32_t edi;
        common::uint32_t ebp;

        /*
        common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds;
        */
        common::uint32_t error;

        common::uint32_t eip;
        common::uint32_t cs;
        common::uint32_t eflags;
        common::uint32_t esp;
        common::uint32_t ss;        
    } __attribute__((packed));
    
    class Thread{
        friend class ThreadManager;

        private:
            common::uint8_t stack[4096]; // 4 KiB

            CPUState* cpustate;
            int id;
            void* func;
            bool is_yield; //check yield situation
            bool is_join; // check join situation

        public:
            Thread();
            Thread(int id);
            Thread(GlobalDescriptorTable *gdt,void (*f)());
            void Thread_initialize(GlobalDescriptorTable *gdt,void (*f)());
            void* getfunc(){return func;}
            void  setfunc(void (*f)()){ func = &f;}
            CPUState* getCPU(){return cpustate;}
            int getid(){return id;}
            bool getyield(){return is_yield;}
            void  setyield(bool a){ is_yield = a;}
            bool getjoin(){return is_join;}
            void  setjoin(bool a){ is_join = a;}
            
    };
    
    class ThreadManager{
        
        private:
            Thread* thread[256];
            int currentThread;
            int numThread;
            
        public:
            ThreadManager();
            Thread* get_next_thread(int index){return thread[index];}
            bool thread_add(Thread* thread); // add thread 
            void thread_delete(Thread* thread); // delete thread
            void let_another_thread_run(Thread* thread); // yield function
            void thread_join_function(Thread* thread1, Thread* thread2); // join function
            int getNumTHhread(){ return numThread;}
            int getCurrentThread(){ return currentThread;}
            CPUState* Schedule(CPUState* cpustate);
            void set_thread(int index,Thread* thread1){thread[index]=thread1;}
            void setCurrentThread(int number){currentThread =number;}
            
    };      

    class Task
    {
    friend class TaskManager;
    private:
        common::uint8_t stack[4096]; // 4 KiB
        CPUState* cpustate;
        ThreadManager* thread_manager; // to manage the threads

        
        
    public:
        
        void setTM(ThreadManager* th);
        Task(ThreadManager* th);
        Task(GlobalDescriptorTable *gdt, void entrypoint());
        ~Task();
      

    };
    
    
    class TaskManager
    {
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
    public:
        TaskManager();
        ~TaskManager();
        bool AddTask(Task* task);
        CPUState* Schedule(CPUState* cpustate);
    };
    
    
}


#endif