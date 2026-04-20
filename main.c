/**
 * main.c - Точка входа, управление параметрами, тестирование, расчёт площади фигуры
 * Вариант: 4 4 3 (Обычной сложности)
 * 
 * Функции кривых (ASM, x87, cdecl):
 *   f1(x) = exp(x) + 2,    df1(x) = exp(x)
 *   f2(x) = -1/x,          df2(x) = 1/x^2
 *   f3(x) = -2/3*(x+1),    df3(x) = -2/3
 * 
 * Методы:
 *   Корни: Комбинированный метод хорд и касательных (root.c)
 *   Интегралы: Адаптивная формула Симпсона (integral.c)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================================================
 * ОБЪЯВЛЕНИЯ ВНЕШНИХ ФУНКЦИЙ
 * ============================================================ */
extern double f1(double), df1(double);
extern double f2(double), df2(double);
extern double f3(double), df3(double);
extern double root(double (*)(double), double (*)(double),
                   double, double, double, int*, int);
extern double integral(double (*)(double), double, double,
                       double, int*, int);

/* ============================================================
 * ВСПОМОГАТЕЛЬНЫЕ ОБОРТКИ
 * ============================================================ */
static double F13(double x) { return f1(x) - f3(x); }
static double dF13(double x) { return df1(x) - df3(x); }
static double F23(double x) { return f2(x) - f3(x); }
static double dF23(double x) { return df2(x) - df3(x); }
static double F12(double x) { return f1(x) - f2(x); }
static double dF12(double x) { return df1(x) - df2(x); }
static double S13(double x) { return f1(x) - f3(x); }
static double S12(double x) { return f1(x) - f2(x); }

/* ============================================================
 * ТАБЛИЦЫ ФУНКЦИЙ ДЛЯ ТЕСТИРОВАНИЯ
 * ============================================================ */
static double (*get_func_by_id(int id))(double) {
    switch (id) {
        case 1:  return f1;
        case 2:  return f2;
        case 3:  return f3;
        case 4:  return F13;
        case 5:  return F23;
        case 6:  return F12;
        case 13: return S13;
        case 14: return S12;
        default: return NULL;
    }
}

static double (*get_dfunc_by_id(int id))(double) {
    switch (id) {
        case 7:  return df1;
        case 8:  return df2;
        case 9:  return df3;
        case 10: return dF13;
        case 11: return dF23;
        case 12: return dF12;
        default: return NULL;
    }
}

/* ============================================================
 * ТЕСТИРОВАНИЕ (п.6 ТЗ)
 * ============================================================ */
