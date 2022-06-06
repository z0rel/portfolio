#include "sched_utils.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


static int
compare_ints(const void *a, const void *b);


static int
compare_ints(const void *a, const void *b)
{
    return (*(struct sched_int_median *) a).val - (*(struct sched_int_median *) b).val;
}


struct sched_int_median_result
sched_get_median(atomic_int *array, int i_size)
{
    // Allocate an array of the same size and sort it.
    struct sched_int_median *arr_sorted;
    struct sched_int_median *histogramm;
    int i, j, k;
    int item;
    double d_median;
    struct sched_int_median_result res = { 0, 0, 0 };

    if (i_size == 0)
        return res;

    arr_sorted = (struct sched_int_median *) malloc(i_size * sizeof(struct sched_int_median));
    histogramm = (struct sched_int_median *) malloc(i_size * sizeof(struct sched_int_median));

    for (i = 0; i < i_size; ++i)
    {
        arr_sorted[i].val = array[i];
        arr_sorted[i].item = i;
        histogramm[i].item = -1;
        histogramm[i].val = 0;
    }
    qsort(arr_sorted, i_size, sizeof(struct sched_int_median), &compare_ints);

    j = -1;
    item = INT32_MIN;
    for (i = 0; i < i_size; ++i)
    {
        if (item != arr_sorted[i].val)
        {
            item = arr_sorted[i].val;
            ++j;
            histogramm[j].item = item;
        }
        histogramm[j].val++;
    }

    printf("histogramm:\n");
    for (i = 0; i <= j; ++i)
    {
        printf("  %i:%3i: ", histogramm[i].item, histogramm[i].val);
        for (k = 0; k < i_size; ++k)
            if (array[k] == histogramm[i].item)
                printf("%i, ", k);
        printf("\n");
    }

    printf("\n");

    // Middle or average of middle values in the sorted array.
    if ((i_size % 2) == 0)
        d_median = (arr_sorted[i_size / 2].val + arr_sorted[(i_size / 2) - 1].val) / 2.0;
    else
        d_median = arr_sorted[i_size / 2].val;

    res.val = d_median;
    res.item = arr_sorted[i_size / 2].item;

    free(arr_sorted);
    free(histogramm);

    return res;
}
