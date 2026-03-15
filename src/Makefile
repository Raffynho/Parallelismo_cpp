CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -pthread

# Nomi dei due eseguibili
TARGET_HUFFMAN = huffman
TARGET_GEN = generator

CHARS ?= 1000000

# Diciamo a Make quali comandi non sono file fisici
.PHONY: all clean run run-gen

# Di default, 'make' ora compila ENTRAMBI i programmi
all: $(TARGET_HUFFMAN) $(TARGET_GEN)

# Regola per il programma di compressione
$(TARGET_HUFFMAN): main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET_HUFFMAN) main.o

# Regola per il generatore di testo
$(TARGET_GEN): generator.o
	$(CXX) $(CXXFLAGS) -o $(TARGET_GEN) generator.o

# Regola per compilare i file .cpp in file .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Esegue il programma di compressione
run: $(TARGET_HUFFMAN)
	./$(TARGET_HUFFMAN)

# Esegue il programma generatore di testo passandogli la variabile CHARS
run-gen: $(TARGET_GEN)
	./$(TARGET_GEN) $(CHARS)

# Esegue un test prestazionale automatico e salva i risultati in un CSV
benchmark: $(TARGET_GEN) $(TARGET_HUFFMAN)
	@echo "\nINIZIO BENCHMARK AUTOMATICO...\n"
	@echo "Preparazione file CSV..."
	@echo "Dimensione Originale (byte),Dimensione Compressa (byte),Tempo (ms)" > benchmark_results.csv
	@for size in 1000 100000 500000 1000000 5000000 10000000 100000000 1000000000; do \
		echo "=================================================="; \
		echo ">>> GENERAZIONE FILE CON $$size CARATTERI... <<<"; \
		./$(TARGET_GEN) $$size > /dev/null; \
		echo ">>> ESECUZIONE (5 RIPETIZIONI) <<<"; \
		for i in 1 2 3 4 5 6 7 8 9 10; do \
			echo -n "Run $$i: "; \
			./$(TARGET_HUFFMAN); \
		done; \
	done
	@echo "=================================================="
	@echo "BENCHMARK COMPLETATO! Risultati salvati in 'benchmark_results.csv'\n"
# Pulisce i file generati
clean:
	rm -f *.o $(TARGET_HUFFMAN) $(TARGET_GEN) output.bin input.txt
