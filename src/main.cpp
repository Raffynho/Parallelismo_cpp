#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace std;

// 1. Struttura del Nodo dell'Albero di Huffman
struct Node {
    char ch;
    int freq;
    Node *left, *right;

    // Costruttore
    Node(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
};

// Comparatore personalizzato per creare una Min-Heap (Coda di priorità)
struct Compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq; // Il nodo con frequenza minore ha priorità più alta
    }
};

// 4. Generazione del Dizionario (Ricorsiva)
void generateDictionary(Node* root, string code, unordered_map<char, string> &huffmanCode) {
    if (root == nullptr) return;

    // Se è un nodo foglia (non ha figli), abbiamo trovato un carattere
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = code;
    }

    // Scendi a sinistra aggiungendo '0', scendi a destra aggiungendo '1'
    generateDictionary(root->left, code + "0", huffmanCode);
    generateDictionary(root->right, code + "1", huffmanCode);
}

// Funzione principale di compressione
size_t compressHuffman(const string& text, const string& outFilename) {
    // Fase 1: Calcolo delle frequenze
    unordered_map<char, int> freq;
    for (char ch : text) {
        freq[ch]++;
    }

    // Fase 2: Creazione della Coda di Priorità (Min-Heap)
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto pair : freq) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Fase 3: Costruzione dell'Albero di Huffman
    while (pq.size() > 1) {
        // Estrai i due nodi con frequenza minima
        Node *left = pq.top(); pq.pop();
        Node *right = pq.top(); pq.pop();

        // Crea un nuovo nodo interno (carattere nullo '\0')
        int sum = left->freq + right->freq;
        Node *parent = new Node('\0', sum);
        parent->left = left;
        parent->right = right;

        // Inserisci il nuovo nodo genitore nella coda
        pq.push(parent);
    }

    // La radice dell'albero è l'ultimo nodo rimasto
    Node* root = pq.top();

    // Fase 4: Generazione del dizionario
    unordered_map<char, string> huffmanCode;
    generateDictionary(root, "", huffmanCode);

    cout << "--- Dizionario di Huffman ---\n";
    for (auto pair : huffmanCode) {
        cout << pair.first << " : " << pair.second << "\n";
    }

    // Fase 5: Compressione e "Bit-packing"
    string encodedString = "";
    for (char ch : text) {
        encodedString += huffmanCode[ch]; 
    }

    // RIMOSSO: cout << "\nStringa binaria (concettuale): " << ...

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

    // NUOVA PARTE: Scrittura su file binario
    ofstream outFile(outFilename, ios::binary); // Apriamo in modalità binaria!
    
    if (!outFile.is_open()) {
        cerr << "Errore: Impossibile creare il file di output!\n";
        return 0;
    }

    outFile.write(reinterpret_cast<const char*>(compressedBytes.data()), compressedBytes.size());
    outFile.close();

    // Alla fine della funzione, invece di fare solo stampe, restituisci la grandezza:
    return compressedBytes.size();
}

string readFile(const string& filename) {
    ifstream file(filename); // Apre il file in modalità lettura
    
    // Controllo di sicurezza: il file esiste ed è stato aperto correttamente?
    if (!file.is_open()) {
        cerr << "Errore: Impossibile trovare o aprire il file '" << filename << "'\n";
        return ""; // Ritorna una stringa vuota in caso di errore
    }

    // Legge tutto il contenuto del file in modo efficiente
    stringstream buffer;
    buffer << file.rdbuf(); 
    
    return buffer.str(); // Converte il buffer in una stringa standard
}

int main() {
    string inputFile = "input.txt"; 
    string outputFile = "output.bin"; 
    
    string text = readFile(inputFile);
    if (text.empty()) return 1; 

    auto start = chrono::high_resolution_clock::now();

    // Ora salviamo il valore restituito dalla funzione
    size_t compSize = compressHuffman(text, outputFile);

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    double timeMs = duration.count() / 1000.0;

    // Stampa a schermo (opzionale, per vedere che sta lavorando)
    cout << "Tempo: " << timeMs << " ms | Dimensione: " << compSize << " byte\n";

    // NUOVO: Salvataggio nel file CSV in modalità "app" (Append)
    ofstream csvFile("benchmark_results.csv", ios::app);
    if (csvFile.is_open()) {
        // Scriviamo: Dimensione Originale, Dimensione Compressa, Tempo in millisecondi
        csvFile << text.length() << "," << compSize << "," << timeMs << "\n";
        csvFile.close();
    } else {
        cerr << "Errore nell'apertura del file CSV!\n";
    }

    return 0;
}