/*
* Copyright (C) 2013 - 2016  Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or (b) that interact
* with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/


#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
 
#define BUF_SIZE 2000

typedef union {  float f;struct {unsigned int num : 32;} parts;} float_cast;
typedef union {  float f;struct {unsigned int num : 32;} parts;} int_cast;


int main(int argc, char* argv[])
{
         float_cast d1;
         d1.f=1.1 ;
         printf("mantisa = %d\n", d1.parts.num);
         int_cast d2;
         d2.parts.num=d1.parts.num ;
         printf("float = %f\n", d2.f);
         
         int input_fd;
         int output_fd,dev_fd;
         
                     /* Input and output file descriptors */

         ssize_t ret_in, ret_out,ret_in1, ret_out1;    /* Number of bytes returned by read() andwrite() */
         
         char buffer[BUF_SIZE],buffer1[BUF_SIZE];      /* Character buffer */
     
                   /* Are src ,dev and dest file name arguments missing */
         
         if(argc != 4){
             printf ("Usage: cubic file1 device-name file2");
             return 1;
         }
         
                   /* Create input file descriptor */
         
         input_fd = open (argv [1], O_RDONLY);
         if (input_fd == -1) {
                 perror ("open");
                return 2;
         }
      
         
                    /* Create output file descriptor */
 
         output_fd = open(argv[3], O_WRONLY, 0644);
         if(output_fd == -1){
                 perror("open");
                 return 3;
         }
        
                   /* Create device file descriptor */
      
         dev_fd = open(argv[2], O_RDWR, 0644);
         if(dev_fd == -1){
                 perror("open");
                 return 3;
         }
     
      
                   /* sending process to device*/

         while((ret_in = read (input_fd, &buffer, BUF_SIZE)) > 0){
                 ret_out = write (dev_fd, &buffer, (ssize_t) ret_in);
                 if(ret_out != ret_in){
                     perror("write to dev");
                     return 4;
                 }
         }
      
                   /* receiving process from device*/

         while((ret_in1 = read (dev_fd, &buffer1, BUF_SIZE)) > 0){
                 ret_out1 = write (output_fd, &buffer1,BUF_SIZE);
                 if(ret_out1 != ret_in1){
                     perror("read from dev");
                     return 4;
                 }
         }
     
     
        
         close(input_fd);
         close (dev_fd);
         close (output_fd);
         printf("/********end*******!\n");
 }