static int run_test(int argc, char *argv[]) {
    int i = 0;
    int func_id = 0, dfunc_id = 0;
    double a = 0.0, b = 0.0, eps = 0.0;
    int iter = 0;
    double result = 0.0;
    double (*F)(double) = NULL;
    double (*dF)(double) = NULL;
    char *type_str = NULL;
    char *endptr = NULL;
    int type_is_root = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t_func") == 0) {
            /* Формат: -t_func <type> <func_id> <dfunc_id> <a> <b> <eps> */
            if (i + 6 >= argc) {
                fprintf(stderr, "Ошибка: Недостаточно аргументов для -t_func.\n");
                fprintf(stderr, "Формат: -t_func <type> <func_id> <dfunc_id> <a> <b> <eps>\n");
                return 1;
            }

            type_str = argv[i + 1];
            if (strcmp(type_str, "root") == 0) {
                type_is_root = 1;
            } else if (strcmp(type_str, "integral") == 0) {
                type_is_root = 0;
            } else {
                fprintf(stderr, "Ошибка: Неверный тип: '%s'. Допустимо: 'root' или 'integral'.\n", type_str);
                return 1;
            }

            func_id = atoi(argv[i + 2]);
            F = get_func_by_id(func_id);
            if (!F) {
                fprintf(stderr, "Ошибка: Неверный ID функции (1-6, 13-14).\n");
                return 1;
            }

            dfunc_id = atoi(argv[i + 3]);
            if (type_is_root) {
                dF = get_dfunc_by_id(dfunc_id);
                if (!dF) {
                    fprintf(stderr, "Ошибка: Неверный ID производной (7-12).\n");
                    return 1;
                }
            }

            a = strtod(argv[i + 4], &endptr);
            if (endptr == argv[i+4] || *endptr != '\0') {
                fprintf(stderr, "Ошибка: Некорректное значение 'a': '%s'.\n", argv[i+4]);
                return 1;
            }

            b = strtod(argv[i + 5], &endptr);
            if (endptr == argv[i+5] || *endptr != '\0') {
                fprintf(stderr, "Ошибка: Некорректное значение 'b': '%s'.\n", argv[i+5]);
                return 1;
            }

            eps = strtod(argv[i + 6], &endptr);
            if (endptr == argv[i+6] || *endptr != '\0' || eps <= 0.0) {
                fprintf(stderr, "Ошибка: Некорректное значение 'eps': '%s'. Ожидается > 0.\n", argv[i+6]);
                return 1;
            }

            if (type_is_root) {
                printf("[TEST] root(f_id=%d, df_id=%d, a=%.4f, b=%.4f, eps=%.2e)\n", func_id, dfunc_id, a, b, eps);
                result = root(F, dF, a, b, eps, &iter, 1);
                printf("[RESULT] Корень = %.10f, Итераций = %d\n\n", result, iter);
            } else {
                printf("[TEST] integral(f_id=%d, a=%.4f, b=%.4f, eps=%.2e)\n", func_id, a, b, eps);
                result = integral(F, a, b, eps, &iter, 1);
                printf("[RESULT] Интеграл = %.10f, Итераций = %d\n\n", result, iter);
            }
            return 0;
        }
    }

    /* Если -t_func не указан, но вызван -t, запускаем автотесты */
    printf("=== ЗАПУСК ВСТРОЕННЫХ АВТОТЕСТОВ ===\n");
    
    F = F13; dF = dF13;
    result = root(F, dF, -5.0, -3.0, 1e-6, &iter, 1);
    printf("[AUTOTEST] F13 root=%.8f (iter=%d)\n", result, iter);

    F = F23; dF = dF23;
    result = root(F, dF, -2.5, -1.5, 1e-6, &iter, 1);
    printf("[AUTOTEST] F23 root=%.8f (iter=%d)\n", result, iter);

    F = S13;
    result = integral(F, -4.0, -2.0, 1e-5, &iter, 1);
    printf("[AUTOTEST] S13 integral=%.8f (iter=%d)\n", result, iter);

    printf("=== АВТОТЕСТЫ ЗАВЕРШЕНЫ ===\n\n");
    return 0;
}

/* ============================================================
 * СПРАВКА (-help)
 * ============================================================ */
