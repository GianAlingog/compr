#include <bits/stdc++.h>
using namespace std;

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

string removeExtension(const string &filename) {
    filesystem::path p(filename);
    return (p.parent_path() / p.stem()).string();
}

string removeParent(const string &filename) {
    filesystem::path p(filename);
    return (p.stem()).string();
}

vector<unsigned char> readFile(string filename) {
    ifstream file(filename, ios::binary);
    assert(file.is_open());

    vector<unsigned char> file_content { istreambuf_iterator<char>(file), istreambuf_iterator<char>() };

    file.close();

    return file_content;
}

vector<huffman_node> huffmanify(map<unsigned char, unsigned int> &frequency_table) {
    // Build initial nodes of binary tree
    int node_count = 0;
    vector<huffman_node> nodes;

    auto cmp = [&](int i, int j) -> bool {
        return nodes[i] < nodes[j];
    };

    multiset<int, decltype(cmp)> queue(cmp);
    for (auto &[c, x] : frequency_table) {
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

    return nodes;
}

map<unsigned char, string> generateEncoder(vector<huffman_node> &nodes) {
    // Build the hash table (for now using strings)
    map<unsigned char, string> encoder;
    string current_hash;

    int node_count = nodes.size();
    int root = node_count - 1;

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

    dfs(dfs, root);

    return encoder;
}

void writeFile(map<unsigned char, unsigned int> &frequency_table, string &bit_string, string &in_file, string &out_file) {
    ofstream file(out_file, ios::binary);

    // Header
    const char ess[4] = {'E', 'S', 'S', '0'};
    file.write(ess, 4);

    // Original filename
    unsigned short name_len = static_cast<unsigned short>(in_file.size());
    file.write(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    file.write(in_file.data(), name_len);

    // Frequencies
    unsigned char byte = 0;
    short bit_count = 0;

    for (int c = 0; c < 256; c++) {
        unsigned char ch = (unsigned char)c;
        unsigned int freq = frequency_table[ch];
        file.write(reinterpret_cast<char*>(&freq), sizeof(freq));
    }

    // Unused bits
    int size = bit_string.size();
    int used = size % 8;
    if (used == 0) used += 8;
    int unused = 8 - used;
    file.put(unused);

    // Bitstream
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

void encode(string &in_file) {
    vector<unsigned char> file_content = readFile(in_file);

    // Build the frequency table
    map<unsigned char, unsigned int> frequency_table;
    for (auto &c : file_content) frequency_table[c]++;

    // Output the encoded string (this may not be so trivial)
    auto huffman_nodes = huffmanify(frequency_table);
    auto encoder = generateEncoder(huffman_nodes);
    string encoded_content;
    for (auto &c : file_content) encoded_content += encoder[c];

    // Write into a binary file
    string out_file = removeExtension(in_file) + ".ess";
    writeFile(frequency_table, encoded_content, in_file, out_file);

    cout << "Compressed text of " << filesystem::file_size(in_file) << " bits to " << filesystem::file_size(out_file) << " bits." << endl;
}

void decode(string &in_file) {
    ifstream file(in_file, ios::binary);
    assert(file.is_open());

    // Header
    char ess[4];
    file.read(ess, 4);
    if (strncmp(ess, "ESS0", 4) != 0) {
        throw runtime_error("Invalid archive format");
    }

    // Original filename
    unsigned short name_len;
    file.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
    string out_file(name_len, '\0');
    file.read(out_file.data(), name_len);

    // Frequencies
    map<unsigned char, unsigned int> frequency_table;
    for (int c = 0; c < 256; c++) {
        unsigned int freq;
        file.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        if (freq > 0) {
            frequency_table[static_cast<unsigned char>(c)] = freq;
        }
    }

    // Huffman tree
    auto nodes = huffmanify(frequency_table);
    int node_count = nodes.size();
    int root = node_count - 1;

    // Unused bits
    unsigned char unused;
    file.read(reinterpret_cast<char*>(&unused), 1);

    // File content
    vector<unsigned char> file_content { istreambuf_iterator<char>(file), istreambuf_iterator<char>() };
    
    file.close();

    // Decode
    vector<unsigned char> decoded;
    int curr = root;
    int total_bytes = static_cast<int>(file_content.size());
    for (int i = 0; i < total_bytes; i++) {
        unsigned char byte = file_content[i];
        int used_bits = 8;
        if (i == total_bytes - 1) {
            used_bits = 8 - unused;
        }

        for (int b = 7; b >= 8 - used_bits; b--) {
            int bit = (byte >> b) & 1;
            curr = (bit == 0) ? nodes[curr].left : nodes[curr].right;
            if (nodes[curr].set) {
                decoded.push_back(nodes[curr].c);
                curr = root;
            }
        }
    }

    // Write
    ofstream out(out_file, ios::binary);
    out.write(reinterpret_cast<char*>(decoded.data()), decoded.size());

    cout << "Decompressed " << out_file << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " -c|-d <filename>" << endl;
        return 1;
    }

    // Read file
    string flag = argv[1], in_file = argv[2]; // Possibly dangerous

    if (flag == "-c") {
        encode(in_file);
    } else if (flag == "-d") {
        decode(in_file);
    } else {
        cerr << "Unknown flag: " << flag << " (use -c to compress, -d to decompress)" << endl;
        return 1;
    }

    return 0;
}
