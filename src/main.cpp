#include "BB84.hpp"

// Nagłówki potrzebne do DEFINICJI i I/O
#include <iostream>     // Dla std::cout
#include <random>       // Dla definicji generatorów (np. std::random_device rd;)
#include <vector>       // Chociaż w BB84.hpp, dla pewności i czystości kodu
#include <algorithm>    // Dla std::min
#include <fstream>
#include <string>


// Globalne generatory dla losowości - UŻYWAJĄ std::
// Zmienne te są globalne i dostępne dla funkcji w namespace BB84 i main()
std::random_device rd;
std::mt19937 generator(rd());
std::uniform_int_distribution<> bit_dist(0, 1); // Generuje 0 lub 1
std::uniform_real_distribution<> prob_dist(0.0, 1.0); // Generuje wartość [0, 1)


void record_data(double qber, double attack_intensity, const std::string& filename = "training_data.csv"){

    // std::ios::out   - otwórz do zapisu;   std::ios::app 
    std::fstream file(filename, std::ios::out | std::ios::app);

    // Jesli plik jest pusty, dodaj nagłówek

    if(file.tellp() == 0 ){
        file << "QBER_Value,Attack_detected\n";
    }

    // zapisz QBER i flagę ataku (1 jeśli attack_intensity > 0, 0 w przeciwnym razie )
    file << qber << "," << (attack_intensity >0.0 ? 1 : 0) << "\n";

    file.close();
}



namespace BB84 {

    Qubit generate_qubit() {        // 1. Kamil generuje losowy bit i losową bazę (Z=0, X=1).
        return {
            bit_dist(generator), // value
            bit_dist(generator)  // basis
        };
    }


    Qubit send_qubit(const Qubit& q, double natural_noise_level, double attack_intensity) {     // 2. Kanał transmisyjny (szum naturalny).
        Qubit transmitted_q = q;
        

        // 1.Podsłuch czyli atak intercept-resent
        // thief przechwytuje kubit z prawdopodobienstwem rownym attack_intensity

        if(prob_dist(generator) < attack_intensity){

            //thief mierzy kubit w losowej bazie (jak katarzyna)
            int thief_basis = bit_dist(generator);
            int thief_result;

            // Pomiar thief wprowadza blad w stan
            if(q.basis == thief_basis){
                thief_result = q.value; // zgodne bazy: sukces
            }else{
                thief_result = bit_dist(generator);// niezgodne bazy: 50% błędu
            }

            //thief tworzy nowy kubit i wysyla go dalej
            transmitted_q.value = thief_result;
            transmitted_q.basis = thief_basis;
        }


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
// ustawienia symulacji
    const int  NUM_RUNS = 2000; //liczba eksperymentów do zebrania danych
    const int QUBITS_PER_RUN = 5000; // Dlugosc klucza w pojedynczym eksperymencie
    const double NATURAL_NOISE = 0.01; // Stały szum kanału (1%)

    std::cout<<"Rozpoczynam generowanie danych treningowych( " << NUM_RUNS << " przebiegów)....\n";

    for(int run = 0; run <NUM_RUNS; ++run){
        double attack_intensity = 0.0;

        // Co drugie uruchomienie symulujemy atak ( 50% czysty, 50% atakowany)
        if(run % 2 ==1){
            //Atak subtelny: losowa intensywność od 1% do 15% przechwyconych kubitów
            std::uniform_real_distribution<> attack_dist(0.01, 0.15);
            attack_intensity = attack_dist(generator);
        }

        std::vector<Qubit> kamil_orginal;
        std::vector<Qubit> transmitted_qubits;

        for (int i = 0; i < QUBITS_PER_RUN; ++i) {
            Qubit q_orig = BB84::generate_qubit();
            Qubit q_trans = BB84::send_qubit(q_orig,NATURAL_NOISE, attack_intensity);
            kamil_orginal.push_back(q_orig);
            transmitted_qubits.push_back(q_trans);
        }
// --- Etap 2 & 3: Pomiar i Przesiewanie ---
        std::vector<int> katarzyna_bases(QUBITS_PER_RUN);
        std::vector<int> katarzyna_results(QUBITS_PER_RUN);

        for(int i = 0; i < QUBITS_PER_RUN; ++i){
            
            katarzyna_bases[i]= BB84::measure_qubit(transmitted_qubits[i],katarzyna_bases[i]);
        }

        std::vector<int> raw_key_kamil;
        std::vector<int> raw_key_katarzyna;
        BB84::sifting(kamil_orginal, katarzyna_bases, katarzyna_results, raw_key_kamil, raw_key_katarzyna);

      
// Etap 4. obliczanie QBER

        double qber = BB84::calculate_qber(raw_key_kamil, raw_key_katarzyna);

        record_data(qber, attack_intensity);

        if (run % 100 == 0) {
            std::cout << "Przebieg " << run << "/" << NUM_RUNS << " zakończony. QBER: " << qber * 100.0 << "%. Atak: " 
                      << (attack_intensity > 0.0 ? "TAK (Int: " + std::to_string(attack_intensity) + ")" : "NIE") << ".\n";
        }

    }
    

    std::cout<< "\n Generowanie danych zakończone. Dane treningowe w pliku training_data.csv\n";
    return 0;
}
