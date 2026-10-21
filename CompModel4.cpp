#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <algorithm>

using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    mt19937 mt{ static_cast<mt19937::result_type>(
        chrono::steady_clock::now().time_since_epoch().count()
    ) };

    // --- Параметры модели ---
    int N = 10;               // Число независимых потоков
    double a = 1.0, b = 7.0;  // Параметры равномерного распределения U(a, b)
    int M = 100000;           // Число событий итогового потока для статистики

    uniform_real_distribution<double> dist{ a, b };

    // --- Инициализация потоков ---
    // nextEvent[i] — момент следующего события i-го потока
    vector<double> nextEvent(N);
    for (int i = 0; i < N; i++) {
        nextEvent[i] = dist(mt); // первое событие каждого потока
    }

    // Вектор для хранения интервалов итогового потока
    vector<double> intervals;
    intervals.reserve(M);

    double currentTime = 0.0;

    // --- Моделирование ---
    for (int k = 0; k < M; k++) {
        // 1. Находим самый ранний момент среди всех потоков
        double minTime = nextEvent[0];
        int minIdx = 0;
        for (int i = 1; i < N; i++) {
            if (nextEvent[i] < minTime) {
                minTime = nextEvent[i];
                minIdx = i;
            }
        }

        // 2. Записываем интервал от предыдущего события
        double interval = minTime - currentTime;
        intervals.push_back(interval);
        currentTime = minTime;

        // 3. Разыгрываем этому потоку новое событие
        nextEvent[minIdx] = currentTime + dist(mt);
    }

    // --- Расчёт статистики ---
    double sumX = 0.0, sumX2 = 0.0;
    for (int i = 0; i < M; i++) {
        sumX += intervals[i];
        sumX2 += intervals[i] * intervals[i];
    }

    double meanX = sumX / M;
    double meanX2 = sumX2 / M;
    double D = meanX2 - meanX * meanX;
    double sigma = sqrt(D);
    double coeffVar = sigma / meanX; // Коэффициент вариации

    // --- Теоретические значения ---
    double lambda1 = 2.0 / (a + b);  // Интенсивность одного потока
    double lambdaSum = N * lambda1;  // Интенсивность суммарного потока
    double theoreticalMean = 1.0 / lambdaSum;

    // --- Вывод основных характеристик ---
    cout << "=== Лаб. 4: Суперпозиция " << N << " потоков ===\n";
    cout << "Параметры каждого потока: U(" << a << ", " << b << ")\n";
    cout << "Число смоделированных событий: " << M << "\n";
    cout << "---------------------------------------------------------\n";
    cout << fixed << setprecision(4);
    cout << "Интенсивность одного потока (lambda1):     " << lambda1 << "\n";
    cout << "Интенсивность итогового потока (lambdaSum): " << lambdaSum << "\n";
    cout << "Теоретический средний интервал (1/lambdaSum): " << theoreticalMean << "\n";
    cout << "Эмпирический средний интервал:             " << meanX << "\n";
    cout << "Дисперсия D:                               " << D << "\n";
    cout << "Среднеквадратичное отклонение sigma:       " << sigma << "\n";
    cout << "Коэффициент вариации (sigma/M):            " << coeffVar << "\n";
    cout << "  (у пуассоновского потока = 1)\n";

    // --- Построение гистограммы плотности ---
    int numBins = 20;          // Число столбцов гистограммы
    double binWidth = 5.0 / numBins; // Максимальный интервал ~ b, берём с запасом
    // На самом деле максимальный интервал в суперпозиции ~ b/N, но для наглядности берём шире

    // Уточним binWidth: возьмём диапазон [0, max_interval]
    double maxInterval = *max_element(intervals.begin(), intervals.end());
    binWidth = maxInterval / numBins;

    vector<int> counts(numBins, 0);
    for (int i = 0; i < M; i++) {
        int idx = (int)(intervals[i] / binWidth);
        if (idx >= numBins) idx = numBins - 1;
        counts[idx]++;
    }

    cout << "\n=== Гистограмма плотности распределения интервалов ===\n";
    cout << "Интервал\tЧастота\tЭмп. плотность f(x)\tТеор. f(x) = λe^(-λx)\n";
    cout << "---------------------------------------------------------\n";

    for (int j = 0; j < numBins; j++) {
        double left = j * binWidth;
        double right = (j + 1) * binWidth;

        // Эмпирическая плотность: f(x) ≈ (число в столбце) / (M * ширина столбца)
        double empiricalDensity = (double)counts[j] / (M * binWidth);

        // Теоретическая плотность пуассоновского потока в середине столбца
        double mid = (left + right) / 2.0;
        double theoreticalDensity = lambdaSum * exp(-lambdaSum * mid);

        cout << "[" << fixed << setprecision(3) << left << "; " << right << ")\t"
            << counts[j] << "\t" << setprecision(4) << empiricalDensity << "\t\t"
            << theoreticalDensity << "\n";
    }

    // --- Сравнение вероятностей попадания в интервалы ---
    cout << "\n=== Сравнение вероятностей P{x ∈ [c; d)} ===\n";
    cout << "Интервал\tЭмп. вероятность\tТеор. (e^-λc - e^-λd)\n";
    cout << "---------------------------------------------------------\n";

    for (int j = 0; j < numBins; j++) {
        double left = j * binWidth;
        double right = (j + 1) * binWidth;

        double empiricalProb = (double)counts[j] / M;
        double theoreticalProb = exp(-lambdaSum * left) - exp(-lambdaSum * right);

        cout << "[" << fixed << setprecision(3) << left << "; " << right << ")\t"
            << setprecision(4) << empiricalProb << "\t\t"
            << theoreticalProb << "\n";
    }

    return 0;
}