#include <bits/stdc++.h>
using namespace std;

vector<unsigned char> readFile(string filename) {
    ifstream file(filename, ios::binary);
    assert(file.is_open());

    vector<unsigned char> file_content { istreambuf_iterator<char>(file), istreambuf_iterator<char>() };

    file.close();

    return file_content;
}

map<unsigned char, string> huffmanEncode(map<unsigned char, unsigned int> &frequency_map) {
    // Build initial nodes of binary tree
    struct huffman_node {
        bool set;
        unsigned int w;
        unsigned char c;
        int left, right;
        huffman_node(unsigned char _c, unsigned int _w): 
            set(true), c(_c), w(_w), left(-1), right(-1)
        {;;}
        huffman_node(int _left, int _right, unsigned int _w):
            set(false), c('\0'), w(_w), left(_left), right(_right)
        {;;}
        bool operator<(const huffman_node &oth) const {
            if (w == oth.w) return c < oth.c;
            return w < oth.w;
        }
    };

    int node_count = 0;
    vector<huffman_node> nodes;

    auto cmp = [&](int i, int j) -> bool {
        return nodes[i] < nodes[j];
    };

    multiset<int, decltype(cmp)> queue(cmp);
    for (auto &[c, x] : frequency_map) {
        nodes.push_back(huffman_node(c, x));
        queue.insert(node_count++);
    }


    // Do not allow single-character inputs for now
    assert(queue.size() >= 2);


    // Build binary tree with Huffman encoding algorithm
    while (int(queue.size()) >= 2) {
        auto right = *queue.begin();
        queue.erase(queue.begin());

        auto left = *queue.begin();
        queue.erase(queue.begin());

        nodes.push_back(huffman_node(left, right, nodes[left].w + nodes[right].w));
        queue.insert(node_count++);
    }


    // Build the hash table (for now using strings)
    map<unsigned char, string> encoder;
    string current_hash;

    auto dfs = [&](auto &&dfs, int curr) -> void {
        if (!nodes[curr].set) {
            assert(nodes[curr].left != -1 and nodes[curr].right != -1);
            // Traverse left child
            current_hash.push_back('0');
            dfs(dfs, nodes[curr].left);
            current_hash.pop_back();

            // Traverse right child
            current_hash.push_back('1');
            dfs(dfs, nodes[curr].right);
            current_hash.pop_back();
        } else {
            encoder[nodes[curr].c] = current_hash;
        }
    };

    dfs(dfs, node_count - 1);

    return encoder;
}

void writeFile(map<unsigned char, unsigned int> &frequency_table, string &bit_string, string filename) {
    ofstream file(filename, ios::binary);
    unsigned char byte = 0;
    short bit_count = 0;

    for (int c = 0; c < 256; c++) {
        unsigned char ch = (unsigned char)c;
        unsigned int freq = frequency_table[ch];
        file.write(reinterpret_cast<char*>(&freq), sizeof(freq));
    }

    // Dedicate the first byte to how many bits are used in the last byte
    int size = bit_string.size();
    int used = size % 8;
    if (used == 0) used += 8;
    int unused = 8 - used;
    // file.put(used);
    file.put(unused);

    for (auto &bit : bit_string) {
        byte <<= 1;
        if (bit == '1') byte |= 1;
        bit_count++;

        if (bit_count == 8) {
            file.put(byte);
            byte = 0;
            bit_count = 0;
        }
    }

    if (bit_count > 0) {
        byte <<= (unused);
        file.put(byte);
    }

    file.close();

    return;
}

int main(int argc, char* argv[]) {
    assert(1 <= argc and argc <= 2);

    // Read file
    string in_file = "input.txt";
    if (argc > 1) in_file = argv[1]; // Possibly dangerous
    vector<unsigned char> file_content = readFile(in_file);


    // Build the frequency map
    map<unsigned char, unsigned int> frequency_map;
    for (auto &c : file_content) frequency_map[c]++;


    // Output the encoded string (this may not be so trivial)
    auto encoder = huffmanEncode(frequency_map);
    string encoded_content;
    for (auto &c : file_content) encoded_content += encoder[c];
    // cout << encoded_content << endl;

    // Write into a binary file
    string out_file = "output.ess";
    writeFile(frequency_map, encoded_content, out_file);

    cout << "Compressed text of " << filesystem::file_size(in_file) << " bits to " << filesystem::file_size(out_file) << " bits." << endl;
    return 0;
}
