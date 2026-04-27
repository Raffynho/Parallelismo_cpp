CXX = g++
# Aggiunto -fopenmp ai flag globali
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -pthread -fopenmp

TARGET_SEQ = huffman_seq
TARGET_OMP = huffman_omp
TARGET_GEN = generator

.PHONY: all clean run-gen benchmark

# Di default compila tutto
all: $(TARGET_SEQ) $(TARGET_OMP) $(TARGET_GEN)

# L'eseguibile sequenziale unisce main.o e huffman.o
$(TARGET_SEQ): main.o huffman.o
	$(CXX) $(CXXFLAGS) -o $(TARGET_SEQ) main.o huffman.o

# L'eseguibile multicore unisce main.o e huffman_omp.o
$(TARGET_OMP): main.o huffman_omp.o
	$(CXX) $(CXXFLAGS) -o $(TARGET_OMP) main.o huffman_omp.o

$(TARGET_GEN): generator.o
	$(CXX) $(CXXFLAGS) -o $(TARGET_GEN) generator.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-gen: $(TARGET_GEN)
	./$(TARGET_GEN) $(CHARS)

seq-benchmark: all
	@echo "\nINIZIO BENCHMARK COMPARATIVO...\n"
	@echo "Tipo,Dimensione Originale (byte),Dimensione Compressa (byte),Tempo Compressione (ms),Tempo Decompressione (ms),Bit Perfect" > benchmark_results.csv
	@for size in 100000 500000 1000000 5000000 10000000 25000000 50000000; do \
		echo "=================================================="; \
		echo ">>> GENERAZIONE FILE CON $$size CARATTERI... <<<"; \
		./$(TARGET_GEN) $$size > /dev/null; \
		echo "--- ESECUZIONE SINGLE CORE ---"; \
		for i in 1 2 3; do \
			./$(TARGET_SEQ) Single-Core; \
		done; \
	done
	@echo "=================================================="
	@echo "BENCHMARK COMPLETATO! Risultati nel file CSV.\n"

multi-benchmark: all
	@echo "\nINIZIO BENCHMARK COMPARATIVO...\n"
	@echo "Tipo,Dimensione Originale (byte),Dimensione Compressa (byte),Tempo Compressione (ms),Tempo Decompressione (ms),Bit Perfect" > benchmark_results.csv
	@for size in 100000 500000 1000000 5000000 10000000 25000000 50000000; do \
		echo "=================================================="; \
		echo ">>> GENERAZIONE FILE CON $$size CARATTERI... <<<"; \
		./$(TARGET_GEN) $$size > /dev/null; \
		echo "--- ESECUZIONE MULTI CORE ---"; \
		for i in 1 2 3; do \
			./$(TARGET_OMP) Multi-Core; \
		done; \
	done
	@echo "=================================================="
	@echo "BENCHMARK COMPLETATO! Risultati nel file CSV.\n"
	
# Esegue il benchmark confrontando i due programmi!
benchmark: all
	@echo "\nINIZIO BENCHMARK COMPARATIVO...\n"
	@echo "Tipo,Dimensione Originale (byte),Dimensione Compressa (byte),Tempo Compressione (ms),Tempo Decompressione (ms),Bit Perfect" > benchmark_results.csv
	@for size in 100000 500000 1000000 5000000 10000000 25000000 50000000; do \
		echo "=================================================="; \
		echo ">>> GENERAZIONE FILE CON $$size CARATTERI... <<<"; \
		./$(TARGET_GEN) $$size > /dev/null; \
		echo "--- ESECUZIONE SINGLE CORE ---"; \
		for i in 1 2 3; do \
			./$(TARGET_SEQ) Single-Core; \
		done; \
		echo "--- ESECUZIONE MULTI CORE ---"; \
		for i in 1 2 3; do \
			./$(TARGET_OMP) Multi-Core; \
		done; \
	done
	@echo "=================================================="
	@echo "BENCHMARK COMPLETATO! Risultati nel file CSV.\n"

clean:
	rm -f *.o $(TARGET_SEQ) $(TARGET_OMP) $(TARGET_GEN) output.bin input.txt
