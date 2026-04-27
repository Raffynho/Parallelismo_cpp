#include "huffman.h"
#include <omp.h> // LIBRERIA OPENMP AGGIUNTA

struct Node {
    char ch;
    int freq;
    Node *left, *right;
    Node(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq; 
    }
};

void generateDictionary(Node* root, string code, unordered_map<char, string> &huffmanCode) {
    if (root == nullptr) return;
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = code;
    }
    generateDictionary(root->left, code + "0", huffmanCode);
    generateDictionary(root->right, code + "1", huffmanCode);
}

// --- COMPRESSIONE MULTICORE ---
size_t compressHuffman(const string& text, const string& outFilename) {
    
    int num_threads = omp_get_max_threads();

    // 1. Calcolo frequenze MULTICORE (Map-Reduce)
    // Diamo a ogni thread la propria mappa per evitare conflitti (race conditions)
    vector<unordered_map<char, int>> local_freqs(num_threads);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        // Dividiamo il testo in blocchi uguali tra i thread
        #pragma omp for schedule(static)
        for (size_t i = 0; i < text.length(); ++i) {
            local_freqs[tid][text[i]]++;
        }
    }

    // Uniamo le mappe locali in un'unica mappa globale (Sequenziale, ma velocissimo)
    unordered_map<char, int> freq;
    for (int i = 0; i < num_threads; ++i) {
        for (auto pair : local_freqs[i]) {
            freq[pair.first] += pair.second;
        }
    }

    // 2. Costruzione Albero (Sequenziale)
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto pair : freq) pq.push(new Node(pair.first, pair.second));

    while (pq.size() > 1) {
        Node *left = pq.top(); pq.pop();
        Node *right = pq.top(); pq.pop();
        int sum = left->freq + right->freq;
        Node *parent = new Node('\0', sum);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    Node* root = pq.top();
    
    unordered_map<char, string> huffmanCode;
    generateDictionary(root, "", huffmanCode);

// 3. Traduzione in bit MULTICORE
    vector<string> local_encoded(num_threads);

    // OTTIZZAZIONE 1: Pre-allocazione della memoria (usare reserve, NON resize!)
    // Stimiamo circa 5 bit medi per carattere.
    size_t estimated_size = (text.length() / num_threads) * 5; 
    for (int i = 0; i < num_threads; ++i) {
        local_encoded[i].reserve(estimated_size); 
    }

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for schedule(static)
        for (size_t i = 0; i < text.length(); ++i) {
            // Usiamo .at() al posto di [] per garantire che sia Thread-Safe!
            local_encoded[tid] += huffmanCode.at(text[i]);
        }
    }

    // Uniamo le stringhe dei thread
    string encodedString = "";
    for (int i = 0; i < num_threads; ++i) {
        encodedString += local_encoded[i];
    }

    // 4. Bit-Packing (Sequenziale)
    vector<uint8_t> compressedBytes;
    uint8_t currentByte = 0;
    int bitCount = 0;

    for (char bit : encodedString) {
        currentByte = currentByte << 1; 
        if (bit == '1') currentByte = currentByte | 1; 
        bitCount++;
        if (bitCount == 8) {
            compressedBytes.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0) {
        currentByte = currentByte << (8 - bitCount); 
        compressedBytes.push_back(currentByte);
    }

    // 5. Salvataggio
    ofstream outFile(outFilename, ios::binary); 
    if (!outFile.is_open()) return 0;

    size_t textLen = text.length();
    outFile.write(reinterpret_cast<const char*>(&textLen), sizeof(textLen));
    size_t mapSize = freq.size();
    outFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    for (auto pair : freq) {
        outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
        outFile.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
    }

    outFile.write(reinterpret_cast<const char*>(compressedBytes.data()), compressedBytes.size());
    outFile.close();

    return sizeof(textLen) + sizeof(mapSize) + (mapSize * (sizeof(char)+sizeof(int))) + compressedBytes.size();
}

// --- DECOMPRESSIONE (Rimane sequenziale) ---
string decompressHuffman(const string& inFilename) {
    ifstream inFile(inFilename, ios::binary);
    if (!inFile.is_open()) return "";

    size_t textLen, mapSize;
    inFile.read(reinterpret_cast<char*>(&textLen), sizeof(textLen));
    inFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    // Fix applicato: push diretto nella priority_queue!
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (size_t i = 0; i < mapSize; ++i) {
        char ch;
        int f;
        inFile.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        inFile.read(reinterpret_cast<char*>(&f), sizeof(f));
        pq.push(new Node(ch, f));
    }

    if (pq.empty()) return ""; 

    while (pq.size() > 1) {
        Node *left = pq.top(); pq.pop();
        Node *right = pq.top(); pq.pop();
        int sum = left->freq + right->freq;
        Node *parent = new Node('\0', sum);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    Node* root = pq.top();

    vector<uint8_t> compressedBytes;
    char byteBuffer;
    while (inFile.read(&byteBuffer, sizeof(byteBuffer))) {
        compressedBytes.push_back(static_cast<uint8_t>(byteBuffer));
    }
    inFile.close();

    string decodedString = "";
    Node* current = root;
    size_t decodedCount = 0; 

    for (uint8_t byte : compressedBytes) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            if (bit == 0) current = current->left;
            else current = current->right;

            if (!current->left && !current->right) {
                decodedString += current->ch;
                current = root;               
                decodedCount++;
                if (decodedCount == textLen) return decodedString;
            }
        }
    }
    return decodedString;
}

string readFile(const string& filename) {
    ifstream file(filename); 
    if (!file.is_open()) return ""; 
    stringstream buffer;
    buffer << file.rdbuf(); 
    return buffer.str(); 
}