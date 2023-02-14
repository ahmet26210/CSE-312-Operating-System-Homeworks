

#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>
#include <queue_sc.h>
// #define GRAPHICSMODE

#define NUMBER_OF_PAGE 5
#define SIZE_OF_PM 2
#define SIZE_OF_PAGE 4

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
void printf(char *str);
void printfHex(uint8_t key);

void printf(char *str)
{
    static uint16_t *VideoMemory = (uint16_t *)0xb8000;

    static uint8_t x = 0, y = 0;

    for (int i = 0; str[i] != '\0'; ++i)
    {
        switch (str[i])
        {
        case '\n':
            x = 0;
            y++;
            break;
        default:
            VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | str[i];
            x++;
            break;
        }

        if (x >= 80)
        {
            x = 0;
            y++;
        }

        if (y >= 25)
        {
            for (y = 0; y < 25; y++)
                for (x = 0; x < 80; x++)
                    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char *foo = "00";
    char *hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char *foo = " ";
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
        uint16_t *VideoMemory = (uint16_t *)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
    }

    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t *VideoMemory = (uint16_t *)0xb8000;
        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);

        x += xoffset;
        if (x >= 80)
            x = 79;
        if (x < 0)
            x = 0;
        y += yoffset;
        if (y >= 25)
            y = 24;
        if (y < 0)
            y = 0;

        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 | (VideoMemory[80 * y + x] & 0xF000) >> 4 | (VideoMemory[80 * y + x] & 0x00FF);
    }
};

