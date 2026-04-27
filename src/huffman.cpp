#include "huffman.h"

// --- Strutture Private ---
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

// --- 1. COMPRESSIONE ---
size_t compressHuffman(const string& text, const string& outFilename) {
    unordered_map<char, int> freq;
    for (char ch : text) freq[ch]++;

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

    string encodedString = "";
    for (char ch : text) encodedString += huffmanCode[ch]; 

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

    // --- SALVATAGGIO CON HEADER ---
    ofstream outFile(outFilename, ios::binary); 
    if (!outFile.is_open()) return 0;

    // 1. Salviamo la lunghezza originale del testo
    size_t textLen = text.length();
    outFile.write(reinterpret_cast<const char*>(&textLen), sizeof(textLen));

    // 2. Salviamo la grandezza della mappa delle frequenze
    size_t mapSize = freq.size();
    outFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    // 3. Salviamo le coppie (carattere, frequenza)
    for (auto pair : freq) {
        outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
        outFile.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
    }

    // 4. Salviamo i dati compressi
    outFile.write(reinterpret_cast<const char*>(compressedBytes.data()), compressedBytes.size());
    outFile.close();

    // Ritorna il peso totale del file (Header + Dati)
    return sizeof(textLen) + sizeof(mapSize) + (mapSize * (sizeof(char)+sizeof(int))) + compressedBytes.size();
}

// --- 2. DECOMPRESSIONE ---
string decompressHuffman(const string& inFilename) {
    ifstream inFile(inFilename, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Errore in apertura del file binario!\n";
        return "";
    }

    // 1. Leggiamo l'header
    size_t textLen;
    inFile.read(reinterpret_cast<char*>(&textLen), sizeof(textLen));

    size_t mapSize;
    inFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    // ------
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (size_t i = 0; i < mapSize; ++i) {
        char ch;
        int f;
        inFile.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        inFile.read(reinterpret_cast<char*>(&f), sizeof(f));
        
        // Inseriamo DIRETTAMENTE nella coda.
        // Questo garantisce che l'ordine di inserimento sia IDENTICO 
        // a quello usato in fase di compressione!
        pq.push(new Node(ch, f));
    }
    // -----------------------------

    if (pq.empty()) return ""; // Sicurezza

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

    // 3. Leggiamo tutti i byte compressi rimasti nel file
    vector<uint8_t> compressedBytes;
    char byteBuffer;
    while (inFile.read(&byteBuffer, sizeof(byteBuffer))) {
        compressedBytes.push_back(static_cast<uint8_t>(byteBuffer));
    }
    inFile.close();

    // 4. Traduzione (Il cuore della decodifica)
    string decodedString = "";
    Node* current = root;
    size_t decodedCount = 0; // Contatore per fermarci al momento giusto

    for (uint8_t byte : compressedBytes) {
        // Leggiamo il byte partendo dal bit più a sinistra (indice 7) a quello a destra (0)
        for (int i = 7; i >= 0; --i) {
            // Estrae il singolo bit (0 o 1) usando maschere bit-a-bit
            bool bit = (byte >> i) & 1;

            // Naviga l'albero: 0 = sinistra, 1 = destra
            if (bit == 0) current = current->left;
            else current = current->right;

            // Se siamo arrivati a una foglia
            if (!current->left && !current->right) {
                decodedString += current->ch; // Salva il carattere
                current = root;               // Torna in cima all'albero
                decodedCount++;

                // Se abbiamo letto tutti i caratteri originali, fermiamoci! (Ignora i bit di padding)
                if (decodedCount == textLen) {
                    return decodedString;
                }
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