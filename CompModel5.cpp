#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    mt19937 mt{ static_cast<mt19937::result_type>(
        chrono::steady_clock::now().time_since_epoch().count()
    ) };

    // --- Параметры модели ---
    double a = 2.0, b = 80.0;       // Параметры U(a, b) для интервала между приходами
    // Условие устойчивости: a + b > 51 (см. вывод в конце)
    int sigma1 = 11;                 // Фиксированное время обработки на фазе 1
    int sigma2 = 17;                 // Фиксированное время обработки на фазе 2
    int N = 50000;                   // Число пришедших заданий

    uniform_real_distribution<double> distTau{ a, b };
    uniform_int_distribution<int> distK{ 1, 3 };
    uniform_int_distribution<int> distM{ 1, 2 };

    // --- Состояние системы ---
    int busy1 = 0;                   // 0 - сервер 1 свободен, 1 - занят
    int busy2 = 0;                   // 0 - сервер 2 свободен, 1 - занят

    long double T1 = distTau(mt);    // Время следующего прихода
    long double T2 = 1e18;           // Время окончания обслуживания на сервере 1
    long double T3 = 1e18;           // Время окончания обслуживания на сервере 2

    // --- Очереди (FIFO) ---
    // Очередь 1: хранит (время прихода в систему, k, m)
    vector<long double> q1_time;
    vector<int> q1_k;
    vector<int> q1_m;
    int head1 = 0;

    // Очередь 2: хранит (время прихода в систему, m)
    vector<long double> q2_time;
    vector<int> q2_m;
    int head2 = 0;

    // --- Задание, которое только что закончило фазу 1 и должно перейти на фазу 2 ---
    // (если сервер 2 занят, оно ждёт; храним его параметры)
    int transferPending = 0;         // 0 - нет, 1 - есть задание, ждущее сервер 2
    long double transferTime = 0.0;  // время прихода в систему этого задания
    int transferM = 0;               // его m

    // --- Статистика ---
    int totalDeparted = 0;
    long double sumSojourn = 0.0;
    long double server1WorkTime = 0.0;
    long double server2WorkTime = 0.0;
    long double prevTime = 0.0;

    // --- Вспомогательная лямбда для попытки запустить сервер 1 ---
    // (т.к. без функций - продублируем код в нужных местах)

    long double currentTime = 0.0;

    while (totalDeparted < N) {
        // 1. Находим ближайшее событие
        long double minT = T1;
        int TYP = 1;
        if (T2 < minT) { minT = T2; TYP = 2; }
        if (T3 < minT) { minT = T3; TYP = 3; }

        // Накапливаем время работы серверов
        server1WorkTime += busy1 * (minT - prevTime);
        server2WorkTime += busy2 * (minT - prevTime);
        prevTime = minT;
        currentTime = minT;

        // 2. Обработка событий
        if (TYP == 1) {
            // === ПРИХОД НОВОГО ЗАДАНИЯ ===
            int k = distK(mt);
            int m = distM(mt);

            // Планируем следующий приход
            T1 = currentTime + distTau(mt);

            // Пытаемся сразу на сервер 1
            if (busy1 == 0) {
                busy1 = 1;
                T2 = currentTime + sigma1;
                // k уменьшим при завершении фазы 1, здесь просто запомним задание
                // Чтобы не плодить сущности - положим в "текущее обслуживаемое" через очередь с приоритетом:
                // Упростим: кладём задание в очередь 1, а сервер его сразу заберёт ниже.
                // Но т.к. сервер только что занят - нужно помнить его k, m.
                // Сделаем через "виртуальную" очередь: кладём в q1, и сразу же вызовем логику обслуживания.
                q1_time.push_back(currentTime);
                q1_k.push_back(k);
                q1_m.push_back(m);
            }
            else {
                q1_time.push_back(currentTime);
                q1_k.push_back(k);
                q1_m.push_back(m);
            }
        }
        else if (TYP == 2) {
            // === КОНЕЦ ОБСЛУЖИВАНИЯ НА СЕРВЕРЕ 1 ===
            // Берём задание, которое обслуживалось (оно в голове q1)
            long double arrTime = q1_time[head1];
            int k = q1_k[head1];
            int m = q1_m[head1];
            q1_time.erase(q1_time.begin() + head1);
            q1_k.erase(q1_k.begin() + head1);
            q1_m.erase(q1_m.begin() + head1);

            k--; // один цикл на сервере 1 завершён

            if (k > 0) {
                // Возвращается в конец очереди 1
                q1_time.push_back(arrTime);
                q1_k.push_back(k);
                q1_m.push_back(m);
            }
            else {
                // k == 0 - нужно переходить на фазу 2
                if (busy2 == 0) {
                    busy2 = 1;
                    T3 = currentTime + sigma2;
                    q2_time.push_back(arrTime);
                    q2_m.push_back(m);
                }
                else {
                    // Сервер 2 занят - сохраняем задание для передачи позже
                    transferPending = 1;
                    transferTime = arrTime;
                    transferM = m;
                }
            }

            // Запускаем следующее задание из очереди 1 (если есть)
            if (head1 < (int)q1_time.size()) {
                T2 = currentTime + sigma1;
            }
            else {
                busy1 = 0;
                T2 = 1e18;
            }
        }
        else if (TYP == 3) {
            // === КОНЕЦ ОБСЛУЖИВАНИЯ НА СЕРВЕРЕ 2 ===
            long double arrTime = q2_time[head2];
            int m = q2_m[head2];
            q2_time.erase(q2_time.begin() + head2);
            q2_m.erase(q2_m.begin() + head2);

            m--; // один цикл на сервере 2 завершён

            if (m > 0) {
                // Возвращается в конец очереди 2
                q2_time.push_back(arrTime);
                q2_m.push_back(m);
            }
            else {
                // m == 0 - задание покидает систему
                sumSojourn += (currentTime - arrTime);
                totalDeparted++;

                // Если было задание, ждущее перехода на фазу 2 - запускаем его
                if (transferPending == 1) {
                    busy2 = 1;
                    T3 = currentTime + sigma2;
                    q2_time.push_back(transferTime);
                    q2_m.push_back(transferM);
                    transferPending = 0;
                }
            }

            // Запускаем следующее задание из очереди 2 (если есть)
            if (head2 < (int)q2_time.size()) {
                T3 = currentTime + sigma2;
            }
            else {
                busy2 = 0;
                T3 = 1e18;
            }
        }
    }

    // --- Вывод результатов ---
    double avgTau = (a + b) / 2.0;
    double lambda = 1.0 / avgTau;
    double rho1 = lambda * 2.0 * sigma1;   // E[k] = 2
    double rho2 = lambda * 1.5 * sigma2;   // E[m] = 1.5
    double rho1Emp = (double)server1WorkTime / currentTime;
    double rho2Emp = (double)server2WorkTime / currentTime;

    cout << "=== Лаб. 5: Двухфазная система с возвратами ===\n";
    cout << "Параметры: a = " << a << ", b = " << b
        << ", sigma1 = " << sigma1 << ", sigma2 = " << sigma2 << "\n";
    cout << "---------------------------------------------------------\n";
    cout << fixed << setprecision(4);
    cout << "Средний интервал между приходами: " << avgTau << "\n";
    cout << "Интенсивность поступления lambda: " << lambda << "\n";
    cout << "Обслужено заданий:                " << totalDeparted << "\n";
    cout << "Среднее время пребывания в системе: " << (sumSojourn / N) << "\n";
    cout << "\n--- Загрузка серверов ---\n";
    cout << "Сервер 1: теор. rho1 = " << rho1
        << ", эмп. = " << rho1Emp << "\n";
    cout << "Сервер 2: теор. rho2 = " << rho2
        << ", эмп. = " << rho2Emp << "\n";

    cout << "\n--- Условие устойчивости ---\n";
    cout << "rho1 = lambda * E[k] * sigma1 = (2/(a+b)) * 2 * 11 = 44/(a+b) < 1\n";
    cout << "rho2 = lambda * E[m] * sigma2 = (2/(a+b)) * 1.5 * 17 = 51/(a+b) < 1\n";
    cout << "ИТОГО: a + b > 51\n";
    cout << "В вашем случае a + b = " << (a + b);
    if (a + b > 51.0) cout << " > 51 — система УСТОЙЧИВА\n";
    else cout << " <= 51 — система НЕУСТОЙЧИВА (очереди растут бесконечно)\n";

    return 0;
}