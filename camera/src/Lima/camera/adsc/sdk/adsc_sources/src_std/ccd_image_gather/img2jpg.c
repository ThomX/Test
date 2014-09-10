#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "jpeglib.h"
#include "filec.h"

/* Convert ADSC .img files to grayscale jpeg format.
 *
 * Usage: img2jpeg [-quality x] [-gamma x] [-min x] [-max x] [-size x y] [-verbose] [file1.img file2.img ... ]
 *
 *	-quality x
 *      	Adjust image quality. 0 is worst, 100 is best, default is 75.
 *		Useful range is 25-95.
 *
 *	-gamma x
 *      	Adjust image contrast. Smaller is darker, larger is lighter, default is 1.0.
 *
 *	-min x
 *      	Pixels less than x are white. Normally calculated automatically.
 *
 *	-max x
 *      	Pixels larger than x are black. Normally calculated automatically.
 *
 *	-size x y
 *      	Output is x x y pixels (smaller than input size).
 *
 *	-verbose
 *      	Print more information.
 *
 * Examples:
 *
 *	img2jpeg -quality 50 < test_001.img > test_001.img.jpg
 *	img2jpeg -gamma 0.4 test*.img
 *
 *	img2jpeg -min 30 -max 400 test_003.img
 *
 */

#define ADXV_TYPE int
#define N_HIST	(65536)


static	int ihist2_data[N_HIST+1];  
static	double hist_factor = 1.0;
static	double hist_offset = 0.0;
static	double igamma = 1.0;

#ifdef _MAIN_


usage() 
{
	fprintf(stderr,"\nUsage: img2jpeg [-quality x] [-gamma x] [-min x] [-max x] [-verbose] [file1.img file2.img ... ]\n");
	fprintf(stderr,"\n\t-quality x\n");
        fprintf(stderr,"\t\tAdjust image quality. 0 is worst, 100 is best, default is 75.\n\t\tUseful range is 25-95.\n\n");
	fprintf(stderr,"\t-gamma x\n");
        fprintf(stderr,"\t\tAdjust image contrast. Smaller is darker, larger is lighter, default is 1.0.\n\n");
	fprintf(stderr,"\t-min x\n");
        fprintf(stderr,"\t\tPixels less than x are white. Normally calculated automatically.\n\n");
	fprintf(stderr,"\t-max x\n");
        fprintf(stderr,"\t\tPixels larger than x are black. Normally calculated automatically.\n\n");
	fprintf(stderr,"\t-verbose\n");
        fprintf(stderr,"\t\tPrint more information.\n\n");

	exit(-1);
}

