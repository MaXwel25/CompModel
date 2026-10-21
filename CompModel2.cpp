#include <iostream>
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

    int K = 0;
    int TYP = 1;
    int i = 1;
    int N = 100;

    int refusals = 0;
    int total_served = 0;

    long double systemtime = 0.0;
    long double T1 = 0.0;
    long double T2 = 1e9;

    uniform_real_distribution<double> tau{ 1.0, 10.0 };
    uniform_real_distribution<double> sigma{ 1.0, 5.0 };

    T1 = tau(mt);

    while (i <= N) {
        systemtime = min(T1, T2);

        if (T1 <= T2) {
            TYP = 1;
        }
        else {
            TYP = 2;
        }

        cout << "state (K = " << K << ") t = " << systemtime;

        if (TYP == 1) {
            cout << "; In i = " << i << " TYP = " << TYP << "\n";

            T1 = systemtime + tau(mt);
            i++;

            if (K == 0) {
                K = 1;
                T2 = systemtime + sigma(mt);
                total_served++;
            }
            else {
                refusals++;
            }
        }
        else {
            cout << "; Out: TYP = " << TYP << "\n";

            K = 0;
            T2 = 1e9;
        }
    }

    double P_refusal = (double)refusals / N;

    double avg_tau = (1.0 + 10.0) / 2.0;   // = 5.5
    double avg_sigma = (1.0 + 5.0) / 2.0;  // = 3.0
    double rho = avg_sigma / avg_tau;      // Загрузка системы

    cout << "\n=== Итоги моделирования ===\n";
    cout << "Всего поступило заявок: " << N << "\n";
    cout << "Обслужено заявок:       " << total_served << "\n";
    cout << "Получено отказов:       " << refusals << "\n";
    cout << fixed << setprecision(4);
    cout << "Эмпирическая вероятность отказа: " << P_refusal << "\n";
    cout << "Теоретическая загрузка (rho):    " << rho << "\n";

    return 0;
}