/*
 * merge.c
 *
 *  Created on: 2015-04-11
 *      Author: exiaoda
 *      ken.dai@outlook.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include "merge.h"


int all_finished(int *size_of_arrays, int *cursors, int num_array)
{
   int i;
   for(i = 0; i < num_array; i++)
   {
      if(cursors[i] < size_of_arrays[i])
      {
         return 0;
      }
   }

   /* looks we have finished iteration for all arrays
    * */
   return 1;
}


int do_merge(Dot **p_dot, int num_array, int *size_of_arrays, FILE* print_fd)
{
   /*read the input array : p_dot, each time choose
   the minimum x value.
   */
   int i;
   float summed_y;
   float min;


   int *cursors = malloc(sizeof(int) * num_array);
   memset(cursors, 0, sizeof(int) * num_array);

   /*iterate all array and find out the minimum x value at current cursor*/
   while(!all_finished(size_of_arrays, cursors, num_array))
   {
      min = 99999.0;
      for(i = 0; i < num_array; i++)
      {

         int cursor = cursors[i];
         if(cursors[i] == size_of_arrays[i])
         {
            continue;
         }

         if(p_dot[i][cursor].x < min)
         {
            min = p_dot[i][cursor].x;
         }
      }

      /* we must advance the cursor for each array
       * if the minimum x value is chosen from that array.
       * multiple arrays may have same minimum x value and need to
       * advance the cursor for all these arrays*/
      for(i = 0; i < num_array; i++)
      {
         int cursor = cursors[i];
         if(cursor == size_of_arrays[i])
         {
            continue;
         }

         if(p_dot[i][cursor].x == min)
         {
            cursors[i]++;
         }
      }

      // min is the x value that we need to calculate the y for every arrays.

      summed_y = 0;
      fprintf(print_fd, "%f", min);
      for(i = 0; i < num_array; i++)
      {
         /* calculate the y value for each array at the given x value (min)
          *  and print the summed y values for array 1, array 1+2, array 1+2+3,.....
          *  to standard output*/
         float x1;
         float x2;
         float y1;
         float y2;
         float calculated_y;
         int   out_range = 0;
         //find the nearest x value.

         int index = bsearch_near(p_dot[i], size_of_arrays[i], min);

         if(index == -1)
         {
            //printf("the size of array %d is zero\n", i);
            continue;
         }
         else if(f_compare(p_dot[i][index].x, min) == 0)
         {
            /*we found the x value in the array
             and no need to interpolate, the y value can be used
             directly. */
            calculated_y = p_dot[i][index].y;
         }
         else if(index == 0 || index == size_of_arrays[i])
         {
            /*the x value is out of range
             * then the contribution of this array is 0 */
            out_range = 1;
         }
         else
         {
            if(index == 0)
            {
               x1 = 0;
               y1 = 0;
            }
            else
            {
               x1 = p_dot[i][index - 1].x;
               y1 = p_dot[i][index - 1].y;
            }

            x2 = p_dot[i][index].x;
            y2 = p_dot[i][index].y;

            /* (x - x1) * (y2 - y1) / (x2 - x1) */
            if(x2 != x1)
            {
               /* calculate y based on interpolation*/
               calculated_y = y1 + (min - x1) * (y2 - y1) / (x2 - x1);
            }
         }

         if(out_range)
         {
            fprintf(print_fd, "\t%f",0.0);
         }
         else
         {
            summed_y += calculated_y;
            fprintf(print_fd, "\t%f",summed_y);
         }

      }

      fprintf(print_fd, "\n");
   }

   free(cursors);
   return 0;
}


/*compare two float number*/
int f_compare(float x, float y)
{
   float diff = x - y;
   if(fabs(diff) < 0.00001)
   {
      return 0;
   }
   else if(diff > 0)
   {
      return 1;
   }
   else
   {
      return -1;
   }
}


/* if the key is not in the array, always return the
   element that is bigger than the key.
   if the key is bigger than all the elements, then
   return -1
*/

int bsearch_near(Dot *arr, int size, float x_key)
{
   int bin;
   int min;
   int max;

   if(!size)
   {
      return -1;
   }

   if(size == 1)
   {
      return 1;
   }

   min = 0;
   max = size - 1;
   if(x_key > arr[size - 1].x)
   {
      return size;
   }
   else if(x_key < arr[0].x)
   {
      return 0;
   }

   while (max - min > 1)
   {
      bin = ( min + max ) / 2;
      if(f_compare(x_key, arr[bin].x) == 0)
      {
         return bin;
      }
      else if(f_compare(x_key, arr[ min ].x) == 0)
      {
         return min;
      }
      else if(f_compare(x_key, arr[ max ].x) == 0)
      {
         return max;
      }
      else if(f_compare(x_key, arr[ bin ].x) < 0)
      {
         max = bin;
      }
      else
      {
         min = bin;
      }
   }

   return max;
}


