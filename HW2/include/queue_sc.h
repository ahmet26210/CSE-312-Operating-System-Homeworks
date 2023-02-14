using namespace myos;
#define PAGE_SIZE 4

namespace myos
{
    typedef struct page_entry_class
    {
        int referenced_bit;
        int counter;
        int modified_bit;
        int protection_bit;
        int valid_bit; // present/absent
        int page_frame_num;
        int data[PAGE_SIZE];
        int virtual_adress;
    } page_entry_class;

    class Queue_SC {
        public:
            int front, rear, size;
            int capacity;//unsigned capacity;
            page_entry_class* array;
           
        Queue_SC(int cap)
        {
            capacity = cap;
            front = size = 0;
        
            rear = capacity - 1;
            array = new page_entry_class[capacity];
        }
        
        int isFull()
        {
            return (size == capacity);
        }
        
        int isEmpty()
        {
            return (size == 0);
        }
        
        int add(page_entry_class item)
        {
            if (isFull()){
                return capacity;

            }
            rear = (rear + 1)
                        % capacity;
            array[rear] = item;
            size = size + 1;
        }
        
        page_entry_class remove()
        {

            page_entry_class item = array[front];
            front = (front + 1)
                        % capacity;
            size = size - 1;
            return item;
        }
        
        page_entry_class front_sc()
        {
            return array[front];
        }
        
        page_entry_class rear_sc()
        {
            
            return array[rear];
        }
    };

      
}