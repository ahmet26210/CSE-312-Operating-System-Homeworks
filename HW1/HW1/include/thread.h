#ifndef _MYOS_THREAD_H
#define _MYOS_THREAD_H

#include <common/types.h>
#include <gdt.h>
#include <multitasking.h>

namespace myos
{
    
    class Thread{
        
        private:
            common::uint8_t stack[4096]; // 4 KiB
            CPUState* cpustate;
            
            void* func;

        public:
            
            Thread(GlobalDescriptorTable *gdt,void (*f)());
           
            void* getfunc(){return func;}
            void  setfunc(void (*f)()){ func = f;}
        
    };

    class ThreadManager{
        
        private:
            Thread* thread[256];
            int currentThread;
            int numThread;
            
        public:
            ThreadManager();
            bool thread_create(Thread* thread);
            bool thread_exit();
            void thread_join();
            void thread_yield();
    };

} // namespace myos



#endif