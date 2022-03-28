/*Задание: 
  Сравните и выведите в консоль время работы последовательного и параллельного с использованием нескольких процессов алгоритмов,
  каждый из которых выделяет в динамической памяти массив 4-байтовых чисел размером 100 Мб и,
  рассматривая каждое значение типа _int32_t как кортеж координат (x1, y1, x2, y2),
  где каждая координата может принимать значение от -128 до 127,
  последовательно вычисляет длину пути от первой до последней точки на координатной плоскости.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define SZ 25e6

typedef struct Point2 {
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Point2 ;

void fill_bits(int32_t* a) { // заполнить число случайными битами
    for (int i = 0; i < 32; i++) {
        int rand_0_1 = rand() % 2;
        *a = *a | (rand_0_1 << i);
    }
}

int32_t* create_mas(int sz) {  // создаем массив с точками
    int32_t* mas = malloc(sizeof(int32_t) * sz);
    for (int i = 0; i < sz; i++) {
        mas[i] = 0;
        fill_bits(&mas[i]);  // случайно флипаем биты в числе
    }
    return mas;
}

Point2 get_point(int32_t a) {  // прочитать из int32 4 числа int8
    int8_t* mas = malloc(sizeof(int8_t) * 4);
    for (int i = 0; i < 4; i++) {  // считываем x1, y1, x2, y2
        int8_t tmp = 0;
        for (int j = 0; j < 8; j++) {
            int bit = a & 1;  // считаем младший бит
            tmp = tmp | (bit << j); // присвоем его текущему числу
            a = a >> 1;  // сдвинем число вправо на 1 бит
        }
        mas[i] = tmp;
    }
    Point2 result = {mas[0], mas[1], mas[2], mas[3]};
    free(mas);
    return result;
}

float dist(Point2 p) {  // дистанция между (x1, y1) и (x2, y2)
    return sqrt(pow(p.x2 - p.x1, 2) + pow(p.y2 - p.y1, 2));
}

double count_segment(int32_t* mas, int left, int right) {
    double sum = 0;
    for (int i = left; i < right - 1; i++) {
        Point2 now = get_point(mas[i]);
        Point2 next = get_point(mas[i + 1]);
        sum += dist(now);  // расстояние отрезка
        Point2 between = {now.x2, now.y2, next.x1, next.y1};
        sum += dist(between);  // расстояние до следующего отрезка
    }
    Point2 lastest = get_point(mas[right - 1]); // расстояние последнего отрезка
    sum += dist(lastest);
    return sum;
}

double count_sum_dist(int32_t* mas, int sz, int num_proc) {  //посчитать суммарный путь по точкам в массиве
    int step = sz / num_proc;
    double sum_between = 0;  // расстояние между частями каждого процесса

    int fd[num_proc][2];  // pipe
    // fd[i][0] - reading
    // fd[i][1] - writing
    int cnt = 0;  // index of current pipe
    int i = 0;  // start index

    while (i < sz) {
        pipe(fd[cnt]);
        pid_t child = fork();

        if (child == 0) {  // мы внутри ребенка
            close(fd[cnt][0]);  // nothing to read

            int start = i;
            int end = start + step;
            if (cnt + 1 == num_proc) {  // если последний процесс
                end = sz;  // покроем остаток от деления на последней итерации
            }
            double sumi = count_segment(mas, start, end);

            write(fd[cnt][1], &sumi, sizeof(sumi));  // writing
            close(fd[cnt][1]);
            exit(0);
        }
        else {  // родительский процесс
            cnt++;  // переходим к следующей трубе
            i += step;  // переходим к след. отрезку
            if (cnt == num_proc) {  // остался остаток, который мы уже обработали -> конец алгоритма
                i = sz;
            }
            if (i < sz) {  // расстояние между отрезками
                Point2 last_of_prev = get_point(mas[i - 1]);
                Point2 first_of_next = get_point(mas[i]);
                Point2 between = {last_of_prev.x2, last_of_prev.y2, first_of_next.x1, first_of_next.y1};
                sum_between += dist(between);
            }
        }
    }
    // считываем суммы потоков
    double all_sum = 0;
    for (int i = 0; i < num_proc; i++) {
        double sum_child;
        close(fd[i][1]);  // закрываем конец на запись
        read(fd[i][0], &sum_child, sizeof(sum_child));
        close(fd[i][0]);
        all_sum += sum_child;
    }
    all_sum += sum_between;
    return all_sum;
}

void debug(Point2 a) {
    printf("%d %d\n", a.x1, a.y1);
    printf("%d %d\n", a.x2, a.y2);
}

int main() {
    int n = 1000;
    int proc = 12;
    int32_t* mas = create_mas(n);
    for (int i = 0; i < n; i++)
        mas[i] = i;
    for (int i = 0; i < n; i++) {
        debug(get_point(mas[i]));
    }
    printf("%f", count_sum_dist(mas, n, proc));
}