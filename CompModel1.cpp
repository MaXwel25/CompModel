#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    // Инициализация генератора случайных чисел
    mt19937 mt{ static_cast<mt19937::result_type>(
        chrono::steady_clock::now().time_since_epoch().count()
    ) };

    const int N = 100; // Количество событий

    // Параллельные векторы для хранения времени и типа событий
    vector<double> times(N);
    vector<int> types(N);

    uniform_int_distribution<int> distType{ 1, 4 };
    uniform_real_distribution<double> distTime{ 0.0, 100.0 };

    // 1. Генерация случайных событий
    for (int i = 0; i < N; i++) {
        types[i] = distType(mt);
        times[i] = distTime(mt);
    }

    cout << "=== Лаб. 1: События и их обработка ===\n";
    cout << "Сгенерировано " << N << " событий. Сортировка вставками...\n\n";

    // 2. Сортировка вставками по времени наступления
    for (int i = 1; i < N; i++) {
        double key_time = times[i];
        int key_type = types[i];
        int j = i - 1;

        // Сдвигаем элементы с большим временем вправо
        while (j >= 0 && times[j] > key_time) {
            times[j + 1] = times[j];
            types[j + 1] = types[j];
            j--;
        }
        // Вставляем ключи на правильные позиции
        times[j + 1] = key_time;
        types[j + 1] = key_type;
    }

    // 3. Последовательная обработка событий
    cout << "Начало обработки событий в хронологическом порядке:\n";
    cout << "----------------------------------------------------\n";

    for (int i = 0; i < N; i++) {
        // Выводим первые 15 и последние 5 событий
        if (i < 15 || i >= N - 5) {
            cout << "Событие #" << (i + 1) << " (Тип: " << types[i]
                << ", Время: " << fixed << setprecision(2) << times[i] << ")\n";

            // Обработка события через if/else if (без switch/case)
            if (types[i] == 1) {
                cout << "  -> [Обработчик 1] Инициализация системы.\n";
            }
            else if (types[i] == 2) {
                cout << "  -> [Обработчик 2] Поступление новой заявки.\n";
            }
            else if (types[i] == 3) {
                cout << "  -> [Обработчик 3] Завершение обслуживания.\n";
            }
            else if (types[i] == 4) {
                cout << "  -> [Обработчик 4] Системный сбой / логирование.\n";
            }
            else {
                cout << "  -> [Ошибка] Неизвестный тип события.\n";
            }
        }
        else if (i == 15) {
            cout << "  ... (пропущено " << (N - 20) << " событий для краткости) ...\n";
        }
    }

    cout << "----------------------------------------------------\n";
    cout << "Моделирование завершено.\n";

    return 0;
}