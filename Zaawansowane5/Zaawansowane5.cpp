/**
 * @file main.cpp
 * @brief Przykład całkowania funkcji \f$f(x) = \sqrt{1 - x^2}\f$ metodą prostokątów
 *        oraz wyliczania przybliżenia liczby \f$\pi\f$ z wykorzystaniem wielowątkowości.
 *
 * Program tworzy T wątków, z których każdy oblicza część sumy składowej całki na
 * wybranym przedziale. Następnie wyniki te są sumowane, aby otrzymać wartość całki.
 * Całkowany jest fragment okręgu jednostkowego zdefiniowany przez równanie
 * \f$y = \sqrt{1 - x^2}\f$ w przedziale [0, 1].
 * Przykładowe użycie:
 * @code
 *  ./program
 *  Podaj liczbe podprzedzialow (N): 100000000
 *  Podaj liczbe Watkow (N): 4
 * @endcode
 */

#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <chrono>

 /**
  * @brief Funkcja, którą całkujemy: \f$f(x) = \sqrt{1 - x^2}\f$.
  *
  * @param[in] x Argument funkcji.
  * @return Wartość \f$\sqrt{1 - x^2}\f$.
  */
inline double f(double x) {
    return std::sqrt(1.0 - x * x);
}

/**
 * @struct ThreadData
 * @brief Struktura przechowująca dane dla wątku.
 *
 * Struktura zawiera informacje o przedziale pracy danego wątku:
 * - start i end, czyli indeks początkowy i końcowy podprzedziałów,
 * - sum, w którym gromadzona jest częściowa suma,
 * - step, czyli szerokość jednego prostokąta do metody prostokątów.
 */
struct ThreadData {
    unsigned long long start; ///< Indeks początkowy przedziału do całkowania.
    unsigned long long end;   ///< Indeks końcowy przedziału do całkowania.
    double sum;               ///< Częściowa suma wyliczona przez wątek.
    double step;              ///< Szerokość jednego prostokąta.
};

/**
 * @brief Funkcja uruchamiana w wątku. Oblicza częściową sumę dla fragmentu całki.
 *
 * Metoda prostokątów bazuje na pobraniu wartości funkcji w środku podprzedziału:
 * \f$ x_i = (i + 0.5) \cdot step \f$.
 *
 * @param[in,out] data Struktura \c ThreadData z danymi wejściowymi (zakres przedziałów)
 *                     i wyjściowymi (sum częściowy).
 */
void threadFunction(ThreadData& data) {
    double partialSum = 0.0;
    for (unsigned long long i = data.start; i < data.end; ++i) {
        double x = (static_cast<double>(i) + 0.5) * data.step;
        partialSum += f(x);
    }
    data.sum = partialSum;
}

/**
 * @brief Funkcja główna programu.
 *
 * Wczytuje liczbę przedziałów \c N i liczbę wątków \c T.
 * Następnie dzieli pracę między wątki, tworzy je, uruchamia
 * i na koniec sumuje wyniki.
 * Zmierzone zostaje również całkowite zużycie czasu obliczeń.
 *
 * @return Kod wyjścia programu (0 jeśli zakończy się sukcesem).
 */
int main() {
    // Wczytanie parametrów: liczba podprzedziałów (N) i liczba wątków (T)
    unsigned long long N;
    int T;
    std::cout << "Podaj liczbe podprzedzialow (N): ";
    std::cin >> N;
    std::cout << "Podaj liczbe Watkow (N): ";
    std::cin >> T;

    // Rozpoczęcie pomiaru czasu
    auto startTime = std::chrono::high_resolution_clock::now();

    double step = 1.0 / static_cast<double>(N); // szerokość jednego prostokąta

    // Utworzenie wątków i wektorów do przechowywania danych
    std::vector<std::thread> threads(T);
    std::vector<ThreadData> threadData(T);

    // Podział przedziałów na wątki
    unsigned long long chunk = N / T;       // ilość pracy dla każdego wątku
    unsigned long long remainder = N % T;   // reszta do rozdysponowania

    unsigned long long currentStart = 0;

    for (int i = 0; i < T; ++i) {
        /**
         * @note Rozdzielamy resztę w taki sposób, aby wątki w pierwszej kolejności
         *       dostawały po jednym dodatkowym podprzedziale, jeżeli coś zostało.
         */
        unsigned long long currentEnd = currentStart + chunk;
        if (i < remainder) {
            currentEnd += 1;
        }

        threadData[i].start = currentStart;
        threadData[i].end = currentEnd;
        threadData[i].step = step;
        threadData[i].sum = 0.0;

        // Uruchomienie wątku
        threads[i] = std::thread(threadFunction, std::ref(threadData[i]));

        currentStart = currentEnd;
    }

    // Czekamy na zakończenie wszystkich wątków i sumujemy wyniki
    double totalSum = 0.0;
    for (int i = 0; i < T; ++i) {
        threads[i].join();
        totalSum += threadData[i].sum;
    }

    // Całka z sqrt(1 - x^2) w przedziale [0,1] to pi/4, stąd przybliżenie pi
    double integralValue = step * totalSum;
    double pi_approx = 4.0 * integralValue;

    // Koniec pomiaru czasu
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    // Wyświetlenie wyniku i czasu wykonania
    std::cout << "Oszacowana wartosc pi: " << pi_approx << std::endl;
    std::cout << "Czas obliczen: " << elapsed.count() << " s" << std::endl;

    return 0;
}
