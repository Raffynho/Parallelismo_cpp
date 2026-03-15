#include <iostream>
#include <fstream>
#include <random>
#include <string>

using namespace std;

// Ora il main accetta argomenti:
// argc = numero di argomenti (almeno 1, il nome del programma stesso)
// argv = array di stringhe contenente gli argomenti
int main(int argc, char* argv[]) {
    
    int numChars = 100000; // Valore di default (100 KB)

    // Se l'utente ha passato un argomento (oltre al nome del programma)
    if (argc > 1) {
        try {
            // Converte la stringa passata (argv[1]) in un intero
            numChars = stoi(argv[1]); 
        } catch (...) {
            cerr << "Attenzione: Valore non valido passato. Uso il default di 100000.\n";
        }
    }

    const string filename = "input.txt";
    ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        cerr << "Errore: Impossibile creare il file " << filename << "\n";
        return 1;
    }

    random_device rd;
    mt19937 gen(rd());
    string alphabet = "aaaaeeeeiiiioooouuuu  bcdfghlmnpqrstvz \n.,;";
    uniform_int_distribution<> distrib(0, alphabet.size() - 1);

    cout << "Generazione di " << numChars << " caratteri in corso...\n";

    for (int i = 0; i < numChars; ++i) {
        outFile << alphabet[distrib(gen)];
    }

    outFile.close();
    cout << "Fatto! File '" << filename << "' creato con successo.\n";

    return 0;
}