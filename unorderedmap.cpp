#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

//The hash function is written based on Pseudocode on canvas page
unsigned int hashFunction(char const* key, int table_size)
{
    unsigned int hash_code = 0;

    for (int i = 0; key[i] != '\0'; i++)
    {
        unsigned int b = 0;

        if ((i & 1) != 1) // even
            b = (hash_code << 7) ^ key[i] ^ (hash_code >> 3);
        else if ((i & 1) == 1) // odd
        {
            b = (hash_code << 11) ^ key[i] ^ (hash_code >> 5);
            b = ~b;
        }
        hash_code = hash_code ^ b;
    }

    hash_code &= 0x7FFFFFFF;
    return hash_code % static_cast<unsigned int>(table_size);
}

//The node struct contains key, value, and a pointer to next pointer in the chaining
struct ListNode
{
    string key = "", val = "";
    ListNode* next = nullptr;
};

class UnorderedMap
{
private:
    //define your data structure here
    //define other attributes e.g. bucket count, maximum load factor, size of table, etc.
    unsigned int size_;
    double curr_load_factor;
    unsigned int capacity;
    double max_load_factor;
    vector<ListNode*> buckets;

public:
    class Iterator;
    UnorderedMap(unsigned int bucketCount, double loadFactor);
    Iterator begin() const;
    Iterator end() const;
    string& operator[](string const& key);
    void rehash();
    void remove(string const& key);
    unsigned int size();
    double loadFactor();

    int tableSize() const;

    class Iterator
    {
        //make Unordered Map a friend class of iterator so that iterator can access data members in the unordered_map object
        friend class UnorderedMap;
    public:
        //this constructor does not need to be a default constructor;
        //the parameters for this constructor are up to your discretion.
        //hint: you may need to pass in an UnorderedMap object.
        const UnorderedMap* map;
        ListNode* pointer;

        //passed in a pointer to a map object and the node in the chain
        Iterator(const UnorderedMap* m, ListNode* node)
        {
            map = m;
            pointer = node;
        }

        //Overload the "=" with functions in map
        //deep copy the value from right side to left side
        //return a reference to the iterator object
        Iterator& operator=(Iterator const& rhs)
        {
            pointer->key = rhs.pointer->key;
            pointer->val = rhs.pointer->val;
            pointer->next = rhs.pointer->next;
            return *this;
        }

        //This function moves iterator one node forward everytime
        Iterator& operator++()
        {
            //when in the linked list chaining
            //move forward in the linked list
            if (pointer->next != nullptr)
            {
                pointer = pointer->next;
                return *this;
            }

            //when not in the linked list chain
            //first find out which bucket the key value pair belongs to
            //assign pointer to the first list node in the linked list
            //now the iterator is in the linked list again
            else {
                int addr = static_cast<int>(hashFunction(pointer->key.c_str(), map->tableSize()));
                for (int i = addr + 1; i < map->tableSize(); i++) {
                    if (map->buckets[i]) {
                        pointer = map->buckets[i];
                        return *this;
                    }
                }
                pointer = nullptr;
                return *this;
            }
        }

        //return a boolean that compares left side pointer and right side pointer
        bool operator!=(Iterator const& rhs)
        {
            return pointer != rhs.pointer;
        }

        //return a boolean that compares left side pointer and right side pointer
        bool operator==(Iterator const& rhs)
        {
            return pointer == rhs.pointer;
        }

        //return a key value pair the iterator holds
        pair<string, string> operator*() const
        {
            return make_pair(pointer->key, pointer->val);
        }


    };
};

//initialize an unordered map object
UnorderedMap::UnorderedMap(unsigned int bucketCount, double loadFactor)
{
    size_ = 0;
    curr_load_factor = 0.0;
    capacity = bucketCount;
    max_load_factor = loadFactor;
    //initialize the vector to the size of bucketCount
    buckets.resize(bucketCount);
    //the buckets are initialized to begin with null pointer
    for (unsigned int i = 0; i < bucketCount; i++)
    {
        buckets[i] = nullptr;
    }
}

//This function find the first valid node in all buckets
//Or it will return an iterator whose pointer is a null pointer
UnorderedMap::Iterator UnorderedMap::begin() const
{
    for (int i = 0; i < buckets.size(); i++)
        if (buckets[i] != nullptr)
            return Iterator(this, buckets[i]);

    return Iterator(this, nullptr);
}

//This function returns an iterator object with a null pointer
UnorderedMap::Iterator UnorderedMap::end() const
{
    return Iterator(this, nullptr);
}