main (argc, argv)
int argc;
char *argv[];
{
  	unsigned short *raw=NULL;
	char infile[80], outfile[80], str[80], *header=NULL;
	int dim, size[10], type=SMV_UNSIGNED_SHORT;
	int istat, hlen;
	int hist_begin, hist_end;
	int hist_begin_set=0, hist_end_set=0;
	int quality=75;
	void write_JPEG_file ();
	void ihist2();
	ADXV_TYPE *idata=NULL;
	static int prev_image_width, prev_image_height;
	JSAMPLE *image_buffer=NULL;	/* Points to large array of R,G,B-order data */
	int image_height;		/* Number of rows in image */
	int image_width;		/* Number of columns in image */
	int verbose=0;
	int out_x=0, out_y=0;
	void	convert_to_adxv(unsigned short *input, int nx, int ny, int type, ADXV_TYPE *output);
	void	convert_to_jsample(ADXV_TYPE *raw,int image_width,int image_height,unsigned char *image_buffer, int hist_begin, int hist_end);
	void	resize_image(ADXV_TYPE *idata, int iwidth, int iheight, ADXV_TYPE *odata, int owidth, int oheight);

	while (argc-- > 1) {
		if(!strcmp(*++argv, "-verbose")) {
			verbose=1;
		}
		else
		if(!strcmp(*argv, "-quality")) {
			if (argc > 1) {
				quality = (int)atof(*++argv) + 0.5;
				if (quality < 0) {
					fprintf(stderr,"Warning: quality too small, using 0\n");
					quality = 0;
				}
				if (quality > 100) {
					fprintf(stderr,"Warning: quality too large, using 100\n");
					quality = 100;
				}
				argc -= 1;
			}
			else {
				usage();
			}
		}
		else
		if(!strcmp(*argv, "-min")) {
			if (argc > 1) {
				hist_begin = (int)atof(*++argv);
				hist_begin_set = 1;
				argc -= 1;
			}
			else {
				usage();
			}
		}
		else
		if(!strcmp(*argv, "-max")) {
			if (argc > 1) {
				hist_end = (int)atof(*++argv);
				hist_end_set = 1;
				argc -= 1;
			}
			else {
				usage();
			}
		}
		else
		if(!strcmp(*argv, "-size")) {
			if (argc > 1) {
				out_x = atoi(*++argv);
				out_y = atoi(*++argv);
				argc -= 2;
			}
			else {
				usage();
			}
		}
		else
		if(!strcmp(*argv, "-gamma")) {
			if (argc > 1) {
				igamma = atof(*++argv);
				if (igamma < 0.0001) {
					fprintf(stderr,"Warning: gamma too small, using 0.0001\n");
					igamma = 0.0001;
					fflush(stderr);
				}
				igamma = 1.0 / igamma;
				argc -= 1;
			}
			else {
				usage();
			}
		}
		else
		if ((*argv)[0] == '-') {
			usage();
		}
		else
			break;
	}

	do {

	if ( argc == 0) {
		strcpy(infile,"stdin");
		strcpy(outfile,"stdout");
	}
	else {
		strcpy(infile,*argv++);
		argc--;
		strcpy(outfile,infile);
		strcat(outfile,".jpg");
	}


  	istat = rdfile (infile, &header, &hlen,
		(char**) &raw, &dim, size, &type );

	if ( istat != 0 ) {
		fprintf(stderr,"Error: reading file %s (%d)\n", infile, istat);
		continue;
	}
	/*
	else 
	if ( dim != 2 ) {
		fprintf(stderr,"Error: file dimensions are not 2 (%d)\n",dim);
		continue;
    	}
  	else
	if ( (type != SMV_UNSIGNED_SHORT) && (type != SMV_SIGNED_LONG) ) {
      		fprintf(stderr,"Error: file is neither unsigned short nor long integer (%d)\n", type);
      		continue;
    	}
	*/
	if (verbose) {
		if (type == SMV_UNSIGNED_SHORT) {
			fprintf(stderr,"Input is binary unsigned short.\n"); 
			fflush(stderr);
		}
		if (type == SMV_SIGNED_LONG) {
			fprintf(stderr,"Input is binary signed integer.\n"); 
			fflush(stderr);
		}
	}

	/*
	image_width = size[0];
	image_height = size[1];
	*/
  	gethd ("SIZE1", str, header);
  	sscanf (str, "%d", &image_width);
  	gethd ("SIZE2", str, header);
  	sscanf (str, "%d", &image_height);

	fprintf(stderr,"%s (%d x %d pixels) --> %s\n",infile,image_width,image_height,outfile);

	if ((image_width > prev_image_width) || (image_height > prev_image_height)) {
		if (idata != NULL) 
			free(idata);
		if (image_buffer != NULL) 
			free(image_buffer);
  		image_buffer = (JSAMPLE *)malloc (image_width * image_height * sizeof ( JSAMPLE ));
		if (image_buffer == NULL) {
			fprintf(stderr,"Error: could not allocate %d bytes\n");
			fflush(stderr);
			exit(-1);
		}
  		idata = (ADXV_TYPE *)malloc (image_width * image_height * sizeof ( ADXV_TYPE ));
		if (idata == NULL) {
			fprintf(stderr,"Error: could not allocate %d bytes\n");
			fflush(stderr);
			exit(-1);
		}
		prev_image_width = image_width;
		prev_image_height = image_height;
	}

	convert_to_adxv(raw,image_width,image_height,type,idata);

	ihist2 (idata, image_width, image_height, 0, 0, image_width-1, image_height-1, ihist2_data, N_HIST);

	if (hist_begin_set == 0)
		hist_begin = get_hist_begin(ihist2_data, N_HIST);
	if (hist_end_set == 0)
		hist_end  = get_hist_end(ihist2_data, N_HIST,0.0008);

	if (verbose == 1) {
		fprintf(stderr,"Quality: %d, Gamma: %1.2f, Min: %d Max: %d\n",
			quality,1.0/igamma,hist_begin,hist_end); fflush(stderr);
	}

	/* Resize output */
	if (out_x != 0) {
		resize_image(idata, image_width, image_height, idata, out_x, out_y);
		image_width = out_x;
		image_height = out_y;
	}

	convert_to_jsample(idata,image_width,image_height,image_buffer, hist_begin, hist_end);

	write_JPEG_file (outfile, image_width, image_height, quality, image_buffer);

	if (raw != NULL)
		free (raw);
	if (header != NULL)
		free (header);

	} while (argc > 0);
}

