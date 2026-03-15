# ifndef FUNCTION_H
# define FUNCTION_H

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



# endif