string& UnorderedMap::operator[](string const& key)
{
    int index = static_cast<int>(hashFunction(key.c_str(), tableSize()));
    ListNode* temp = buckets[index];
    bool exist = false;
    //This while loop checks whether the key is in the unordered map
    while (temp != nullptr)
    {
        if (temp->key == key)
        {
            exist = true;
            break;
        }
        temp = temp->next;
    }

    //if the key exists in the unordered map
    //return the corresponding value
    if (exist)
        return temp->val;
    //If the key does not exist in the unordered map
    //Creates a new list node in corresponding bucket
    else{
        temp = new ListNode();
        temp->key = key;
        temp->val = "";
        temp->next = buckets[index];
        buckets[index] = temp;
        //If current load factor exceeds the max load factor
        //We need to Rehash the table
        size_ ++;
        curr_load_factor = double(size_) / double(buckets.size());
        if (curr_load_factor >= max_load_factor)
            rehash();
        return temp->val;
    }
}

//First we need to double the capacity and create a new vector of buckets
void UnorderedMap::rehash()
{
    capacity *= 2;
    vector<ListNode*> newBuckets(capacity);
    //for every node in the buckets, assign them a new index using the doubled capacity
    //build new hash table from the old hash table
    //insert each node to the beginning of the the linked list chain
    for (int i = 0; i < buckets.size(); i++)
    {
        ListNode* ptr = buckets[i];
        while (ptr != nullptr)
        {
            unsigned int index = hashFunction(ptr->key.c_str(), static_cast<int>(capacity));
            buckets[i] = ptr->next;
            ptr->next = newBuckets[index];
            newBuckets[index] = ptr;
            ptr = buckets[i];
        }
    }
    //after transferring the values, swap the two buckets
    buckets.swap(newBuckets);
}

void UnorderedMap::remove(string const& key)
{
    int index = static_cast<int>(hashFunction(key.c_str(), tableSize()));
//If the key does not exist in the unordered map
//don't do anything
    if (buckets[index] == nullptr)
        return;

//If the key is the beginning of a linked list chain
//the beginning of the linked list will be the next node in the chain
    ListNode* temp = buckets[index];
    if (temp->key == key)
    {
        size_ -= 1;
        buckets[index] = buckets[index]->next;
        return;
    } else {
        //If the key is not the beginning of the linked list chain
        //Find the key value pair when traversing the linked list
        //Delete the node and reconnects the linked list
        ListNode* prev = buckets[index];
        temp = temp->next;
        while (temp->next != nullptr)
        {
            if (temp->key == key)
            {
                size_ -= 1;
                prev->next = temp->next;
                break;
            }
            prev = prev->next;
            temp = temp->next;
        }
    }
}

unsigned int UnorderedMap::size()
{
    return size_;
}

int UnorderedMap::tableSize() const
{
    return static_cast<int>(buckets.size());
}

//calculate load factor for the hash table
double UnorderedMap::loadFactor()
{
    curr_load_factor = double(size_)/buckets.size();
    return curr_load_factor;
}


//implement other operators in Iterator class
//Do not change main()
int main()
{
    int lines = 0, buckets = 0;
    double maxLoadFactor = 0.0;
    std::string command = "", ufid = "", name = "", key = "";
    std::cin >> lines >> buckets >> maxLoadFactor;
    UnorderedMap myMap = UnorderedMap(buckets, maxLoadFactor);
    while (lines--)
    {
        std::cin >> command;
        if (command == "hash")
        {
            std::cin >> key;
            const char* convertedKey = key.c_str();
            std::cout << hashFunction(convertedKey, buckets) << "\n";
        }
        else if (command == "insert")
        {
            std::cin >> ufid >> name;
            myMap[ufid] = name;
        }
        else if (command == "size")
        {
            std::cout << myMap.size() << "\n";
        }
        else if (command == "load")
        {
            std::cout << std::fixed << std::setprecision(2) << myMap.loadFactor() << "\n";
        }
        else if (command == "search")
        {
            std::cin >> ufid;
            std::cout << myMap[ufid] << "\n";
        }
        else if (command == "traverse")
        {
            for (UnorderedMap::Iterator iter = myMap.begin(); iter != myMap.end(); ++iter)
            {
                std::cout << (*iter).first << " " << (*iter).second << "\n";
            }
            /* This should also work
            for (auto tableEntry: myMap)
            {
                std::cout << tableEntry.first << " " << tableEntry.second << "\n";
            }
            */
        }
        else if (command == "remove")
        {
            std::cin >> ufid;
            myMap.remove(ufid);
        }
    }
    return 0;
}
