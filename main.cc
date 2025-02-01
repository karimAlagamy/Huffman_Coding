#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>

using namespace std;

struct node {
    char ch;
    int freq;
    node *left;
    node *right;
};

struct compare {
    bool operator()(node* n1, node* n2) {
        return n1->freq > n2->freq;
    }
};

node* create_node(char c, int f) {
    node *n = new node;
    n->ch = c;
    n->freq = f;
    n->left = n->right = NULL;
    return n;
}

void extract_data(const string& filename,
                  priority_queue<node*, vector<node*>, compare>& min_heap,
                  unordered_map<char, int>& freqMap) {

    // Open the file
    ifstream file("test.txt");
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        exit(1);
    }

    char ch;

    // Read characters from the file one by one
    while (file.get(ch)) {
        freqMap[ch]++;   
    }

    // Close the file
    file.close();

    for (const auto& pair : freqMap) 
        min_heap.push(create_node(pair.first, pair.second));
    
}



void print_node(node *n) {
    cout << n->ch << " : " << n->freq << endl;
}
void print_freq_map(unordered_map<char, int> freqMap) {
    for (const auto& pair : freqMap) {
        cout << pair.first << " : " << pair.second << endl;
    }
}
void print_priority_queue(priority_queue<node*, vector<node*>, compare> min_heap) {
    while (!min_heap.empty()) {
        print_node(min_heap.top());
        min_heap.pop();
    }
}

int main() {
  
    // Init the priority queue and frequency map
    priority_queue<node*, vector<node*>, compare> min_heap;
    unordered_map<char, int> freqMap;

    // Extract data from the file into the priority queue and frequency map
    extract_data("test.txt", min_heap, freqMap);



    return 0;
}

