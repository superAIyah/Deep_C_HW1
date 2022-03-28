#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define SZ 25e6

#ifndef Point
#define Point

typedef struct Point2 {
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Point2;

#endif

static void fill_bits(int32_t* a);
static int32_t* create_mas(int sz);
static Point2 get_point(int32_t a);
static float dist(Point2 p);
static double count_segment(int32_t* mas, int left, int right);
double count_sum_dist_parallel(int32_t* mas, int sz, int num_proc);