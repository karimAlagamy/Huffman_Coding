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

    bool is_internal = false;
};

struct huffman_tree {
    node* root;
    unordered_map<char, pair<unsigned int, int>> codes;
};

struct compare {
    bool operator()(node* n1, node* n2) {
        return n1->freq > n2->freq;
    }
};

node* create_node(char c, int f)
 {
    node *n = new node;
    n->ch = c;
    n->freq = f;
    n->left = n->right = NULL;
    return n;
}
huffman_tree* create_huffman_tree(node* r, unordered_map<char, pair<unsigned int, int>> c)
 {
    huffman_tree* h = new huffman_tree;
    h->root = r;
    h->codes = c;
    return h;
}

void extract_data(const string& filename, priority_queue<node*, vector<node*>, compare>& min_heap) 
{

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
        min_heap.push(create_node(pair.first, pair.second)); 
    }}

node* merge_nodes(node* n1, node* n2)
{
    node* merged_node = create_node('\0',  n1->freq + n2->freq);
    merged_node->left = n1;
    merged_node->right = n2;
    merged_node->is_internal = true;
    return merged_node;
    }
node* build_huffman_tree(priority_queue<node*, vector<node*>, compare>& min_heap) {
    
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
void compute_codes(node* root, unordered_map<char, pair<unsigned int, int>>& codes, unsigned int code, int length) {
    if (root == nullptr) return;

    if (!root->is_internal) {  // Leaf node
        codes[root->ch] = {code, length};  // Store code with its bit length
    }

    // Move left: add '0' (no need to modify the code, just shift)
    compute_codes(root->left, codes, code << 1, length + 1);

    // Move right: add '1' (shift and set the least significant bit)
    compute_codes(root->right, codes, (code << 1) | 1, length + 1);
}

huffman_tree* get_huffman_tree(const string& filename) {

    priority_queue<node*, vector<node*>, compare> min_heap;
    unordered_map<char, pair<unsigned int, int>> codes_map;
    node* root = nullptr;

    extract_data(filename, min_heap);
    root = build_huffman_tree(min_heap);
    compute_codes(root, codes_map, 0, 0);

    huffman_tree* huffman_tree = create_huffman_tree(root, codes_map);

    return huffman_tree;
}
void free_tree(node* root) {
    if (root == nullptr) return;
    free_tree(root->left);
    free_tree(root->right);
    delete root;
}

void print_node(node* n) {
    if (n->is_internal == true)
        cout << "  " << n->freq << endl;

    else
        cout << n->ch << " " << n->freq << endl;
}
void print_tree(node* root) {
    if (root == nullptr) return;
    print_tree(root->left);
    print_node(root);
    print_tree(root->right);
}
void print_codes(const unordered_map<char, pair<unsigned int, int>>& codes) {
    for (const auto& pair : codes) {
        cout << pair.first << " : ";
        for (int i = pair.second.second - 1; i >= 0; --i) {
            cout << ((pair.second.first >> i) & 1);
        }
        cout << endl;
    }
}

void compress(const string& input_filename, const string& output_filename, const unordered_map<char, pair<unsigned int, int>>& codes) {
    ifstream infile(input_filename, ios::binary);
    ofstream outfile(output_filename, ios::binary);

    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "Error opening files!" << endl;
        return;
    }

    unsigned char buffer = 0;   // 8-bit buffer to accumulate bits
    int bit_count = 0;          // Number of bits currently in the buffer
    char ch;

    while (infile.get(ch)) {
        if (isgraph(ch)) {  // Encode only printable characters
            auto [code, length] = codes.at(ch);  // Get Huffman code and its length

            for (int i = length - 1; i >= 0; --i) {
                buffer = (buffer << 1) | ((code >> i) & 1);  // Shift buffer and add the next bit
                bit_count++;

                if (bit_count == 8) {  // If buffer is full, write it to the output file
                    outfile.put(buffer);
                    buffer = 0;
                    bit_count = 0;
                }
            }
        }
    }

    // Write remaining bits (if any) with padding
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);  // Shift to fill remaining bits with zeros
        outfile.put(buffer);
    }

    infile.close();
    outfile.close();
}
void decompress(const string& input_filename, const string& output_filename, node* root) {
    ifstream infile(input_filename, ios::binary);
    ofstream outfile(output_filename);

    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "Error opening files!" << endl;
        return;
    }

    unsigned char buffer;
    node* current = root;

    while (infile.read(reinterpret_cast<char*>(&buffer), 1)) {  // Read byte-by-byte
        for (int i = 7; i >= 0; --i) {  // Process bits from MSB to LSB
            int bit = (buffer >> i) & 1;

            // Traverse the Huffman tree
            if (bit == 0)
                current = current->left;
            else
                current = current->right;

            // If it's a leaf node, output the character
            if (current && !current->is_internal) {
                outfile.put(current->ch);
                current = root;  // Reset to the root for the next character
            }
        }
    }

    infile.close();
    outfile.close();
}


int main() {
    
    string input_file = "org.txt";
    string compressed_file = "compressed.bin";
    string decompressed_file = "decompressed.txt";

    huffman_tree* huffman_tree = get_huffman_tree(input_file);
    compress(input_file, compressed_file, huffman_tree->codes);
    decompress(compressed_file, decompressed_file, huffman_tree->root);

    free_tree(huffman_tree->root);  // Free allocated memory

    return 0;
}