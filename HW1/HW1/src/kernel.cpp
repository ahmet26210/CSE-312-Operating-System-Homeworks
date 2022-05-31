
#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>


// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

#define FALSE 0
#define TRUE 1
#define N 2

int turn;
int interested[N];
int numberglobal;

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}




class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

void ProduceItem(int num){ // Producer
    printf("Produced: ");
    printfHex(num);
    numberglobal = num;
}

void ConsumeItem(int num){ // Consumer
    numberglobal = num;
    printf("Consumed: ");
    printfHex(num);
}

void enter_region(int process){ // Peterson Algorithm
    int other;

    other = 1-process;

    interested[process] =TRUE;

    turn = process;

    while (turn == process && interested[other] == TRUE);
}

void leave_region(int process) // Peterson Algorithm
{
    interested[process] = FALSE;
}


void TaskAfuncA(){
   while(true){
        enter_region(0);
        //Critical Region Starts
        for (int i = 0; i < 30; i++) {
           
           ProduceItem(i);
         
        }
        //Critical Region Ends
        leave_region(0);

        //printf("A");
        for(uint32_t i=0; i<9999; ++i){ // for looking easily the result
            for(uint32_t j=0; j<9999; ++j){
            }
        }
    }
}
void TaskAfuncB(){
    while(true){
        enter_region(1);
        //Critical Region Starts
        for (int i = 0; i < 30; i++) {
           ConsumeItem(i);
        }
        //Critical Region Ends
        leave_region(1);
        //printf("B");
        for(uint32_t i=0; i<9999; ++i){
            for(uint32_t j=0; j<9999; ++j){
            }
        }
    }
}
void TaskBfuncA(){
    
    while(true){
        printf("C");
        for(uint32_t i=0; i<9999; ++i){
            for(uint32_t j=0; j<9999; ++j){
            }
        }
    }
}
void TaskBfuncB(){
    while(true){
        printf("D");
        for(uint32_t i=0; i<9999; ++i){
            for(uint32_t j=0; j<9999; ++j){
            }
        }
    }
}



void taskA()
{
    while(true)
        printf("A");
}
void taskB()
{
    while(true)
        printf("B");
}



void thread_create(Thread* thread , GlobalDescriptorTable* gdt , void (*f)()){
    
    thread->Thread_initialize(gdt,f);

}

void thread_exit(Thread* thread1,ThreadManager* thread_manager){
    
    thread_manager->thread_delete(thread1);
    
}

void thread_yield(ThreadManager* thread_manager,Thread* thread){
    thread_manager->let_another_thread_run(thread);
}

void thread_join(ThreadManager* thread_manager,Thread* thread1,Thread* thread2){
    thread_manager->thread_join_function(thread1,thread2);
}



typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    GlobalDescriptorTable gdt;

    Thread thread1(0); // Producer
    Thread thread2(1); // Consumer
    Thread thread3(2); // yield join
    Thread thread4(3); // yield join
    // Thread thread5(4);
    // Thread thread6(5);
    // Thread thread7(6);
    // Thread thread8(7);

    
    thread_create(&thread1, &gdt, TaskAfuncA); 
    thread_create(&thread2, &gdt, TaskAfuncB);

    thread_create(&thread3, &gdt, TaskBfuncA);
    thread_create(&thread4, &gdt, TaskBfuncB);

    ThreadManager thread_manager , thread_manager1;

    // thread_create(&thread5, &gdt, TaskBfuncA);
    // thread_create(&thread6, &gdt, TaskBfuncB);

    // thread_create(&thread7, &gdt, TaskBfuncA);
    // thread_create(&thread8, &gdt, TaskBfuncB);

    thread_manager.thread_add(&thread1);
    thread_manager.thread_add(&thread2);
    
    thread_manager1.thread_add(&thread3);
    thread_manager1.thread_add(&thread4);

    // thread_manager1.thread_add(&thread5);
    // thread_manager1.thread_add(&thread6);

    // thread_manager1.thread_add(&thread7);
    // thread_manager1.thread_add(&thread8);

    //thread_exit(&thread1,&thread_manager);
    
    thread_join(&thread_manager1,&thread3,&thread4);
    
    thread_exit(&thread4,&thread_manager1);
    
    //thread_yield(&thread_manager1,&thread3);
    
    TaskManager taskManager;
    
    Task task1(&thread_manager);
    Task task2(&thread_manager1);

    taskManager.AddTask(&task1);

    //taskManager.AddTask(&task2);


    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    printf("Initializing Hardware, Stage 1\n");
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        VideoGraphicsArray vga;
        
    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif


    interrupts.Activate();
    
    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