int read_file(FILE *fd, Dot** pdot_array)
{
   int n;
   int rownum = 0;
   float x;
   float y;

   while(!feof(fd))
   {
      x = 0.0;
      y = 0.0;
      n = fscanf(fd, "%f %f\n", &x, &y);
      if(n == 2)
      {
         rownum++;
      }
      else
      {
         /*format error*/
         return -1;
      }
   }

   if(rownum >= MAX_ARRAY_SIZE)
   {
      return -2;
   }
   else if(rownum == 0)
   {
      return -3;
   }

   *pdot_array = malloc(sizeof(Dot) * rownum);
   rewind(fd);
   rownum = 0;
   while(!feof(fd))
   {
      x = 0.0;
      y = 0.0;
      if(fscanf(fd, "%f %f\n", &x, &y) == 2)
      {
         (*pdot_array)[rownum].x = x;
         (*pdot_array)[rownum].y = y;
      }
      rownum++;
   }

   return rownum;
}


void print_usage()
{
   printf("usage: merge_curves -o output_file -i input_file_1 input_file_2 ... input_file_n\n");
}


/*
 *this program merges dataset files for gnuplot
usage : merge_curves -o output_file dataset1 dataset2  ... datasetn

dataset: the file stores data, which is in the plain text format
each line is one dot , i.e.
file dataset1:
1.2 5.0
2.3 3.5
3.8 3.0

file dataset2:
0.8 2.0
1.5 3.0
2.5 4.0

output:
0.8 0.0 2.0      # contribution from 1st array is 0, it is out of x range, from 2nd is 2.0
1.2 5.0 7.57     #5.0 + 2.0 + (1.2-0.8) * (3.0-2.0)/(1.5-0.8) contribution from 1st array is 5.0, from 2nd is 2.57
1.5 4.6 7.6      #3.0 + 5.0 + (1.5 - 1.2) * (3.5 - 5.0) / (2.3 - 1.2)  contribution from 1st array is 4.6, 2nd is 3.0
2.3  ...
2.5  ...
3.8  3.0 0.0     #contribution from 2nd array is 0, it is out of x range.


*/

int main(int argc, char **argv)
{
   /*find the nearst element in the array*/
   int index = 0;
   int num_input_arrays = 0;
   int *size_of_arrays = NULL;
   Dot **pDot = NULL;
   int ret;
   char c;
   FILE *out_fd = stdout;
   opterr = 0;
   while ((c = getopt (argc, argv, "vhto:")) != -1)
   {
      switch (c)
      {
         case 'v':
            printf("merge_curves v0.1\n");
            exit(0);
         case 'h':
            print_usage();
            exit(0);
         case 't':
            return test();

         case 'o':
            if(optarg == NULL)
            {
               fprintf(stderr, "output file missed with option -o\n");
               print_usage();
               exit(-1);
            }

            //strcpy(ouput_file, optarg);
            out_fd = fopen(optarg, "w");
            if(out_fd == NULL )
            {
               perror("output file fopen");
               exit(-1);
            }
            break;
         default:
            print_usage();
            exit(-1);
      }
   }

   num_input_arrays = argc - optind;
   if(num_input_arrays < 2)
   {
      fprintf(stderr, "you must provide at least 2 input data files!\n");
      print_usage();
      exit(-1);
   }

   pDot = malloc(sizeof(Dot *) * num_input_arrays);
   size_of_arrays = malloc(sizeof(int) * num_input_arrays);

   for (index = optind; index < argc; index++)
   {
      Dot *dot_array = NULL;
      FILE *input_fd = NULL;
      int rows;

      if(argc - index == MAX_INPUT_FILES)
      {
         fprintf(stderr, "the maximum number of input file must less than %d\n", MAX_INPUT_FILES);
         exit(-1);
      }

      input_fd = fopen(argv[index], "r");
      if(input_fd == NULL)
      {
         fprintf(stderr, "open file %s failed: %s", argv[index], strerror(errno));
         exit(-1);
      }
      rows = read_file(input_fd, &dot_array);
      if(rows == -1)
      {
         fprintf(stderr, "file %s format error", argv[index]);
         exit(-1);

      }
      else if(rows == -2)
      {
         fprintf(stderr, "file %s exceeds %d rows", argv[index], MAX_ARRAY_SIZE);
         exit(-1);
      }
      else if(rows == -3)
      {
         fprintf(stderr, "file %s has 0 row", argv[index]);
         exit(-1);
      }
      else if(dot_array == NULL)
      {
         fprintf(stderr, "file %s unknown reading error", argv[index]);
         exit(-1);
      }

      size_of_arrays[index - optind] = rows;
      pDot[index - optind] = dot_array;

      fclose(input_fd);
   }

   ret = do_merge(pDot, num_input_arrays, size_of_arrays, out_fd);
   if(ret == -1)
   {
      return -1;
   }

   fclose(out_fd);
   free(pDot);
   free(size_of_arrays);
   return 0;
}



/*for test purpose*/
int test()
{
   Dot *pDot[2];
   int *size_of_arrays;
   int ret;
   Dot dot1[] = {{1.2,5.0}, {2.3,3.5}, {3.8,3.0}};
   Dot dot2[] = {{0.8,2.0}, {1.5,3.0}, {2.5,4.0}};

   pDot[0] = dot1;
   pDot[1] = dot2;

   size_of_arrays = malloc(sizeof(int) * 2);
   size_of_arrays[ 0 ] = 3;
   size_of_arrays[ 1 ] = 3;

   ret = do_merge(pDot, 2, size_of_arrays, stdout);
   return ret;
}
