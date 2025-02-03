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

node* create_node(char c, int f) {
    node *n = new node;
    n->ch = c;
    n->freq = f;
    n->left = n->right = NULL;
    return n;
}
huffman_tree* create_huffman_tree(node* r, unordered_map<char, pair<unsigned int, int>> c) {
    huffman_tree* h = new huffman_tree;
    h->root = r;
    h->codes = c;
    return h;
}

void extract_data(const string& filename, priority_queue<node*, vector<node*>, compare>& min_heap)
 {
    ifstream file(filename, ios::binary);  
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        exit(1);
    }

    unordered_map<char, int> freq_map;
    char ch;

    while (file.get(ch))
        freq_map[ch]++; 

    file.close();

    for (const auto& pair : freq_map) 
        min_heap.push(create_node(pair.first, pair.second));
    
}


node* merge_nodes(node* n1, node* n2) {
    node* merged_node = create_node('\0', n1->freq + n2->freq);
    merged_node->left = n1;
    merged_node->right = n2;
    merged_node->is_internal = true;
    return merged_node;
}
node* build_huffman_tree(priority_queue<node*, vector<node*>, compare>& min_heap) {
    while (min_heap.size() > 1) {
        node* n1 = min_heap.top(); min_heap.pop();
        node* n2 = min_heap.top(); min_heap.pop();
        node* internal_node = merge_nodes(n1, n2);
        min_heap.push(internal_node);
    }
    return min_heap.top();  // The final node is the root of the Huffman tree
}
void compute_codes(node* root, unordered_map<char, pair<unsigned int, int>>& codes, unsigned int code, int length) {
    if (root == nullptr) return;

    if (!root->is_internal) {  // Leaf node
        codes[root->ch] = {code, length};
    }

    compute_codes(root->left, codes, code << 1, length + 1);
    compute_codes(root->right, codes, (code << 1) | 1, length + 1);
}

huffman_tree* get_huffman_tree(const string& filename) {
    priority_queue<node*, vector<node*>, compare> min_heap;
    unordered_map<char, pair<unsigned int, int>> codes_map;

    extract_data(filename, min_heap);
    node* root = build_huffman_tree(min_heap);
    compute_codes(root, codes_map, 0, 0);

    return create_huffman_tree(root, codes_map);
}
void free_tree(node* root) {
    if (root == nullptr) return;
    free_tree(root->left);
    free_tree(root->right);
    delete root;
}
void print_codes(const unordered_map<char, pair<unsigned int, int>>& codes) {
    for (const auto& pair : codes) {
        if (pair.first == ' ')
            cout << "SPACE" << " : ";
        else if (pair.first == '\n')
            cout << "NEWLINE" << " : ";
        else if (pair.first == '\f')
            cout << "'FORM FEED'" << " : ";  // ✅ Handles '\f'
        else
            cout << pair.first << " : ";

        for (int i = pair.second.second - 1; i >= 0; --i) {
            cout << ((pair.second.first >> i) & 1);
        }
        cout << endl;
    }
}

void compress(const string& input_filename, const string& output_filename, const unordered_map<char, pair<unsigned int, int>>& codes) {
    ifstream infile(input_filename);
    ofstream outfile(output_filename, ios::binary);

    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "Error opening files!" << endl;
        return;
    }

    unsigned char buffer = 0;  // Buffer to accumulate bits
    int bit_count = 0;         // Track number of bits in the buffer
    char ch;

    while (infile.get(ch)) 
    {
        auto it = codes.find(ch);
        if (it == codes.end()) 
        {
            if (isprint(ch)) 
                cerr << "Warning: No code found for character '" << ch << "'. Skipping...\n";

            else 
                cerr << "Warning: No code found for non-printable character (ASCII: " << static_cast<int>(ch) << "). Skipping...\n";

            cerr.flush();
            continue;
        }

        auto [code, length] = it->second;

        // Add the Huffman code to the buffer
        for (int i = length - 1; i >= 0; --i) {
            buffer = (buffer << 1) | ((code >> i) & 1);
            bit_count++;

            // Write full byte to output file when buffer is full
            if (bit_count == 8) {
                outfile.put(buffer);
                buffer = 0;
                bit_count = 0;
            }
        }
    }

    // Write remaining bits (if buffer is not empty)
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);  // Pad remaining bits with zeros
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

void compare_files(const char *file1, const char *file2) {
    FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");

    if (fp1 == NULL || fp2 == NULL) {
        perror("Error opening files");
        exit(EXIT_FAILURE);
    }

    int ch1, ch2;
    int position = 0;  // Position counter
    int line = 1;      // Line counter for text files

    int differences = 0;  // Track number of differences

    while (1) {
        ch1 = fgetc(fp1);
        ch2 = fgetc(fp2);
        position++;

        // Check for EOF in both files
        if (ch1 == EOF && ch2 == EOF) {
            break;  // Both files ended simultaneously
        }

        // Handle line number counting for text files
        if (ch1 == '\n' || ch2 == '\n') {
            line++;
        }

        // Compare characters
        if (ch1 != ch2) {
            differences++;
            printf("Difference at byte %d (line %d): 0x%02X vs 0x%02X\n", position, line, ch1, ch2);
        }
    }

    if (differences == 0) {
        printf("Original file and decompressed file are identical \n");
    } else {
        printf("Total differences found: %d\n", differences);
    }

    fclose(fp1);
    fclose(fp2);
}
long get_file_size(const string& filename) {
    ifstream file(filename, ios::binary | ios::ate);  // Open at the end of the file
    return file.tellg();  // Get the current position (file size)
}
double calculate_memory_saved(const string& original_file, const string& compressed_file) {
    long original_size = get_file_size(original_file);
    long compressed_size = get_file_size(compressed_file);

    if (original_size == 0) {
        cerr << "Error: Original file is empty or not found." << endl;
        return 0.0;
    }

    double saved_percentage = (1.0 - (double)compressed_size / original_size) * 100;
    return saved_percentage;
}

int main() {
    string input_file = "org.txt";
    string compressed_file = "compressed.bin";
    string decompressed_file = "decompressed.txt";

    huffman_tree* tree = get_huffman_tree(input_file);

    cout << "Huffman Codes:" << endl;
    print_codes(tree->codes);

    compress(input_file, compressed_file, tree->codes);
    decompress(compressed_file, decompressed_file, tree->root);

    free_tree(tree->root);
    delete tree;

    cout << "Compression and decompression completed successfully!" << endl;

    // Compare Files
    compare_files(input_file.c_str(), decompressed_file.c_str());

    // Calculate Memory Saved
    double saved_percentage = calculate_memory_saved(input_file, compressed_file);
    cout << "Memory Saved: " << saved_percentage << "%" << endl;

    return 0;
}
