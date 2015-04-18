/*
 * merge.h
 *
 *  Created on: 2015Äê4ÔÂ14ÈÕ
 *      Author: exiaoda
 */

#ifndef MERGE_MERGE_H_
#define MERGE_MERGE_H_

#define MAX_ARRAY_SIZE 10000
#define MAX_INPUT_FILES 8

typedef struct _Dot
{
      float x;
      float y;
}Dot;


int do_merge(Dot **p_dot, int num_array, int *size_of_arrays, FILE* print_fd);
int get_size(Dot *arr);
int bsearch_near(Dot *arr, int size, float x_key);
int all_finished(int *size_of_arrays, int *cursors, int num_array);
int f_compare(float x, float y);
void print_usage();
int read_file(FILE *fd, Dot** pdot_array);
int test();

#endif /* MERGE_MERGE_H_ */
