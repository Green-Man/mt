#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <math.h>

#include "pfm.h"

#define IR 10

void show_usage(const char *myname)
{
  fprintf(stderr, "Usage: %s dir #cx #cy #r #theta_s basename\n", myname);
  fprintf(stderr, "   cx,cy:     (ints) pixel location of the center of the source (see readme.txt)\n");
  fprintf(stderr, "   r,theta_s: (floats) outgoing surface location in polar coordinates w.r.t. source\n");
  fprintf(stderr, "   basename:  (string) directory containing image data\n");
  exit(1);
}

int
main(int argc, char **argv)
{
  if (argc < 7)
    show_usage(argv[0]);

  int cx = atoi(argv[2]);
  int cy = atoi(argv[3]);

  float r = atof(argv[4]);
  float th = atof(argv[5])*M_PI/180.0;

#ifdef WAX
  float pelspermm = 51.0f/14.0f; // ~3.6
#else
  float pelspermm = 38.0f/12.0f; // ~3.2
#endif

  // convert to pixels
  r = r * pelspermm;

  int x = cx + (int)(r*cos(th)+0.5);
  int y = cy - (int)(r*sin(th)+0.5);

  glob_t globs;

  char imgdir[1000];
  sprintf(imgdir,"%s/img*_flipped_warp.pfm", argv[1]);

  if (0 != glob(imgdir,GLOB_ERR,NULL,(glob_t *)&globs))
    {
      fprintf(stderr, "failed to find any images in [%s]\n", imgdir);
      return -1;
    }

  {
  FILE *out = fopen("scat.txt","wt");
  
  gantry_img_header_t header;

  for (int i=0; i<globs.gl_pathc; i++)
    {

      char fname[1024];
      strcpy(fname,globs.gl_pathv[i]);

      float *im = read_pfm(fname,header);

      float resp  = 0.0f;
      float norm  = 0.0f;
      float sigma = 3.0;
      for (int dy=-IR; dy<=IR; dy++)
	{
	  for (int dx=-IR; dx<=IR; dx++)
	    {
	      float w = exp(-(dx*dx+dy*dy)/(sigma*sigma));
	      resp += w * im[(y+dy)*header.width*4+(x+dx)*4+0];
	      norm += w;
	    }
	}
      resp /= norm;

      if (resp < 1e-2) continue;

      fprintf(out,"%f %f\n",header.camarm,resp); fflush(stdout);

      delete [] im;
    }
  fclose(out);
  }

  {
  FILE *out = fopen("scat.plt","wt");
  fprintf(out, "set terminal postscript eps color solid \"Helvetica\" 20\n");
  fprintf(out, "set output \"%s.eps\"\n",argv[6]);
  fprintf(out, "set xlabel \"camera angle (degrees)\" font \"Helvetica,29\"\n");
  fprintf(out, "set ylabel \"measured response\" font \"Helvetica,29\"\n");
#ifdef WAX
  fprintf(out, "set title \"wax (ts=20,r=%s,tr=%s)\" font \"Helvetica,27\"\n",argv[4],argv[5]);
#else
  fprintf(out, "set title \"diluted skim milk (ts=20,r=%s,tr=%s)\" font \"Helvetica,27\"\n",argv[4],argv[5]);
#endif
  fprintf(out, "set xrange [-90:90]\n");
  fprintf(out, "set yrange [0:2]\n");
  fprintf(out, "set xtics 10\n");
  fprintf(out, "set ytics 0.25\n");
  fprintf(out, "set grid lt 9 lw 0.5\n");
  fprintf(out, "set size 1,1\n");
  fprintf(out, "unset key\n");
  fprintf(out, "set da sty li\n");
  fprintf(out, "plot \"scat.txt\" using 1:2 notitle with linespoints lw 4 ps 1\n");
  fclose(out);
  }

  {
  char cmd[1024];
  sprintf(cmd,"gnuplot scat.plt");
  system(cmd);
  sprintf(cmd,"ps2pdf %s.eps",argv[6]);
  system(cmd);
  }
    
  return 0;
}
