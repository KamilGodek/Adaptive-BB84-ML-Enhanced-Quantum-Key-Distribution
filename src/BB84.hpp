#ifndef BB84_HPP
#define BB84_HPP

#include <vector>
#include <random>     // std::random_device, std::mt19937, itp.
#include <algorithm>  // std::min

// Definicja Kubitu:
// W symulacji kubit to informacja o wartości (bit) oraz o bazie kodowania.
struct Qubit {
    int value; // 0 lub 1 (stan |0> lub |1>)
    int basis; // 0 = baza Z (rectilinear), 1 = baza X (diagonal)

};

// Deklaracje głównych funkcji protokołu BB84
namespace BB84 {

    // 1. Kamil generuje losowy kubit.
    Qubit generate_qubit();

    // 2. Kanał transmisyjny (z opcjonalnym szumem naturalnym).
    Qubit send_qubit(
        const Qubit& q, 
        double natural_noise_level = 0.0,
        double attack_intensity = 0.0
    );

    // 3. Katarzyna mierzy kubit w losowo wybranej bazie.
    // 'katarzyna_basis' jest zmienną wyjściową (przez referencję).
    int measure_qubit(const Qubit& q, int& katarzyna_basis);

    // 4. Przesiewanie (sifting): tworzenie Surowego Klucza.
    void sifting(
        const std::vector<Qubit>& kamil_qubits,       // Kubity wysłane przez Kamila
        const std::vector<int>& katarzyna_bases,     // Bazy wybrane przez Katarzynę
        const std::vector<int>& katarzyna_results,   // Wyniki pomiarów
        std::vector<int>& raw_key_kamil,             // Surowy klucz Kamila
        std::vector<int>& raw_key_katarzyna          // Surowy klucz Katarzyny
    );

    // 5. Obliczenie współczynnika błędu kwantowego QBER.
    double calculate_qber(
        const std::vector<int>& key_kamil,
        const std::vector<int>& key_katarzyna
    );
}

#endif // BB84_HPP
