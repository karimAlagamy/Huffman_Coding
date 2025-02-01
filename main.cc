#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <vector>

using namespace std;

struct node {
    string ch;
    int freq;
    node *left;
    node *right;
};

struct compare {
    bool operator()(node* n1, node* n2) {
        return n1->freq > n2->freq;
    }
};

node* create_node(string c, int f) {
    node *n = new node;
    n->ch = c;
    n->freq = f;
    n->left = n->right = NULL;
    return n;
}

// Extract data from file and build frequency map and min-heap
void extract_data(const string& filename, priority_queue<node*, vector<node*>, compare>& min_heap) {

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        exit(1);
    }

    unordered_map<char, int> freq_map;
    char ch;

    while (file.get(ch)) {
        if(isgraph(ch))
            freq_map[ch]++;
    }
    file.close();

    for (const auto& pair : freq_map) {
        min_heap.push(create_node(string(1, pair.first), pair.second));
    }
}

node* merge_nodes(node* n1, node* n2) {
    node* merged_node = create_node("INTERNAL", n1->freq + n2->freq);
    merged_node->left = n1;
    merged_node->right = n2;
    return merged_node;
}
node* huffman_coding(priority_queue<node*, vector<node*>, compare>& min_heap) {
    
    node* internal_node = nullptr;

    while (min_heap.size() > 1) {
        node* n1 = min_heap.top();
        min_heap.pop();

        node* n2 = min_heap.top();
        min_heap.pop();

        internal_node = merge_nodes(n1, n2);
        min_heap.push(internal_node);
    }

    // The final node is the root of the Huffman tree
    return min_heap.top();
}
void get_codes(node* root, unordered_map<char, string>& codes, string code ) {

    if (root == nullptr) return;

    if (root->ch.size() == 1) {
        codes[root->ch[0]] = code;  
    }

    get_codes(root->left, codes, code + "0");   
    get_codes(root->right, codes, code + "1");  
}

void print_node(node* n) {
    if (n->ch == "INTERNAL") {
        cout << "  " << n->freq << endl;
    }
    else
        cout << n->ch << " " << n->freq << endl;
}
void print_tree(node* root) {
    if (root == nullptr) return;
    print_tree(root->left);
    print_node(root);
    print_tree(root->right);
}
void print_codes(const unordered_map<char, string>& codes) {
    for (const auto& pair : codes) {
        cout << pair.first << " " << pair.second << endl;
    }
}
void free_tree(node* root) {
    if (root == nullptr) return;
    free_tree(root->left);
    free_tree(root->right);
    delete root;
}

int main() {
    
    priority_queue<node*, vector<node*>, compare> min_heap;
    unordered_map<char, string> codes_map;

    extract_data("test.txt", min_heap);

    node* root = huffman_coding(min_heap);
    // print_tree(root);

    get_codes(root, codes_map, "");
    print_codes(codes_map);

    free_tree(root);  // Free allocated memory

    return 0;
}