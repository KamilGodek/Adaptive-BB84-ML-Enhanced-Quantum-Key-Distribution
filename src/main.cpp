#include "BB84.hpp"

// Nagłówki potrzebne do DEFINICJI i I/O
#include <iostream>     // Dla std::cout
#include <random>       // Dla definicji generatorów (np. std::random_device rd;)
#include <vector>       // Chociaż w BB84.hpp, dla pewności i czystości kodu
#include <algorithm>    // Dla std::min

// Globalne generatory dla losowości - UŻYWAJĄ std::
// Zmienne te są globalne i dostępne dla funkcji w namespace BB84 i main()
std::random_device rd;
std::mt19937 generator(rd());
std::uniform_int_distribution<> bit_dist(0, 1); // Generuje 0 lub 1
std::uniform_real_distribution<> prob_dist(0.0, 1.0); // Generuje wartość [0, 1)

namespace BB84 {

    Qubit generate_qubit() {        // 1. Kamil generuje losowy bit i losową bazę (Z=0, X=1).
        return {
            bit_dist(generator), // value
            bit_dist(generator)  // basis
        };
    }


    Qubit send_qubit(const Qubit& q, double natural_noise_level) {     // 2. Kanał transmisyjny (szum naturalny).
        Qubit transmitted_q = q;
        
        // Szum: z małym prawdopodobieństwem odwraca bit.
        if (prob_dist(generator) < natural_noise_level) {
            transmitted_q.value = 1 - transmitted_q.value; // Odwrócenie bitu
        }
        return transmitted_q;
    }


    int measure_qubit(const Qubit& q, int& katarzyna_basis) {         // 3. Katarzyna mierzy kubit
        katarzyna_basis = bit_dist(generator); // Katarzyna losuje bazę
        
        if (q.basis == katarzyna_basis) {
            // Zgodne bazy: pomiar jest deterministyczny.
            return q.value;
        } else {
            // Niezgodne bazy: wynik pomiaru jest losowy (50/50).
            return bit_dist(generator);
        }
    }


    void sifting(                // 4. Przesiewanie (Sifting)
        const std::vector<Qubit>& kamil_qubits, // Zmiana nazwy zmiennej
        const std::vector<int>& katarzyna_bases, // Zmiana nazwy zmiennej
        const std::vector<int>& katarzyna_results, // Zmiana nazwy zmiennej
        std::vector<int>& raw_key_kamil, // Zmiana nazwy zmiennej
        std::vector<int>& raw_key_katarzyna // Zmiana nazwy zmiennej
    ) {
        raw_key_kamil.clear();
        raw_key_katarzyna.clear();
        
        for (std::size_t i = 0; i < kamil_qubits.size(); ++i) {
            // Surowy Klucz tylko dla bitów, gdzie bazy są zgodne.
            if (kamil_qubits[i].basis == katarzyna_bases[i]) {
                raw_key_kamil.push_back(kamil_qubits[i].value);
                raw_key_katarzyna.push_back(katarzyna_results[i]);
            }
        }
    }


    double calculate_qber(          // 5. Obliczenie QBER
        const std::vector<int>& key_kamil, // Zmiana nazwy zmiennej
        const std::vector<int>& key_katarzyna // Zmiana nazwy zmiennej
    ) {
        if (key_kamil.empty() || key_katarzyna.empty()) return 0.0;
        
        int errors = 0;
        // Używamy std::size_t dla bezpieczeństwa w porównaniach rozmiarów
        std::size_t min_len = std::min(key_kamil.size(), key_katarzyna.size()); 

        for (std::size_t i = 0; i < min_len; ++i) {
            if (key_kamil[i] != key_katarzyna[i]) {
                errors++;
            }
        }
        return (double)errors / min_len;
    }
}

// Główna funkcja symulacji
int main() {
    // --- Ustawienia symulacji ---
    const int NUM_QUBITS = 10000;
    const double NATURAL_NOISE = 0.01; // 1% naturalnego szumu w kanale
    
    // --- Etap 1: Generacja i Transmisja ---
    std::vector<Qubit> kamil_original; // Kubity wysłane przez Kamila
    std::vector<Qubit> transmitted_qubits; // Kubity, które dotarły do Katarzyny

    for (int i = 0; i < NUM_QUBITS; ++i) {
        Qubit q_orig = BB84::generate_qubit();
        Qubit q_trans = BB84::send_qubit(q_orig, NATURAL_NOISE);
        kamil_original.push_back(q_orig);
        transmitted_qubits.push_back(q_trans);
    }
    
    std::cout << "1. Generacja i transmisja " << NUM_QUBITS << " kubitów zakończona.\n";

    // --- Etap 2: Pomiar przez Katarzynę ---
    std::vector<int> katarzyna_bases(NUM_QUBITS);
    std::vector<int> katarzyna_results(NUM_QUBITS);

    for (int i = 0; i < NUM_QUBITS; ++i) {
        int katarzyna_basis = 0; // Zmienna tymczasowa dla katarzyna_basis
        // Przekazujemy katarzyna_bases[i] do funkcji, aby zapisać wylosowaną bazę
        katarzyna_results[i] = BB84::measure_qubit(transmitted_qubits[i], katarzyna_bases[i]); 
    }
    std::cout << "2. Katarzyna dokonała pomiarów.\n";

    // --- Etap 3: Przesiewanie (Sifting) ---
    std::vector<int> raw_key_kamil;
    std::vector<int> raw_key_katarzyna;
    
    BB84::sifting(kamil_original, katarzyna_bases, katarzyna_results, raw_key_kamil, raw_key_katarzyna);

    std::cout << "3. Przesiewanie baz zakończone. Długość Surowego Klucza (Raw Key): " 
              << raw_key_kamil.size() << " bitów.\n";
              
    // --- Etap 4: Obliczenie QBER ---
    double qber = BB84::calculate_qber(raw_key_kamil, raw_key_katarzyna);
    
    std::cout << "4. Obliczony QBER (szum naturalny): " << qber * 100.0 << "%\n";

    return 0;
}
