/**
 * root.c - Комбинированный метод хорд и касательных (улучшенная версия)
 * Реализация по методичке ВМК МГУ: строгое сохранение знака функции на концах отрезка.
 * 
 * Алгоритм:
 * 1. Вычисляем приближение методом хорд (всегда внутри (a,b) при смене знака)
 * 2. Вычисляем приближение методом касательных из конца, где F и F'' одного знака
 * 3. Выбираем лучшее приближение (ближе к корню, но внутри отрезка)
 * 4. Обновляем отрезок, сохраняя смену знака F(a)*F(b) < 0
 */
#include <math.h>
#include <stdio.h>

double root(double (*F)(double), double (*dF)(double),
            double a, double b, double eps1, int *iter, int verbose) {
    /* Объявления в начале (C89) */
    double fa, fb, f_new;
    double x_chord, x_tangent, x_new;
    double dfa, dfb;
    int n = 0;
    const int MAX_ITER = 10000;

    /* Гарантируем a < b */
    if (a > b) {
        double t = a; a = b; b = t;
    }
    
    fa = F(a);
    fb = F(b);

    /* Проверка: корень должен быть внутри (смена знака) */
    if (fa * fb > 0.0) {
        fprintf(stderr, "Warning: F(a)*F(b) > 0, корень может отсутствовать на отрезке.\n");
    }

    while (fabs(b - a) > eps1 && n < MAX_ITER) {
        fa = F(a);
        fb = F(b);
        dfa = dF(a);
        dfb = dF(b);

        /* === Метод хорд ===
         * Формула: x_c = a - F(a)*(b-a)/(F(b)-F(a))
         * Всегда даёт точку внутри (a,b) при F(a)*F(b) < 0
         */
        if (fabs(fb - fa) < 1e-15) {
            /* Защита от деления на почти ноль */
            x_chord = (a + b) * 0.5;
        } else {
            x_chord = a - fa * (b - a) / (fb - fa);
        }

        /* === Метод касательных (Ньютона) ===
         * Выбираем конец, где производная больше по модулю (устойчивее)
         * и где функция ближе к нулю
         */
        x_tangent = 1e30; /* Большое значение по умолчанию */
        
        if (fabs(dfa) > 1e-15 && fabs(fa) < fabs(fb)) {
            x_tangent = a - fa / dfa;
        } else if (fabs(dfb) > 1e-15) {
            x_tangent = b - fb / dfb;
        }

        /* === Выбор лучшего приближения ===
         * Приоритет: касательная внутри отрезка > хорда > середина
         */
        if (x_tangent >= a && x_tangent <= b) {
            /* Касательная внутри отрезка - используем её */
            x_new = x_tangent;
        } else if (x_chord >= a && x_chord <= b) {
            /* Хорда всегда внутри при смене знака, но проверим */
            x_new = x_chord;
        } else {
            /* fallback - середина отрезка */
            x_new = (a + b) * 0.5;
        }

        /* Вычисляем функцию в новой точке */
        f_new = F(x_new);

        /* === Обновление отрезка с сохранением смены знака ===
         * Если F(a)*F(x_new) < 0, то корень в [a, x_new]
         * Иначе корень в [x_new, b]
         */
        if (fa * f_new < 0.0) {
            b = x_new;
        } else if (f_new * fb < 0.0) {
            a = x_new;
        } else {
            /*特殊情况: f_new очень близко к 0 или знак не определился
             * Сжимаем отрезок к точке с меньшим |F|
             */
            if (fabs(fa) < fabs(fb)) {
                b = x_new;
            } else {
                a = x_new;
            }
        }

        n++;

        if (verbose) {
            printf("[Корень] Итер %3d: [%14.8f, %14.8f], len=%.2e\n", 
                   n, a, b, b - a);
        }
    }

    *iter = n;
    
    if (n >= MAX_ITER) {
        fprintf(stderr, "Warning: root() достиг максимума итераций (%d)\n", MAX_ITER);
    }
    
    return (a + b) * 0.5;
}