static void print_help(void) {
    printf("========================================================================\n");
    printf(" ПРАКТИКУМ ВМК МГУ | ЗАДАНИЕ № 6 (Весенний семестр)\n");
    printf(" Тема: Сборка многомодульных программ. Вычисление корней и интегралов.\n");
    printf(" Вариант: 4 4 3 (Обычной сложности, ε = 0.001)\n");
    printf("========================================================================\n\n");
    
    printf("ОПИСАНИЕ ЗАДАЧИ:\n");
    printf("  С заданной точностью ε вычислить площадь плоской фигуры, ограниченной\n");
    printf("  тремя кривыми:\n");
    printf("    y = f1(x) = exp(x) + 2\n");
    printf("    y = f2(x) = -1/x\n");
    printf("    y = f3(x) = -2/3 * (x + 1)\n\n");
    
    printf("ЛОГИКА РЕШЕНИЯ:\n");
    printf("  1. Находятся абсциссы точек пересечения кривых (корни уравнений\n");
    printf("     f_i(x) = f_j(x)) методом комбинированных хорд и касательных (точность ε1).\n");
    printf("  2. Площадь фигуры представляется как алгебраическая сумма определенных\n");
    printf("     интегралов между найденными точками пересечения.\n");
    printf("  3. Интегралы вычисляются по квадратурной формуле Симпсона (парабол)\n");
    printf("     с адаптивным удвоением шага до достижения точности ε2.\n");
    printf("  4. ε1 и ε2 подбираются так, чтобы гарантировать итоговую погрешность ≤ ε.\n\n");
    
    printf("ИСПОЛЬЗУЕМЫЕ МЕТОДЫ:\n");
    printf("  - root.c  : Комбинированный метод хорд и касательных. Требует F(x) и F'(x).\n");
    printf("  - integral.c : Адаптивная формула Симпсона. Автоматически подбирает шаг.\n");
    printf("  - math_funcs.asm : f1..f3 и df1..df3 (NASM, x87 FPU, cdecl).\n\n");
    
    printf("КОМАНДНАЯ СТРОКА (CLI):\n");
    printf("  ./main_prog [опции]\n\n");
    
    printf("ОПЦИИ:\n");
    printf("  -h, -help           Вывести эту подробную справку.\n");
    printf("  -i, --verbose       Включить подробный вывод итераций (корни и интегралы).\n");
    printf("  -e <val>            Задать точность интегрирования ε2 вручную.\n");
    printf("                      (По умолчанию: ε/100 = 1e-5). Принимает только числа > 0.\n");
    printf("  -t, --test          Запустить набор встроенных автотестов root() и integral().\n");
    printf("  -t_func <type> <f_id> <df_id> <a> <b> <eps>\n");
    printf("                      Выполнить ручной тест конкретной функции.\n");
    printf("                      type   : \"root\" или \"integral\"\n");
    printf("                      f_id   : 1=f1, 2=f2, 3=f3, 4=F13, 5=F23, 6=F12, 13=S13, 14=S12\n");
    printf("                      df_id  : 7=df1, 8=df2, 9=df3, 10=dF13, 11=dF23, 12=dF12\n");
    printf("                      a, b   : Границы отрезка\n");
    printf("                      eps    : Точность (>0)\n\n");
    
    printf("ПРИМЕРЫ КОМАНД ДЛЯ ТЕСТА:\n");
    printf("  # Поиск корня F13(x)=0 на [-5, -3] с точностью 1e-6\n");
    printf("  ./main_prog -t_func root 4 10 -5.0 -3.0 0.000001\n\n");
    printf("  # Интегрирование S12(x) на [-1, 1] с точностью 0.0001\n");
    printf("  ./main_prog -t_func integral 14 0 -1.0 1.0 0.0001\n\n");
    printf("  # Запуск основной программы с логированием\n");
    printf("  ./main_prog -i\n\n");
    printf("  # Запуск с ручной точностью интеграла\n");
    printf("  ./main_prog -e 0.00001\n\n");
    printf("========================================================================\n");
}

/* ============================================================
 * ТОЧКА ВХОДА main()
 * ============================================================ */
