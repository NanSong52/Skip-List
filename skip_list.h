#include <iostream>
#include <cstdlib> // for rand() and srand()
#include <cmath>
#include <cstring>
#include <mutex>    // import mutex
#include <fstream>  // import file stream

#define STORE_FILE "store/dump_file" 
std::mutex mtx; // declare a mutex
std::string delimiter = ":";

// declared a template with two parameters: Key and value that node will store
template <typename K, typename V>
class Node {
public:
    // Constructor and destructor 
    Node() {}
    Node(K k, V v, int); // a level 'int' indicates the height of the node
    ~Node(); 

    // Member functions
    K get_key() const; 
    V get_value() const;
    void set_value(V); // Sets the node's value to the provided value

    // Member variables
    // forward:  an array of pointers to the next node (at a corresponding level) in the list
    // node_level: the size of the forward array
    Node<K, V> **forward; 
    int node_level; 
private:
    K key;
    V value;
};




// Parameterized Constructor: Initializes the node with a key, value, and level. 
// Allocates memory for the forward array and sets all pointers to nullptr (using memset).
template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    // level + 1 because the level is 0-based
    this->forward = new Node<K, V> *[level + 1];
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
};

// Deconstructor: Cleans up and prevent memory leaks when a Node object is destroyed.
// dynamically allocated memory can lwad to memory leaks.
template <typename K, typename V>
Node<K, V>::~Node() {
    delete[] forward;
};

// Getter and Setter functions: the key and value of the node.
template <typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
};

template <typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
};

template <typename K, typename V>
void Node<K, V>::set_value(V value) {
    this->value = value;
};

// Class: Skip list
template <typename K, typename V>
class SkipList {
public:
    SkipList(int); // Constructor
    ~SkipList(); // Destructor
    int get_random_level(); // Generates a random level for a node
    Node<K, V> *create_node(K, V, int); // Creates a new node with the provided key, value, and level
    int insert_element(K, V); // Inserts a new element
    bool search_element(K); // Searches for an element
    void delete_element(K); // Deletes an element
    void display_list(); // Displays the skip list
    void dump_file();
    void load_file();
    void clear(Node<K, V> *);
    int size();

private:
    void get_key_value_from_string(const std::string &str, std::string *key, std::string *value);
    bool is_valid_string(const std::string &str);

private:
    int _max_level; // Maximum level of the skip list
    int _skip_list_level; // Current level of the skip list
    Node<K, V> *_header; // Header node of the skip list
    std::ofstream _file_writer; // File writer
    std::ifstream _file_reader; // File reader
    int _element_count; // Number of nodes
};

template <typename K, typename V>
Node<K, V> *SkipList<K, V>::create_node(const K k, const V v, int level) {
    // Create a new node with the provided key, value, and level
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n; // return the new node
}



//insert
//1. level 2.insert 3. update (forward previous node and post node)
template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));
    for (int i = _skip_list_level; i >= 0; i--) {
        // search for the node which is the nearest and smaller than the key of new node in current level i
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        // store the node found in a update array
        update[i] = current; // previous node
    }
    // move to the the next node of lowest level
    // reaches the level 0 and forward pointer to the right node which is next node to new node

    // if the key already exists equal to searched key, return 1
    current = current->forward[0];
    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1; // key already exists
    }

    // if the key does not exist, create a new node
    if (current == NULL || current->get_key() != key) {
        // generate a random level for the new node
        int random_level = get_random_level();
        // if the random level is greater than the skip list's level, update the skip list's level
        if (random_level > _skip_list_level) {
            // Update the header node for the new level
            for (int i = _skip_list_level + 1; i < random_level + 1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // create a new node and update the forward and backward pointers
        Node<K, V> *inserted_node = create_node(key, value, random_level);

        // insert new node
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        _element_count++; // increment the element count
    }
    mtx.unlock();
    return 0;
}

// search
template <typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
    std::cout << "search_element-----------------" << std::endl;
    // pointer (current) to the header node
    Node<K, V> *current = _header;
    // iterate from the highest level of the skip list
    for (int i = _skip_list_level; i >= 0; i--) //vertically
    {
        // iterate through the current level until the next node's key is greater and equal than the provided key
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            // move to the next node (horizontally)
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}


// Delete element from skip list 
template <typename K, typename V>
void SkipList<K, V>::delete_element(K key) {
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));
    
    // from the highest level to the lowest level to search
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current; // record previous node
    }

    current = current->forward[0];
    // if the key is found, delete the node
    if (current != NULL && current->get_key() == key) {
        // iterate through all the levels and update the forward pointers
        for (int i = 0; i <= _skip_list_level; i++) {
            if (update[i]->forward[i] != current) // if the forward node is not the current node
                break;
            update[i]->forward[i] = current->forward[i]; // else update the forward pointer
        }
        // remove levels having no elements
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level--;
        }
        
        std::cout << "Successfully deleted key! "<< key << std::endl;
        delete current;
        _element_count--;
    }
    mtx.unlock();
    return;
}



template<typename K, typename V>
int SkipList<K, V>::get_random_level(){
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};


// Display the skip list
template <typename K, typename V>
void SkipList<K, V>::display_list() {
    // iterate through all the levels of the skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        Node<K, V>* node = this->_header->forward[i]; // get the first node of the current level
        std::cout << "Level " << i << ": ";
        // iterate through all the nodes of the current level
        while (node != nullptr) {
            // print the key and value of the current node
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            // move to the next node
            node = node->forward[i];
        }
        std::cout << std::endl; // end of the current level
    }
}

// Dump data in memory to file
template <typename K, typename V>
void SkipList<K, V>::dump_file() {
    _file_writer.open(STORE_FILE); // open the file in write mode
    Node<K, V>* node = this->_header->forward[0]; // get the first node of the skip list

    while (node != nullptr) {
        // write the key and value to the file
        _file_writer << node->get_key() << ":" << node->get_value() << ";\n"; 
        node = node->forward[0]; // move to the next node
    }

    _file_writer.flush(); // flush the output stream
    _file_writer.close(); // close the file
}


template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}


template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string *key, std::string *value) {
    if (!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

// Get current SkipList size
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}


// Load data from disk
template <typename K, typename V>
void SkipList<K, V>::load_file() {
    _file_reader.open(STORE_FILE);
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();

    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        // Define key as int type
        insert_element(stoi(*key), *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }

    delete key;
    delete value;
    _file_reader.close();
}


// constructor: skip list
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

// destructor: skip list
template<typename K, typename V> 
SkipList<K, V>::~SkipList() {

    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }

    // clear the skip list using recursion
    if(_header->forward[0]!=nullptr){
        clear(_header->forward[0]);
    }
    delete(_header);
    
}
template <typename K, typename V>
void SkipList<K, V>::clear(Node<K, V> * cur)
{
    if(cur->forward[0]!=nullptr){
        clear(cur->forward[0]);
    }
    delete(cur);
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};