#endif /* _MAIN_ */

int	img2jpg_sub(unsigned short *raw, int size_x, int size_y, int out_x, int out_y, char *outfile)
{
	int type=SMV_UNSIGNED_SHORT;
	int hist_begin, hist_end;
	int hist_begin_set=0, hist_end_set=0;
	int quality=25;
	void write_JPEG_file ();
	void ihist2();
	ADXV_TYPE *idata=NULL;
	JSAMPLE *image_buffer=NULL;	/* Points to large array of R,G,B-order data */
	int image_height;		/* Number of rows in image */
	int image_width;		/* Number of columns in image */
	int verbose=0;
	void	convert_to_adxv(unsigned short *input, int nx, int ny, int type, ADXV_TYPE *output);
	void	convert_to_jsample(ADXV_TYPE *raw,int image_width,int image_height,unsigned char *image_buffer, int hist_begin, int hist_end);
	void	resize_image(ADXV_TYPE *idata, int iwidth, int iheight, ADXV_TYPE *odata, int owidth, int oheight);
	int get_hist_begin(int *data, int n_hist);
	int get_hist_end(int *data, int n_hist, double hist_end);

	image_width = size_x;
	image_height = size_y;

	if(1 == verbose)
	fprintf(stderr,"img file (%d x %d pixels) --> jpg file: %s (%d x %d pixels)\n",image_width,image_height,outfile, out_x, out_y);

  	image_buffer = (JSAMPLE *)malloc (image_width * image_height * sizeof ( JSAMPLE ));
	if (image_buffer == NULL) {
		fprintf(stderr,"Error: could not allocate %d bytes\n", image_width * image_height * sizeof ( JSAMPLE ));
		fflush(stderr);
		return(-1);
	}
  	idata = (ADXV_TYPE *)malloc (image_width * image_height * sizeof ( ADXV_TYPE ));
	if (idata == NULL) {
		fprintf(stderr,"Error: could not allocate %d bytes\n", image_width * image_height * sizeof ( ADXV_TYPE ));
		fflush(stderr);
		free(image_buffer);
		return(-1);
	}

	convert_to_adxv(raw,image_width,image_height,type,idata);

	ihist2 (idata, image_width, image_height, 0, 0, image_width-1, image_height-1, ihist2_data, N_HIST);

	if (hist_begin_set == 0)
		hist_begin = get_hist_begin(ihist2_data, N_HIST);
	if (hist_end_set == 0)
		hist_end  = get_hist_end(ihist2_data, N_HIST,0.0008);

	if (1 == verbose) {
		fprintf(stderr,"Quality: %d, Gamma: %1.2f, Min: %d Max: %d\n",
			quality,1.0/igamma,hist_begin,hist_end); fflush(stderr);
	}

	/* Resize output */
	if (out_x != 0) {
		resize_image(idata, image_width, image_height, idata, out_x, out_y);
	}

	convert_to_jsample(idata,out_x,out_y,image_buffer, hist_begin, hist_end);

	write_JPEG_file (outfile, out_x, out_x, quality, image_buffer);

	free(image_buffer);
	free(idata);
	return(0);
}


/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */

void
write_JPEG_file (filename, image_width, image_height, quality, image_buffer)
char *filename;
int  image_width, image_height, quality;
JSAMPLE *image_buffer;	
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */

  if (!strcmp(filename,"stdout")) {
   	outfile = stdout;
  }
  else {
   	if ((outfile = fopen(filename, "wb")) == NULL) {
    		fprintf(stderr, "Error: can't open %s for writing.\n", filename);
    		return;
   	}
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 1;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image, was JCS_RGB */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * 1;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}