int main(int argc, char *argv[]) {
    /* === ОБЪЯВЛЕНИЕ ВСЕХ ПЕРЕМЕННЫХ В НАЧАЛЕ БЛОКА (C89) === */
    double eps = 0.001;
    double eps1 = 1e-6;
    double eps2 = eps / 100.0;
    
    int verbose = 0;
    int test_mode = 0;
    int i = 1;
    
    double x1 = 0.0, x2 = 0.0, x3 = 0.0;
    int iter1 = 0, iter2 = 0, iter3 = 0;
    
    double I1 = 0.0, I2 = 0.0, total_area = 0.0;
    int iter_int1 = 0, iter_int2 = 0;
    
    double roots[3];
    double temp = 0.0;
    int j = 0;
    double a = 0.0, b_val = 0.0, c = 0.0;
    char *endptr = NULL;
    double val_e = 0.0;

    /* === СТРОГИЙ ПАРСИНГ АРГУМЕНТОВ С ВАЛИДАЦИЕЙ === */
    while (i < argc) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0) {
            test_mode = 1;
        } else if (strcmp(argv[i], "-t_func") == 0) {
            return run_test(argc, argv);
        } else if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Ошибка: Отсутствует значение для ключа -e.\n");
                return 1;
            }
            val_e = strtod(argv[i + 1], &endptr);
            if (endptr == argv[i+1] || *endptr != '\0') {
                fprintf(stderr, "Ошибка: Некорректное значение для ключа -e: '%s'. Ожидается вещественное число.\n", argv[i+1]);
                return 1;
            }
            if (val_e <= 0.0) {
                fprintf(stderr, "Ошибка: Значение точности -e должно быть строго положительным.\n");
                return 1;
            }
            eps2 = val_e;
            i++;
        } else {
            fprintf(stderr, "Ошибка: Неизвестный ключ командной строки: '%s'.\n", argv[i]);
            fprintf(stderr, "Используйте -help для вывода списка допустимых ключей.\n");
            return 1;
        }
        i++;
    }

    /* === РЕЖИМ ТЕСТИРОВАНИЯ === */
    if (test_mode) {
        return run_test(argc, argv);
    }

    /* === ИНФОРМАЦИОННЫЙ ВЫВОД === */
    printf("=== Расчёт площади фигуры (Вариант 4 4 3) ===\n");
    printf("f1 = exp(x)+2 | f2 = -1/x | f3 = -2/3*(x+1)\n");
    printf("eps = %.4f, eps1 = %.1e, eps2 = %.1e\n\n", eps, eps1, eps2);

    /* === ПОИСК ТОЧЕК ПЕРЕСЕЧЕНИЯ (корней) === */
    printf("--- Поиск абсцисс пересечений ---\n");
    x1 = root(F13, dF13, -5.0, -3.0, eps1, &iter1, verbose);
    x2 = root(F23, dF23, -2.5, -1.5, eps1, &iter2, verbose);
    x3 = root(F12, dF12, -0.5, -0.1, eps1, &iter3, verbose);

    if (!verbose) {
        printf("Найдены корни:\n");
        printf("  x1 (f1=f3): %.8f (итераций: %d)\n", x1, iter1);
        printf("  x2 (f2=f3): %.8f (итераций: %d)\n", x2, iter2);
        printf("  x3 (f1=f2): %.8f (итераций: %d)\n", x3, iter3);
    }

    /* === СОРТИРОВКА КОРНЕЙ === */
    roots[0] = x1;
    roots[1] = x2;
    roots[2] = x3;
    
    for (i = 0; i < 3; i++) {
        for (j = i + 1; j < 3; j++) {
            if (roots[i] > roots[j]) {
                temp = roots[i];
                roots[i] = roots[j];
                roots[j] = temp;
            }
        }
    }
    
    a = roots[0];
    b_val = roots[1];
    c = roots[2];

    /* === ВЫЧИСЛЕНИЕ ПЛОЩАДИ === */
    printf("\n--- Интегрирование (Симпсон) ---\n");
    I1 = integral(S13, a, b_val, eps2, &iter_int1, verbose);
    I2 = integral(S12, b_val, c, eps2, &iter_int2, verbose);

    total_area = I1 + I2;

    /* === ФИНАЛЬНЫЙ ВЫВОД === */
    printf("\n=== РЕЗУЛЬТАТ ===\n");
    printf("Пределы интегрирования: [%.6f, %.6f] и [%.6f, %.6f]\n", a, b_val, b_val, c);
    printf("S1 = %.8f (итераций интеграла: %d)\n", I1, iter_int1);
    printf("S2 = %.8f (итераций интеграла: %d)\n", I2, iter_int2);
    printf("Итоговая площадь S = %.8f\n", total_area);

    return 0;
}