void taskA()
{
    while (true)
        printf("A");
}
void taskB()
{
    while (true)
        printf("B");
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for (constructor *i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

class System_Class
{

    int size_of_array;
    int hit_rate;
    int mis_rate;
    int write_back_rate;
    int number_of_page_loaded;

public:
    double size_respect_pm = 2;
    char *algorithms[3] = {"bubblesort", "insertionsort", "quicksort"};
    char *arrOrderType[2] = {"random", "sorted"};
    int given_array[20] = {5, 0, 4, 8, 10, 8, 29, 29, 29, 3, 38, 4, 4, 61, 8, 92, 1, 23, 14, 14};

    page_entry_class virtualMemory[NUMBER_OF_PAGE]; // size - 5 page * 4 = 20 array val
    page_entry_class physicalMemory[SIZE_OF_PM];
    int pageTable[NUMBER_OF_PAGE];

    
    System_Class()
    {
        size_of_array = 20; // sizeof(arr) / sizeof(given_array[0]); // NUMBER_OF_PAGE * sizeRespectToPM;

        for (int i = 0; i < NUMBER_OF_PAGE; i++)
            pageTable[i] = -1; // pages are in disk // hexa -> f

        int j = 0, k = 0;
        for (int i = 0; i < NUMBER_OF_PAGE; i++)
        {
            virtualMemory[i].virtual_adress = i;
            while (j < PAGE_SIZE)
            {
                virtualMemory[i].data[j % PAGE_SIZE] = given_array[k++];
                virtualMemory[i].counter = 0; 
                j++;
            }
            j = 0;
        }

        if(int result = sortProgram(2, "insertionsort", "random")!=-1){
            printf("\n");
            for (size_t l = 0; l < 20; l++)
            {   
                printfHex(given_array[l]);
                printf(" ");
            }
            printf("\n");
        }

    }

    int compare(char str1[], char str2[])
    {
        int flag = 0, i = 0;
        while (str1[i] != '\0' && str2[i] != '\0')
        {
            if (str1[i] != str2[i])
            {
                flag = 1;
                break;
            }
            i++;
        }
        return flag;
    }

    int sortProgram(double sizeRespectToPM, char *algorithm, char *arrOrderType)
    {
        size_of_array= size_respect_pm * SIZE_OF_PM * PAGE_SIZE;
        if(size_of_array / PAGE_SIZE > NUMBER_OF_PAGE){
            printf("error");
            return -1;
        }
        int compResult, select;
        for (int i = 0; i < 3; i++)
        {
            int compResult = compare(algorithm, algorithms[i]);
            if (compResult == 0) // matched
                select = i;
        }

        switch (select)
        {
        case 0:
            bubbleSort(*this, size_of_array);
            break;
        case 1:
            insertionSort(*this, size_of_array);
            break;
        case 2:
           
            break;
        default:
            break;
        }
    }

    void swap_function(int *a, int *b)
    {
        int t = *a;
        *a = *b;
        *b = t;
    }

    int get_hit_rate()
    {
        return hit_rate;
    }
    int get_mis_rate()
    {
        return mis_rate;
    }
    int get_write_rate()
    {
        return write_back_rate;
    }
    int get_page_loaded_rate()
    {
        return number_of_page_loaded;
    }

    void insertionSort(System_Class &given_obj, int size)
    {
        for (size_t i = 0; i < SIZE_OF_PM; i++) 
        {
            physicalMemory[i].counter = 0;
        }

        int key, j;
        for (int i = 1; i < size; i++)
        {
            j = i;
            while (j > 0 && given_array[j] < given_array[j - 1])
            {
                swap_function(given_obj[j], given_obj[j - 1]);
                int temp = given_array[j];
                given_array[j] = given_array[j - 1];
                given_array[j - 1] = temp;
                j--;
                int tmp = j / PAGE_SIZE;
                int tmp2 = j - 1 / PAGE_SIZE;
                int address = page_table_entry_check(tmp);
                int address2 = page_table_entry_check(tmp2);
                if (address != -1)
                {
                    physicalMemory[address].modified_bit= 1;
                }
                if (address2 != -1)
                {
                    physicalMemory[address2].modified_bit = 1;
                }
                for (int i = 0; i < SIZE_OF_PM; i++)
                {
                    physicalMemory[i].counter += physicalMemory[i].referenced_bit;
                }
            }

            if ((i % 8 == 0 && i != 0) || i == size - 1)
                print_current_situtation();
        }
    }

    void print_current_situtation(){
        printf("\n");
        printf("Number of hit rate: ");
        printfHex(hit_rate);
        printf("\n");
        printf("Number of miss rate: ");
        printfHex(mis_rate);
        printf("\n");
        printf("Number of write back: ");
        printfHex(write_back_rate);
        printf("\n");
        printf("Number of page loaded: ");
        printfHex(number_of_page_loaded);
        printf("\n");
    }

    void bubbleSort(System_Class &given_obj, int size)
    {
        int i, j, flag = 0;

        for (size_t i = 0; i < SIZE_OF_PM; i++)
        {
            physicalMemory[i].counter = 0;
        }

        for (i = 0; i < size; i++)
        {
            
            for (j = 0; j < size - i - 1; j++)
            {
                if (given_array[j] > given_array[j + 1])
                {
                   
                    swap_function(given_obj[j], given_obj[j + 1]);

                    int temp = given_array[j];
                    given_array[j] = given_array[j + 1];
                    given_array[j + 1] = temp;
                  
                    int tmp = j/ PAGE_SIZE;
                    int tmp2 = j + 1 / PAGE_SIZE;
                    int address = page_table_entry_check(tmp);
                    int address2 = page_table_entry_check(tmp2);
                    if (address != -1)
                    {
                        physicalMemory[address].modified_bit = 1;
                    }
                    if (address2 != -1)
                    {
                        physicalMemory[address2].modified_bit = 1;
                    }
                    flag = 1;

                    for (int i = 0; i < SIZE_OF_PM; i++)
                    {
                        physicalMemory[i].counter += physicalMemory[i].referenced_bit;
                    }
                }
            }

            if ((i % 8 == 0 && i != 0) || i == size - 1)
                print_current_situtation();

            if (flag == 0)
                break;
        }
    }

    void swap_function(int &i, int &j)
    {
        int temp = i;
        j = i;
        i = temp;
    }

    int &operator[](int i)
    {
        int page_index = i / PAGE_SIZE;
        int offset = i % PAGE_SIZE;
        int frameNum = page_table_entry_check(page_index);
        if (frameNum != -1)
        {
            physicalMemory[frameNum].referenced_bit = 1; 
            hit_rate++;
        }
        else
        {
            page_replacement_function(page_index);
        }

        return physicalMemory[page_index].data[offset];
    }

    int page_table_entry_check(int page_index)
    {
        return pageTable[page_index];
    }

    void page_replacement_function(int page_index)
    {
    
        int flag = 0;
        int temp_index = 0;
        int address = 0;
        int select_bit = 0; 

        mis_rate++;
        number_of_page_loaded++;

        for (int i = 0; i < SIZE_OF_PM; i++)
        {
            if (physicalMemory[i].valid_bit == 0)
            {
                physicalMemory[i] = virtualMemory[page_index];
                physicalMemory[i].valid_bit = 1;
                physicalMemory[i].referenced_bit = 1;
                pageTable[page_index] = i;
                flag = 1;

                break;
            }
        }

        if (flag == 0)
        {

            if (select_bit == 0)
            {
                address = physicalMemory[temp_index].virtual_adress;
                if (physicalMemory[temp_index].modified_bit == 1)
                {
                    virtualMemory[address] = physicalMemory[temp_index]; 
                    virtualMemory[address].valid_bit = 0;
                    write_back_rate++;
                }
                pageTable[page_index] = temp_index;
                pageTable[address] = -1;       
                physicalMemory[temp_index] = virtualMemory[page_index]; 
                physicalMemory[temp_index].valid_bit = 1;

                temp_index++;
                temp_index %= NUMBER_OF_PAGE;
            }
            else if (select_bit == 1)
            {

                Queue_SC second_chance_queue(SIZE_OF_PM);

                for (int i = 0; i < SIZE_OF_PM; i++)
                {
                    page_entry_class tmp = physicalMemory[i];
                    second_chance_queue.add(tmp);
                }
             
                for (int i = 0; i < SIZE_OF_PM; i++)
                {
                    if (second_chance_queue.front_sc().referenced_bit == 0)
                    {
                        second_chance_queue.front_sc() = virtualMemory[page_index];
                        break;
                    }
                    else
                    {
                        page_entry_class temp = second_chance_queue.remove();
                        temp.referenced_bit = 0;
                        second_chance_queue.add(temp);
                    }
                }

                for (int i = 0; i < SIZE_OF_PM; i++)
                {
                    address = physicalMemory[i].virtual_adress;
                    if (physicalMemory[i].modified_bit == 1)
                    {
                        virtualMemory[address] = physicalMemory[i];
                        virtualMemory[address].valid_bit= 0;
                        write_back_rate++;
                    }

                    page_entry_class tmp = second_chance_queue.remove();
                    physicalMemory[i] = tmp;
                    physicalMemory[i].valid_bit = 1;

                    pageTable[page_index] = tmp.virtual_adress; 
                    pageTable[address] = -1;                  
                }
            }
            else if(select_bit == 2)
            {
                
                int index = 0;
                int min = physicalMemory[0].counter;
                for (int i = 0; i < SIZE_OF_PM; i++)
                {
                    if (physicalMemory[i].counter < min)
                    {
                        min = physicalMemory[i].counter;
                        index = i;
                    }
                }

                address = physicalMemory[index].virtual_adress;
                if (physicalMemory[index].modified_bit == 1)
                {
                    virtualMemory[address] = physicalMemory[index]; 
                    virtualMemory[address].valid_bit = 0;
                    write_back_rate++;
                }

                pageTable[page_index] = index; 
                pageTable[address] = -1;      
                physicalMemory[index] = virtualMemory[page_index]; 
                physicalMemory[index].valid_bit = 1;
            }
        }
    }

};



extern "C" void kernelMain(const void *multiboot_structure, uint32_t /*multiboot_magic*/)
{

    GlobalDescriptorTable gdt;

    uint32_t *memupper = (uint32_t *)(((size_t)multiboot_structure) + 8);
    size_t heap = 10 * 1024 * 1024;
    MemoryManager memoryManager(heap, (*memupper) * 1024 - heap - 10 * 1024);


    System_Class system_test;
    printf("Number of hit rate is: ");
    printfHex(system_test.get_hit_rate());
    printf("\n");
    printf("Number of miss rate is : ");
    printfHex(system_test.get_mis_rate());
    printf("\n");
    printf("Number of write back is : ");
    printfHex(system_test.get_write_rate());
    printf("\n");
    printf("Number of loaded page is : ");
    printfHex(system_test.get_page_loaded_rate());

    TaskManager taskManager;
    InterruptManager interrupts(0x20, &gdt, &taskManager);

#ifdef GRAPHICSMODE
    Desktop desktop(320, 200, 0x00, 0x00, 0xA8);
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

    drvManager.ActivateAll();

#ifdef GRAPHICSMODE
    vga.SetMode(320, 200, 8);
    Window win1(&desktop, 10, 10, 20, 20, 0xA8, 0x00, 0x00);
    desktop.AddChild(&win1);
    Window win2(&desktop, 40, 15, 30, 30, 0x00, 0xA8, 0x00);
    desktop.AddChild(&win2);
#endif

    interrupts.Activate();

    while (1)
    {
#ifdef GRAPHICSMODE
        desktop.Draw(&vga);
#endif
    }
}