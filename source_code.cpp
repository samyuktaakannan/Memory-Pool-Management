#include <windows.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <pthread.h>
using namespace std;
class data
{
    char membuffer[4096];
};

void nomemorypool()
{
    //Retrieves the number of milliseconds that have elapsed since the system was started.
    DWORD count=GetTickCount();
    for(int i=0; i<0x5fffff; i++)
    {
        data *p = new data;
        delete p;
    }
    cout<<"Time taken = "<<GetTickCount()-count<<" ms"<<endl;
}

char buf[4100];
class shareddata
{
    char membuffer[4096];
public:
    void *operator new(unsigned int size)
    {
        return (void*)buf;
    }
    void operator delete(void *p)
    {

    }
};

void memorypool()
{
    DWORD count=GetTickCount();
    for(int i=0; i<0x5fffff; i++)
    {
        shareddata *p = new shareddata;
        delete p;
    }
    cout<<"Time taken = "<<GetTickCount()-count <<" ms"<<endl;
}
struct Node
{
    void* memory;
    int size;
    Node* next;
};
void* allocate_memory(void* args)
{
    int size = *(int*)args;
    void* ptr = malloc(size);
    cout << "Allocated memory of size " << size << " bytes at location " << ptr << endl;
    return ptr;
}

class Buffer
{
public:
    Buffer(int size) : m_size(size), m_buffer(new char[size])
    {
        m_head = new Node{m_buffer, m_size, NULL};
    }

    ~Buffer()
    {
        Node* current = m_head;
        while (current)
        {
            Node* next = current->next;
            delete[] static_cast<char*>(current->memory);
            delete current;
            current = next;
        }
    }

    void* allocate(int size)
    {
        Node* current = m_head;
        while (current)
        {
            if (current->size >= size)
            {
                void* ptr = current->memory;
                if (current->size == size)
                {
                    if (current == m_head)
                    {
                        m_head = current->next;
                    }
                    else
                    {
                        Node* prev = m_head;
                        while (prev->next != current)
                        {
                            prev = prev->next;
                        }
                        prev->next = current->next;
                    }
                    delete current;
                }
                else
                {
                    current->memory = static_cast<char*>(current->memory) + size;
                    current->size -= size;
                }
                return ptr;
            }
            else
            {
                current = current->next;
            }
        }
        return NULL;
    }

    int remaining()
    {
        int remaining = 0;
        Node* current = m_head;
        while (current)
        {
            remaining += current->size;
            current = current->next;
        }
        return remaining;
    }
private:
    int m_size;
    char* m_buffer;
    Node* m_head;
};

