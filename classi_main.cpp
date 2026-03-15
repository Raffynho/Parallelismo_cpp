#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <vector>
#include <cstdint>

using namespace std;

class HuffmanCoder {
private:
    // 1. Strutture private: visibili solo all'interno della classe
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

    // 2. Variabili di stato (Attributi)
    Node* root;
    unordered_map<char, string> huffmanCode;

    // 3. Metodi privati di supporto
    void generateDictionaryRecursive(Node* node, string code) {
        if (node == nullptr) return;

        if (!node->left && !node->right) {
            huffmanCode[node->ch] = code;
        }

        generateDictionaryRecursive(node->left, code + "0");
        generateDictionaryRecursive(node->right, code + "1");
    }

    void freeTree(Node* node) {
        if (node == nullptr) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node; // Libera la memoria allocata con 'new'
    }

public:
    // Costruttore
    HuffmanCoder() {
        root = nullptr;
    }

    // Distruttore: viene chiamato in automatico quando l'oggetto viene distrutto
    ~HuffmanCoder() {
        freeTree(root);
    }

    // Metodo pubblico per comprimere
    vector<uint8_t> compress(const string& text) {
        // Pulizia preliminare in caso di chiamate multiple allo stesso oggetto
        if (root != nullptr) {
            freeTree(root);
            root = nullptr;
            huffmanCode.clear();
        }

        // Calcolo frequenze
        unordered_map<char, int> freq;
        for (char ch : text) {
            freq[ch]++;
        }

        // Coda di priorità
        priority_queue<Node*, vector<Node*>, Compare> pq;
        for (auto pair : freq) {
            pq.push(new Node(pair.first, pair.second));
        }

        // Costruzione Albero
        while (pq.size() > 1) {
            Node *left = pq.top(); pq.pop();
            Node *right = pq.top(); pq.pop();

            int sum = left->freq + right->freq;
            Node *parent = new Node('\0', sum);
            parent->left = left;
            parent->right = right;

            pq.push(parent);
        }
        root = pq.top();

        // Generazione dizionario
        generateDictionaryRecursive(root, "");

        // Stampa il dizionario (opzionale, per debug)
        cout << "--- Dizionario di Huffman ---\n";
        for (auto pair : huffmanCode) {
            cout << pair.first << " : " << pair.second << "\n";
        }

        // Bit-packing
        string encodedString = "";
        for (char ch : text) {
            encodedString += huffmanCode[ch];
        }

        vector<uint8_t> compressedBytes;
        uint8_t currentByte = 0;
        int bitCount = 0;

        for (char bit : encodedString) {
            currentByte = currentByte << 1;
            if (bit == '1') {
                currentByte = currentByte | 1;
            }
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

        return compressedBytes; // Restituisce il vettore di byte
    }
};

int main() {
    string text = "questo e' un test per la codifica di huffman in c++ usando le classi!";
    cout << "Testo originale: " << text << "\n\n";
    cout << "Dimensione originale: " << text.length() << " byte\n\n";

    // Creiamo l'oggetto
    HuffmanCoder coder;

    // Chiamiamo il metodo pubblico
    vector<uint8_t> compressedData = coder.compress(text);

    cout << "\nDimensione compressa: " << compressedData.size() << " byte\n";

    // Quando il main finisce, 'coder' viene distrutto e il Distruttore libera la memoria dell'albero in automatico.
    return 0;
}