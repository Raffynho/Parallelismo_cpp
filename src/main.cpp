#include "huffman.h"

// Ora il main legge gli argomenti (argc, argv)
int main(int argc, char* argv[]) {
    // Se non passiamo nulla, lo chiamiamo "Sconosciuto"
    string runType = "Sconosciuto";
    if (argc > 1) {
        runType = argv[1];
    }

    string inputFile = "input.txt"; 
    string outputFile = "output.bin"; 
    
    string originalText = readFile(inputFile);
    if (originalText.empty()) return 1; 

    // Compressione
    auto startComp = chrono::high_resolution_clock::now();
    size_t compSize = compressHuffman(originalText, outputFile);
    auto endComp = chrono::high_resolution_clock::now();
    double timeCompMs = chrono::duration_cast<chrono::microseconds>(endComp - startComp).count() / 1000.0;

    // Decompressione
    auto startDec = chrono::high_resolution_clock::now();
    string decompressedText = decompressHuffman(outputFile);
    auto endDec = chrono::high_resolution_clock::now();
    double timeDecMs = chrono::duration_cast<chrono::microseconds>(endDec - startDec).count() / 1000.0;

    // Controllo Bit Perfect (Operatore ternario: se uguali = "SI", altrimenti "NO")
    string bitPerfect = (originalText == decompressedText) ? "T" : "F";

    // Stampa a schermo molto pulita per vedere a che punto è il benchmark
    cout << "  -> [" << runType << "] Compressione: " << timeCompMs 
         << " ms | Decompressione: " << timeDecMs << " ms | Perfetto: " << bitPerfect << "\n";

    // SALVATAGGIO COMPLETO NEL CSV!
    ofstream csvFile("benchmark_results.csv", ios::app);
    if (csvFile.is_open()) {
        // Scriviamo le 6 colonne nell'ordine esatto dell'intestazione
        csvFile << runType << "," 
                << originalText.length() << "," 
                << compSize << "," 
                << timeCompMs << "," 
                << timeDecMs << "," 
                << bitPerfect << "\n";
        csvFile.close();
    } else {
        cerr << "Errore nell'apertura del file CSV!\n";
    }

    return 0;
}