#include <stdio.h>                                                                  
#include <stdlib.h>                                                                 
#include <dirent.h>                                                                 
#include <string.h>                                                                 
#include <math.h>                                                                   
#include <assert.h>                                                                 
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

#define BYTES_PER_READ 2                                                            
#define NIFTI_HEADER_SIZE 352

struct Nifti{
  char *h_buffer;
  unsigned short *buffer;
  size_t img_size;
};

struct AppArgs{
  char *in_file;
  char *out_dir;
  long int iterations;
};

struct Nifti read_img(char* filename){

  long size;
  size_t number_of_ints;
  size_t number_of_elements;
  size_t h_read;
  size_t read;
  char *h_buffer;
  unsigned short *buffer;

  FILE *file = fopen(filename, "r");

  if (!file) {
      fprintf(stderr, "Unable to open/create file,%s\n", filename);
      exit( 1 );
  }

  /* Size of file */
  fseek(file, 0, SEEK_END);
  size = ftell(file) - NIFTI_HEADER_SIZE;
  fseek(file, 0, SEEK_SET);

  /* 'size' is the number of bytes, not the number of ints,
     so we need to adjust accordingly */
  number_of_ints = (size % sizeof(unsigned short) ? 1 : 0) + (size / sizeof(unsigned short));

  number_of_elements = (size % BYTES_PER_READ ? 1 : 0) +
      (size / BYTES_PER_READ);


  if(!(h_buffer = malloc(NIFTI_HEADER_SIZE))) {
      fprintf(stderr, "Failed to allocate header memory\n");
      exit( 1 );
  }
  if(!(buffer = malloc(number_of_ints * sizeof(unsigned short)))) {
      fprintf(stderr, "Failed to allocate buffer memory\n");
      exit( 1 );
  }

  h_read = fread(h_buffer, 1, NIFTI_HEADER_SIZE, file);
  read = fread(buffer, BYTES_PER_READ, number_of_elements, file);

  struct Nifti img = {h_buffer, buffer, read};
  fclose(file);
  
  return img;

}

unsigned short* increment(unsigned short* buffer, int img_size ){

  for( int  i = 0; i < img_size; i = i + 1 ){

    buffer[i] += 1;
  }

  return buffer;

}

void write_image(struct Nifti img, char* output_fn){

  FILE *of = fopen(output_fn, "w");
  if(img.buffer == NULL) {
      fprintf(stderr, "buffer is null\n");
      exit( 1 );
  }

  fwrite(img.h_buffer, 1, NIFTI_HEADER_SIZE, of);
  fwrite(img.buffer, BYTES_PER_READ, img.img_size, of);

  free(img.buffer);
  free(img.h_buffer);

  fclose(of);

}

void *start_app(void *args){                                                     
                                                                               
  struct AppArgs* aargs = args;                                                
  char *in_file = aargs->in_file;                                                
  char *out_dir = aargs->out_dir;                                              
  long int iterations = aargs->iterations;                                     
  time_t start;
  time_t end;

  char hostname[HOST_NAME_MAX + 1];
  gethostname(hostname, HOST_NAME_MAX + 1);
                                                                               
  char* inbn = basename(in_file);
  char outfn[strlen(out_dir) + strlen(inbn) + 6];                     
                                                                               
  printf("number of iterations: %d\n", iterations);                      
  for (int j=0; j < iterations; j++){                                      
      sprintf(outfn, "%sinc%d-%s", out_dir, j, inbn);                             
                                                                               
      start = time(NULL);
      struct Nifti img = read_img(in_file);                                       
      end = time(NULL);

      printf("read_start,%s,%lu,%lu,%s\n",in_file,start,getpid(),hostname);
      printf("read_end,%s,%lu,%lu,%s\n",in_file,end,getpid(),hostname);
                                                                               
      start = time(NULL);
      img.buffer = increment( img.buffer, img.img_size );      
      end = time(NULL);

      printf("increment_start,%s,%lu,%lu,%s\n",in_file,start,getpid(),hostname);
      printf("increment_end,%s,%lu,%lu,%s\n",in_file,end,getpid(),hostname);

      start = time(NULL);
      write_image( img, outfn );                                               
      end = time(NULL);

      printf("write_start,%s,%lu,%lu,%s\n",outfn,start,getpid(),hostname);
      printf("write_end,%s,%lu,%lu,%s\n",outfn,end,getpid(),hostname);


      in_file = malloc(sizeof(outfn));
      memcpy(in_file, outfn, sizeof(outfn));
  }                                                                        
                                                                               
                                                                               
  return 0;                                                                    
                                                                               
}                              

int main(int argc, char* argv[]){
    const long int ITERATIONS = strtol(argv[3], NULL, 10);
    struct AppArgs aargs = { argv[1], argv[2], ITERATIONS };
    start_app(&aargs);

    return 0;
}