int main()
{
    int i;
    pthread_t thread;
    cout << "Performance Test:" << endl;
    cout << "Without Memory Pool" << endl;
    nomemorypool();
    cout << "With Memory Pool" << endl;
    memorypool();
    cout<<"Hence memory pool is faster"<<endl;
    int unit=50;
    for(i=0;i<50;i++)
    {
        int BUFFER_SIZE1 = 64;
        int BUFFER_SIZE2 = 256;
        int BUFFER_SIZE3 = 512;
        Buffer smallbuffer(BUFFER_SIZE1);
        Buffer mediumbuffer(BUFFER_SIZE2);
        Buffer largebuffer(BUFFER_SIZE3);
        int allocation_size;
        while(allocation_size!=0 && smallbuffer.remaining()!=0 || mediumbuffer.remaining()!=0 || largebuffer.remaining()!=0)
        {
            int a;
            cout<<endl;
            cout << "\t\t\t\tNEW ALLOCATION"<<endl;
            cout << "----------------------------------------------------------------------------------------------------"<<endl;
            cout << "Enter the size of the memory to allocate: (enter 0 to exit): ";
            cin >> allocation_size;
            if(allocation_size!=0)
            {
                if(allocation_size<=largebuffer.remaining() || allocation_size<=mediumbuffer.remaining() || allocation_size<=smallbuffer.remaining())
                {
                    if(allocation_size<=64)
                    {
                        if(allocation_size<=smallbuffer.remaining())
                        {
                            pthread_create(&thread, NULL, allocate_memory, (void*)&allocation_size);
                            pthread_join(thread, NULL);
                            void* memory = smallbuffer.allocate(allocation_size);
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            cout << "Allocated memory in the smaller buffer " <<  endl;
                        }
                        else if(allocation_size<=mediumbuffer.remaining())
                        {
                            pthread_create(&thread, NULL, allocate_memory, (void*)&allocation_size);
                            pthread_join(thread, NULL);
                            void* memory = mediumbuffer.allocate(allocation_size);
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            cout << "Allocated memory in the medium buffer " <<  endl;
                        }
                        else if(allocation_size<=largebuffer.remaining())
                        {
                            pthread_create(&thread, NULL, allocate_memory, (void*)&allocation_size);
                            pthread_join(thread, NULL);
                            void* memory = largebuffer.allocate(allocation_size);
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            cout << "Allocated memory in the larger buffer " <<  endl;
                        }
                    }
                    else if(allocation_size > 64 && allocation_size<=256)
                    {
                        if(allocation_size<=mediumbuffer.remaining())
                        {
                            pthread_create(&thread, NULL, allocate_memory, (void*)&allocation_size);
                            pthread_join(thread, NULL);
                            void* memory = mediumbuffer.allocate(allocation_size);
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            cout << "Allocated memory in the medium buffer " <<  endl;
                        }
                        else if(allocation_size<=largebuffer.remaining())
                        {
                            pthread_create(&thread, NULL, allocate_memory, (void*)&allocation_size);
                            pthread_join(thread, NULL);
                            void* memory = largebuffer.allocate(allocation_size);
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            cout << "Allocated memory in the larger buffer " <<  endl;
                        }
                    }
                    else if(allocation_size > 256 && allocation_size<=512)
                    {
                        pthread_create(&thread, NULL, allocate_memory, (void*)&allocation_size);
                        pthread_join(thread, NULL);
                        void* memory = largebuffer.allocate(allocation_size);
                        cout << endl;
                        cout << "----------------------------------------------------------------------------------------------------"<<endl;
                        cout << "Allocated memory in the large buffer " <<  endl;
                    }
                }

                else if(allocation_size>largebuffer.remaining() && allocation_size>mediumbuffer.remaining() && allocation_size>smallbuffer.remaining())
                {
                    int z;
                    cout << endl;
                    cout << "----------------------------------------------------------------------------------------------------"<<endl;
                    cout<<"Memory compaction takes place"<<endl;
                    cout << endl;
                    int a,b,c;
                    a=allocation_size-smallbuffer.remaining();
                    z=smallbuffer.remaining();
                    pthread_create(&thread, NULL, allocate_memory, (void*)&z);
                    pthread_join(thread, NULL);
                    cout<<smallbuffer.remaining()<<" allocated from smallbuffer and "<<a<<" remaining to be allocated "<<endl;
                    void* memory = smallbuffer.allocate(smallbuffer.remaining());
                    if(a!=0)
                    {
                        int z1,z2,z3,z4;
                        if(a>mediumbuffer.remaining())
                        {
                            b=a-mediumbuffer.remaining();
                            z1=mediumbuffer.remaining();
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            //cout << endl;
                            pthread_create(&thread, NULL, allocate_memory, (void*)&z1);
                            pthread_join(thread, NULL);
                            cout<<mediumbuffer.remaining()<<" allocated from medium buffer and "<<b<<" remaining to be allocated "<<endl;
                            void* memory1 = mediumbuffer.allocate(mediumbuffer.remaining());
                            if(b!=0)
                            {
                                if(b>largebuffer.remaining())
                                {
                                    c=b-largebuffer.remaining();
                                    z2=largebuffer.remaining();
                                    cout << endl;
                                    cout << "----------------------------------------------------------------------------------------------------"<<endl;
                                    //cout << endl;
                                    pthread_create(&thread, NULL, allocate_memory, (void*)&z2);
                                    pthread_join(thread, NULL);
                                    cout<<largebuffer.remaining()<<" allocated from larger buffer "<<endl;
                                    void* memory2 = largebuffer.allocate(largebuffer.remaining());
                                    if(c!=0)
                                        cout<<c<<" cannot be allocated from this unit, borrow from next unit "<<endl;
                                }
                                else
                                {
                                    c=largebuffer.remaining()-b;
                                    z3=largebuffer.remaining();
                                    cout << endl;
                                    cout << "----------------------------------------------------------------------------------------------------"<<endl;
                                    //cout << endl;
                                    pthread_create(&thread, NULL, allocate_memory, (void*)&b);
                                    pthread_join(thread, NULL);
                                    cout<<b<<" allocated from large buffer "<<endl;
                                    void* memory2 = largebuffer.allocate(b);
                                }
                            }
                        }
                        else
                        {
                            b=mediumbuffer.remaining()-a;
                            z4=mediumbuffer.remaining();
                            cout << endl;
                            cout << "----------------------------------------------------------------------------------------------------"<<endl;
                            //cout << endl;
                            pthread_create(&thread, NULL, allocate_memory, (void*)&z4);
                            pthread_join(thread, NULL);
                            cout<<a<<" allocated from medium buffer "<<endl;
                            void* memory1 = mediumbuffer.allocate(a);
                        }
                    }
                }
                cout<<endl;
                cout << "----------------------------------------------------------------------------------------------------"<<endl;
                cout << "Remaining memory in small buffer: " << smallbuffer.remaining() <<  endl;
                cout << "Remaining memory in medium buffer: " << mediumbuffer.remaining() <<  endl;
                cout << "Remaining memory in large buffer: " << largebuffer.remaining() <<  endl;
                cout << "----------------------------------------------------------------------------------------------------"<<endl;
                cout << endl;
            }
            else
                exit(0);
        }
        unit--;
        cout<<unit<<" units of each buffer remaining"<<endl;
        cout << "----------------------------------------------------------------------------------------------------"<<endl;
    }
    return 0;
}
