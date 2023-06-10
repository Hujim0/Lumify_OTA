/*
Made by zekageri

Licence: MIT
*/
#ifndef Linked_h
#define Linked_h

template <typename T>
class ESP_LinkedList
{
private:
    struct Node
    {
        T data;
        Node *next;
        Node *prev;
    };
    Node *head;
    Node *tail;
    int count;

public:
    // constructor to initialize head, tail, and count to default values.
    ESP_LinkedList()
    {
        head = NULL;
        tail = NULL;
        count = 0;
    }
    // destructor to clear the list when the object goes out of scope
    ~ESP_LinkedList()
    {
        clear();
    }
    // adds an element to the end of the list
    void add(T data)
    {
        Node *newNode = new Node();
        newNode->data = data;
        newNode->next = NULL;
        newNode->prev = tail;
        if (head == NULL)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }
    // adds an element at a specific index in the list
    void add(int index, T data)
    {
        if (index < 0 || index > count)
        {
            return;
        }

        Node *newNode = new Node();
        newNode->data = data;

        if (index == 0)
        {
            newNode->prev = NULL;
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        else if (index == count)
        {
            newNode->prev = tail;
            newNode->next = NULL;
            tail->next = newNode;
            tail = newNode;
        }
        else
        {
            Node *current = head;
            for (int i = 0; i < index - 1; i++)
            {
                current = current->next;
            }
            newNode->prev = current;
            newNode->next = current->next;
            current->next->prev = newNode;
            current->next = newNode;
        }
        count++;
    }
    // removes an element at a specific index in the list
    void remove(int index)
    {
        if (index < 0 || index >= count)
        {
            return;
        }
        Node *current = head;
        if (index == 0)
        {
            head = head->next;
            head->prev = NULL;
            delete current;
        }
        else if (index == count - 1)
        {
            current = tail;
            tail = tail->prev;
            tail->next = NULL;
            delete current;
        }
        else
        {
            for (int i = 0; i < index; i++)
            {
                current = current->next;
            }
            current->prev->next = current->next;
            current->next->prev = current->prev;
            delete current;
        }
        count--;
    }
    // clears all elements in the list
    void clear()
    {
        Node *current = head;
        while (current != NULL)
        {
            Node *temp = current;
            current = current->next;
            delete temp;
        }
        head = NULL;
        tail = NULL;
        count = 0;
    }
    // gets the size of the list

    int size()
    {
        return count;
    }

    // gets the element at a specific index in the list
    T get(int index)
    {
        if (index < 0 || index >= count)
        {
            return T();
        }
        Node *current = head;
        for (int i = 0; i < index; i++)
        {
            current = current->next;
        }
        return current->data;
    }

    // operator overload for accessing elements using the array operator, e.g. myList[0]
    T operator[](int index)
    {
        return get(index);
    }

    // checks if the list contains a specific element
    bool contains(T data)
    {
        Node *current = head;
        while (current != NULL)
        {
            if (current->data == data)
            {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    // finds the index of a specific element in the list
    int indexOf(T data)
    {
        Node *current = head;
        int index = 0;
        for (; current != NULL; current = current->next, ++index)
        {
            if (current->data == data)
            {
                return index;
            }
        }
        return -1;
    }

    // reverses the order of the elements in the list
    void reverse()
    {
        if (head == NULL || head->next == NULL)
        {
            return;
        }
        Node *current = head;
        Node *prev = NULL;
        Node *next = NULL;
        while (current != NULL)
        {
            next = current->next;
            current->next = prev;
            current->prev = next;
            prev = current;
            current = next;
        }
        tail = head;
        head = prev;
    }

    /*
        This implementation uses the Bubble sort algorithm to sort the elements in the list in
            ascending order based on the comparison operator '>'.
        It checks if the current node's data is greater than the next node's data.
        If so, it swap the data between current and next.
        This process is repeated until all elements in the list are sorted.
        If the list is already sorted, the loop will break and the method will return.
    */
    void sort()
    {
        if (head == NULL || head->next == NULL)
        {
            return;
        }
        // use any sorting algorithm
        // for example using bubble sort:
        Node *current = head;
        Node *prev = NULL;
        Node *next = NULL;
        bool swapped;
        do
        {
            swapped = false;
            current = head;
            while (current->next != prev)
            {
                if (current->data > current->next->data)
                {
                    T temp = current->data;
                    current->data = current->next->data;
                    current->next->data = temp;
                    swapped = true;
                }
                current = current->next;
            }
            prev = current;
        } while (swapped);
    }

    // adds an element to the end of the list (alias for add)
    void push_back(T data)
    {
        add(data);
    }

    // removes the last element from the list and returns it.
    T pop_back()
    {
        if (count == 0)
        {
            return T();
        }
        T removedData = get(count - 1);
        remove(count - 1);
        return removedData;
    }

    // adds an element to the front of the list
    void push_front(T data)
    {
        add(0, data);
    }

    // removes the first element from the list and returns it.
    T pop_front()
    {
        if (count == 0)
        {
            return T();
        }
        T removedData = get(0);
        remove(0);
        return removedData;
    }
};

#endif