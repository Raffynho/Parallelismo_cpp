# ifndef FUNCTION_H
# define FUNCTION_H

#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

string readFile(const string& filename);

// Funzione che comprime il file di testo in ingresso
size_t compressHuffman(const string& text, const string& outFilename);

// Funzione che decomprime il documento .bin ottenuto dalla funzione cmpressHuffman
string decompressHuffman(const string& inFilename);

# endif