#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <vector>
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    mt19937 mt{ static_cast<mt19937::result_type>(
        chrono::steady_clock::now().time_since_epoch().count()
    ) };

    int K = 0, TYP = 1, i = 1, L = 0, N = 100000;
    int total = 0;
    long double systemtime = 0.0, T1, T2 = 1e9, ML = 0.0;
    double lambda = 0.24;
    exponential_distribution<double> tau{ lambda };
    double mu = 0.3;
    exponential_distribution<double> sigma{ mu };

    vector<long double> arrival;
    vector<long double> wait;

    T1 = tau(mt);

    while (total < N) {
        systemtime = min(T1, T2);

        if (T1 <= T2) {
            TYP = 1;
        }
        else {
            TYP = 2;
        }

        if (TYP == 1) {
            ML += L;

            if (i < N) {
                T1 = systemtime + tau(mt);
            }
            else {
                T1 = 1e9;
            }
            i++;

            if (K == 0) {
                K = 1;
                T2 = systemtime + sigma(mt);
                wait.push_back(0.0);
                total++;
            }
            else {
                L++;
                arrival.push_back(systemtime);
            }
        }
        else {
            if (L == 0) {
                K = 0;
                T2 = 1e9;
            }
            else {
                L--;

                //uniform_int_distribution<size_t> dist(0, arrival.size() - 1);
                //size_t random_idx = dist(mt);

                //double arrival_time = arrival[random_idx];
                //arrival.erase(arrival.begin() + random_idx);

                double arrival_time = arrival[0];
                arrival.erase(arrival.begin());

                double wait_doub = systemtime - arrival_time;
                wait.push_back(wait_doub);
                total++;

                T2 = systemtime + sigma(mt);
            }
        }
    }

    cout << "Распределение и функция распределения:\n";
    cout << "Интервал\tЧисло задач\tВероятность\tF(x)\n";

    int all_interval = 15;
    vector<int> counts(all_interval + 1, 0);

    for (int i = 0; i < wait.size(); i++) {
        int idx = round(wait[i]);
        if (idx > all_interval) {
            idx = all_interval;
        }
        counts[idx]++;
    }

    double res_func = 0.0;
    for (int j = 0; j <= all_interval; j++) {
        double chast = 1.0 * counts[j] / N;
        res_func += chast;
        if (j < all_interval) {
            cout << j << "-" << j + 1 << " интервал\t" << counts[j] << "\t\t" << setprecision(4) << chast << "\t\t" << setprecision(4) << res_func << "\n";
        }
        else {
            cout << j << " п. интервал\t" << counts[j] << "\t\t" << setprecision(4) << chast << "\t\t" << setprecision(4) << res_func << "\n";
        }
    }

    cout << "Средняя длина очереди: " << ML / N << "\n";
    cout << "Среднее время ожидания: ";
    double sum_wait = 0.0;
    for (int i = 0; i < wait.size(); i++) {
        sum_wait += wait[i];
    }
    cout << sum_wait / N << "\n";
}