/*
 * SOME FINE POINTS:
 *
 * In the above loop, we ignored the return value of jpeg_write_scanlines,
 * which is the number of scanlines actually written.  We could get away
 * with this because we were only relying on the value of cinfo.next_scanline,
 * which will be incremented correctly.  If you maintain additional loop
 * variables then you should be careful to increment them properly.
 * Actually, for output to a stdio stream you needn't worry, because
 * then jpeg_write_scanlines will write all the lines passed (or else exit
 * with a fatal error).  Partial writes can only occur if you use a data
 * destination module that can demand suspension of the compressor.
 * (If you don't know what that's for, you don't need it.)
 *
 * If the compressor requires full-image buffers (for entropy-coding
 * optimization or a multi-scan JPEG file), it will create temporary
 * files for anything that doesn't fit within the maximum-memory setting.
 * (Note that temp files are NOT needed if you use the default parameters.)
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.doc.
 *
 * Scanlines MUST be supplied in top-to-bottom order if you want your JPEG
 * files to be compatible with everyone else's.  If you cannot readily read
 * your data in that order, you'll need an intermediate array to hold the
 * image.  See rdtarga.c or rdbmp.c for examples of handling bottom-to-top
 * source data using the JPEG code's internal virtual-array mechanisms.
 */



/* Convert SMV data (unsigned short or integer) to ADXV_TYPE format
 */

void	convert_to_adxv(input,nx,ny,type,output)
unsigned short *input;
int nx,ny;
int type; /* type of input data */
ADXV_TYPE *output;
{
	int i;
	int *iptr;

	if ( type == SMV_UNSIGNED_SHORT ) {
		for(i=0;i<nx*ny;i++)
			output[i] = input[i];
	}
	else
	if ( type == SMV_SIGNED_LONG ) {
		iptr = (int *)input;
		for(i=0;i<nx*ny;i++)
			output[i] = iptr[i];
	}
	else {
		fprintf(stderr,"Error: unknown input type: %d\n",type);
		for(i=0;i<nx*ny;i++)
			output[i] = input[i];
	}
}

/* Convert input raw unsigned short binary to unsigned char's
 */

void	convert_to_jsample(ADXV_TYPE *raw,int image_width,int image_height,unsigned char *image_buffer, int hist_begin, int hist_end)
{
	int i;
	int maxpix= -1;
	int pixel;
	double hscale = (hist_end - hist_begin)/255.0;
	double gscale = 255.0 / pow (255.0, igamma);

	/* If data is less than hist_begin it is 255 (white) and
	 * if data is larger than hist_end it is 0 (black) */

	for(i=0;i<image_width*image_height;i++) {
		if (raw[i] > maxpix)
			maxpix = raw[i];
	}

	for(i=0;i<image_width*image_height;i++) {
		pixel = raw[i];
		if (pixel < hist_begin)
			image_buffer[i] = 255;
		else
		if (pixel > hist_end)
			image_buffer[i] = 0;
		else {
			if (igamma != 1.0) {
				pixel = gscale * pow ((hist_end - pixel)/hscale + 0.5, igamma) + 0.5;
				if (pixel < 0) 
					image_buffer[i] = 0;
				else
				if (pixel > 255) 
					image_buffer[i] = 255;
				else
					image_buffer[i] = pixel;
			}
			else
				image_buffer[i] = (hist_end - pixel)/hscale + 0.5;
		}
	}
}


#define HIST_BEG (0.01)
#define HIST_END (0.00001)

/* Return value below which 1% of
 * histogram data is.
 */
int get_hist_begin(int *data, int n_hist)
{
	register int i, imax=4;
	register int sum=0, sum_all=0;

	imax=0;
	for(i=30;i<n_hist;i++) {
		if (data[i] > imax)
			imax = data[i];
	}
	imax = 4;

	for(i=1;i<n_hist;i++) {
		if (data[i] > imax)
			sum_all += data[i];
	}

	if (sum_all <= 500) {
		imax = 0;
		for(i=1;i<n_hist;i++) {
			if (data[i] > imax)
				sum_all += data[i];
		}
		if (sum_all <= 0)
			return(1);
	}

	for(i=1;i<n_hist;i++) {
		if (data[i] > imax) {
			sum += data[i];
			if ((float)sum/sum_all >= HIST_BEG) {
				if ((hist_factor != 1.0) || (hist_offset != 0.0))
					return((int)(i / hist_factor - hist_offset + 0.5));
				else
					return(i);
			}
		}
	}
	return(1);
}

/* Return value below which 99% of
 * histogram data is.
 */
