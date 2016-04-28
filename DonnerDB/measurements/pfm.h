#ifndef __PFM_H__
#define __PFM_H__

typedef struct gantry_img_header {

  // image resolution
  int width;
  int height;

  int offx;
  int offy;

  // gantry config
  double cambase;
  double camarm;
  double objbase;
  double lamparm;

} gantry_img_header_t;

void init_header (gantry_img_header_t &header);
void read_header (FILE *in, gantry_img_header_t &header);
bool read_pfm (char *fname, gantry_img_header_t &header, float *img);
float * read_pfm (char *fname, gantry_img_header_t &header);
bool write_pfm (char *fname, const gantry_img_header_t &header, float *img);

inline bool write_pfm(int w, int h, float *im, char *fname)
{
  FILE *out = fopen(fname,"wb");

  fprintf(out,"PF\n%d %d\n",w,h);
  fprintf(out,"# CAMBASE: 0.0\n");
  fprintf(out,"# CAMARM: 0.0\n");
  fprintf(out,"# OBJBASE: 0.0\n");
  fprintf(out,"# LAMPARM: 0.0\n");
  fprintf(out,"-1.000000\n");

  fwrite(im,sizeof(float),w*h*3,out);
  fclose(out);
  return true;
}

inline float * read_pfm(char *fname, int &w, int &h)
{
  // Open image file
  FILE *in = fopen(fname, "rb");
  if (!in) {
    fprintf(stderr, "Failed to open file [%s].\n",fname);
    return 0;
  }
  
  // Parse PFM header
  char buf[1024];
  fgets(buf, 1024, in);
  fgets(buf, 1024, in);
  while (buf[0] == '#')
    fgets(buf, 1024, in);
  sscanf(buf, "%d %d", &w, &h);
  if (w < 0 || h < 0) {
    fprintf(stderr, "Invalid header or image not valid PFM.\n");
    return 0;
  }
  fgets(buf, 1024, in);
  while (buf[0] == '#')
    fgets(buf, 1024, in);

  float *img = (float *)malloc(sizeof(float)*3*w*h);
  
  // Read pixel data
  if (fread(img, sizeof(float), 3*w*h, in) != 3*w*h) {
    fprintf(stderr, "Unexpected EOF while reading image data.\n");
    return false;
  }
  
  fclose(in);
  
  return img;
}

inline bool write_pfm_walpha(int offx, int offy,
			     int w, int h,
			     float *im,
			     char *fname,
			     const gantry_img_header_t &header)
{
  float *tmp=new float[w*h*3];
  for (int i=0; i<w*h; i++) {
    float a = im[i*4+3];
    for (int c=0; c<3; c++)
      tmp[i*3+c]=a*im[i*4+c];
  }

  gantry_img_header_t new_header; memcpy(&new_header,&header,sizeof(gantry_img_header_t));
  new_header.width=w;
  new_header.height=h;
  new_header.offx=offx;
  new_header.offy=offy;

  if (!write_pfm(fname,new_header,tmp)) {
    delete [] tmp;
    return false;
  } else {
    delete [] tmp;
    return true;
  }
}

inline void init_header (gantry_img_header_t &header)
{
  bzero(&header,sizeof(header));
}

inline void fprintf_header (FILE *fid, gantry_img_header_t &header)
{
  fprintf(fid,"WIDTH:    %d\n",header.width);
  fprintf(fid,"HEIGHT:   %d\n\n",header.height);
  fprintf(fid,"OFFX:     %d\n",header.offx);
  fprintf(fid,"OFFY:     %d\n\n",header.offy);
  fprintf(fid,"CAMBASE:  %.9f\n",header.cambase);
  fprintf(fid,"CAMARM:   %.9f\n",header.camarm);
  fprintf(fid,"OBJBASE:  %.9f\n",header.objbase);
  fprintf(fid,"LAMPARM:  %.9f\n",header.lamparm);
}

#endif
