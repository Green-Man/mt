#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfm.h"

inline void check_string(char *buf, const char *tag)
{
  if (strncmp(buf+2,tag,strlen(tag)==0)) {
    fprintf(stderr, "Invalid header or file not valid \"gantry\" PFM image.\n");
    fprintf(stderr, "Failed at tag: [%s]\n", tag);
    exit(1);
  }
}

inline void get_string(FILE *in, const char *tag, char *value)
{
  static char buf[1024];

  fgets(buf,1024,in);buf[1023]=0;
  check_string(buf,tag);

  // Trim trailing newline
  buf[strlen(buf)-1]=0;
  char *needle = strstr(buf,":");
  if (!needle) {
    fprintf(stderr, "Failed to parse [%s]\n", buf);
    exit(1);
  }
  // Copy past comment field
  strcpy(value,needle+1);
}

inline void get_double(FILE *in, const char *tag, double &value)
{
  static char buf[1024];

  fgets(buf,1024,in);buf[1023]=0;
  check_string(buf,tag);

  // Trim trailing newline
  buf[strlen(buf)-1]=0;
  char *needle = strstr(buf,":");
  if (!needle) {
    fprintf(stderr, "Failed to parse [%s]\n", buf);
    exit(1);
  }
  // Copy past comment field
  value = atof(needle+1);
}

inline void get_int(FILE *in, const char *tag, int &value)
{
  static char buf[1024];

  fgets(buf,1024,in);buf[1023]=0;
  check_string(buf,tag);

  // Trim trailing newline
  buf[strlen(buf)-1]=0;
  char *needle = strstr(buf,":");
  if (!needle) {
    fprintf(stderr, "Failed while parsing [%s]\n", buf);
    exit(1);
  }
  // Copy past comment field
  value = atoi(needle+1);
}

inline void put_string(FILE *in, const char *tag, const char *value)
{
  fprintf(in,"# %s:%s\n",tag,value);
}

inline void put_double(FILE *in, const char *tag, double value)
{
  fprintf(in,"# %s:%.20f\n",tag,value);
}

inline void put_int(FILE *in, const char *tag, int value)
{
  fprintf(in,"# %s:%d\n",tag,value);
}

void read_header (FILE *in, gantry_img_header_t &header)
{
  init_header(header);

  // Parse PFM header
  static char buf[1024];
  fgets(buf, 1024, in);
  fgets(buf, 1024, in);
  sscanf(buf, "%d %d", &header.width, &header.height);
  if (header.width < 0 || header.height < 0) {
    fprintf(stderr, "Invalid header or file not valid \"gantry\" PFM image.\n");
    exit(1);
  }

  get_double(in,"CAMBASE",header.cambase);
  get_double(in,"CAMARM",header.camarm);
  get_double(in,"OBJBASE",header.objbase);
  get_double(in,"LAMPARM",header.lamparm);

  fgets(buf, 1024, in);
  if (buf[0] == '#') {
    fprintf(stderr, "Invalid header or file not valid \"gantry\" PFM image.\n");
    fprintf(stderr, "Failed after last tag.\n");
    exit(1);
  }  
}

float * read_pfm (char *fname, gantry_img_header_t &header)
{
  // Open image file
  FILE *in = fopen(fname, "rb");
  if (!in) {
    fprintf(stderr, "Failed to open file [%s].\n",fname);
    return NULL;
  }

  read_header(in,header);
  
  float *img = new float[header.width*header.height*4];

  // Read pixel data
  if (fread(img, sizeof(float), 4*header.width*header.height, in) != 4*header.width*header.height) {
    fprintf(stderr, "Unexpected EOF while reading image data.\n");
    return NULL;
  }
  
  fclose(in);
  
  return img;
}

bool read_pfm (char *fname, gantry_img_header_t &header, float *img)
{
  // Open image file
  FILE *in = fopen(fname, "rb");
  if (!in) {
    fprintf(stderr, "Failed to open file [%s].\n",fname);
    return false;
  }

  read_header(in,header);
  
  // Read pixel data
  if (fread(img, sizeof(float), 3*header.width*header.height, in) != 3*header.width*header.height) {
    fprintf(stderr, "Unexpected EOF while reading image data.\n");
    return false;
  }
  
  fclose(in);
  
  return true;
}

bool write_pfm(char *fname, const gantry_img_header_t &header, float *img)
{
  // Open image file
  FILE *out = fopen(fname, "wb");
  if (!out) {
    fprintf(stderr, "Failed to open file [%s].\n",fname);
    return false;
  }

  fprintf(out,"PF\n%d %d\n",header.width,header.height);

  put_double(out,"CAMBASE",header.cambase);
  put_double(out,"CAMARM",header.camarm);
  put_double(out,"OBJBASE",header.objbase);
  put_double(out,"LAMPARM",header.lamparm);

  fprintf(out,"-1.000000\n");

  if (fwrite(img,sizeof(float),header.width*header.height*4,out) != header.width*header.height*4)
    {
      fclose(out);
      return false;
    }

  fclose(out);

  return true;
}