int get_hist_end(int *data, int n_hist, double hist_end)
{
	register int i;
	register int sum=0, sum_all=0;
	int imax=0;

	if (n_hist > 65500)
		n_hist = 65500;

	for(i=30;i<n_hist;i++) {
		if (data[i] > imax)
			imax = data[i];
	}
	imax /= 1000;
	if (imax < 4)
		imax = 4;

	for(i=1;i<n_hist;i++) {
		if (data[i] > imax)
			sum_all += data[i];
	}


	if (sum_all <= 500) {
		sum_all = 0;
		imax = 0;
		for(i=1;i<n_hist;i++) {
			if (data[i] > imax)
				sum_all += data[i];
		}
		if (sum_all <= 0)
			return(10);
	}


	/* Magnify Window */
	if (hist_end < 0) {
		hist_end = 0;
		imax /= 2;
	}

	for(i=n_hist;--i;) {
		if (data[i] > imax) {
			sum += data[i];
			if ((double)sum/sum_all >= hist_end)
			{
				if ((hist_factor != 1.0) || (hist_offset != 0.0))
					return((int)(i / hist_factor - hist_offset + 0.5));
				else
					return(i);
			}
		}
	}
	return(100);
}

void
ihist2 (idata, nx, ny, x1, y1, x2, y2, odata, hist_max)
register ADXV_TYPE *idata;
int nx, ny, x1, y1, x2, y2;
register int *odata;
register int hist_max;
{
	register int index, i, j;

	if (x2 < x1) {
		j = x1;
		x1 = x2;
		x2 = j;
	}
	if (x1 < 0) x1 = 0;
	if (x2 < 0) x2 = 0;
	if (x1 >= nx) x1 = nx-1;
	if (x2 >= nx) x2 = nx-1;

	if (y2 < y1) {
		j = y1;
		y1 = y2;
		y2 = j;
	}
	if (y1 < 0) y1 = 0;
	if (y2 < 0) y2 = 0;
	if (y1 >= ny) y1 = ny-1;
	if (y2 >= ny) y2 = ny-1;
		

	if (odata == NULL)  {
		fprintf(stderr,"Error: odata = NULL...\n");
		return;
	}
	if (idata == NULL)  {
		fprintf(stderr,"Error: idata = NULL...\n");
		return;
	}

	for(i=0;i<=hist_max;i++) 
		odata[i] = 0;

	if ((hist_factor == 1.0) && (hist_offset == 0.0)) {
		for(j=y1;j<=y2;j++) {
		for(i=x1;i<=x2;i++) {
			index = idata[j*nx + i];
			if ((index <= hist_max) && (index >= 0))
				odata[index]++;
		}
		}
	}
	else {
		for(j=y1;j<=y2;j++) {
		for(i=x1;i<=x2;i++) {
			index = (idata[j*nx + i] + hist_offset) * hist_factor + 0.5;
			if ((index <= hist_max) && (index >= 0))
				odata[index]++;
		}
		}
	}
}

/* Input in idata[] is iwidth x iheight pixels. This is
 * resized to owidth x oheight and stored in odata[]
 */
void	resize_image(ADXV_TYPE *idata, int iwidth, int iheight, ADXV_TYPE *odata, int owidth, int oheight)
{
	register int i,j,k,l;
	int binx, biny;
	int count;
	int index_y, index_x;
	double pixel;

#ifdef NTH_PIXEL
	/* Just take every n'th pixel
	 */
	for(j=0;j<oheight;j++)
	for(i=0;i<owidth;i++) {
		index_y = j*iheight/oheight;
		index_x = i*iwidth/owidth;
		odata[j*owidth+i] = idata[index_y * iwidth + index_x];
	}
#else

	/* Calculate the average binned value for each pixel
	 */
	binx = (iwidth/owidth)/2;
	biny = (iheight/oheight)/2;
	for(j=0;j<oheight;j++)
	for(i=0;i<owidth;i++) {
		pixel = count = 0;
		for(k= -binx ; k <= binx; k++) {
		for(l= -binx ; l <= binx; l++) {
			index_y = ((j*iheight/oheight) + l);
			index_x = (i*iwidth/owidth + k);
			if ((index_y >= 0) && (index_y < iheight) &&
			    (index_x >= 0) && (index_x < iwidth)) {
				pixel += idata[index_y * iwidth + index_x ];
				count++;
			}
		}
		}
		if (count > 0)
			odata[j*owidth+i] = pixel/count + 0.5;
		else
			odata[j*owidth+i] = 0;
	}
#endif /* NTH_PIXEL */

}
