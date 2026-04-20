/**
 * root.c - Комбинированный метод хорд и касательных
 * Реализация по методичке ВМК МГУ: строгое сохранение знака функции на концах отрезка.
 */
#include <math.h>
#include <stdio.h>

double root(double (*F)(double), double (*dF)(double),
            double a, double b, double eps1, int *iter, int verbose) {
    /* Объявления в начале (C89) */
    double fa, fb, x_c, x_n, x_new;
    int n = 0;

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

    while (fabs(b - a) > eps1) {
        fa = F(a);
        fb = F(b);

        /* Метод хорд: всегда даёт точку внутри (a, b) при смене знака */
        x_c = a - fa * (b - a) / (fb - fa);

        /* Метод касательных: выбираем конец, где F и F'' одного знака */
        /* Для упрощения: берём касательную из того конца, где |F| меньше */
        if (fabs(fa) < fabs(fb)) {
            x_n = a - fa / dF(a);
        } else {
            x_n = b - fb / dF(b);
        }

        /* Если касательная вышла за пределы — игнорируем её */
        if (x_n < a || x_n > b) {
            x_new = x_c;
        } else {
            /* Берём среднее между хордой и касательной для устойчивости */
            x_new = (x_c + x_n) * 0.5;
        }

        /* Обновляем отрезок: сохраняем смену знака! */
        if (fa * F(x_new) < 0.0) {
            b = x_new;
        } else {
            a = x_new;
        }
        n++;

        if (verbose) {
            printf("[Корень] Итер %3d: [%12.8f, %12.8f]\n", n, a, b);
        }
    }

    *iter = n;
    return (a + b) * 0.5;
}