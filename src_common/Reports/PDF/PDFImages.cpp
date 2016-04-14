
#include "compiler.h"

#include "PDFImages.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define	USEJPEG
//#define USEPNG

#include <ctype.h>		/* to declare isprint() */
#define JPEG_CJPEG_DJPEG	/* define proper options in jconfig.h */
#define JPEG_INTERNAL_OPTIONS	/* cjpeg.c,djpeg.c need to see xxx_SUPPORTED */
#include "jpeglib.h"

typedef struct djpeg_dest_struct * djpeg_dest_ptr;

struct djpeg_dest_struct {
  /* start_output is called after jpeg_start_decompress finishes.
   * The color map will be ready at this time, if one is needed.
   */
  JMETHOD(void, start_output, (j_decompress_ptr cinfo,
			       djpeg_dest_ptr dinfo));
  /* Emit the specified number of pixel rows from the buffer. */
  JMETHOD(void, put_pixel_rows, (j_decompress_ptr cinfo,
				 djpeg_dest_ptr dinfo,
				 JDIMENSION rows_supplied));
  /* Finish up at the end of the image. */
  JMETHOD(void, finish_output, (j_decompress_ptr cinfo,
				djpeg_dest_ptr dinfo));

  /* Target file spec; filled in by djpeg.c after object is created. */
  FILE * output_file;

  /* Output pixel-row buffer.  Created by module init or start_output.
   * Width is cinfo->output_width * cinfo->output_components;
   * height is buffer_height.
   */
  JSAMPARRAY buffer;
  JDIMENSION buffer_height;
};


/*
 * cjpeg/djpeg may need to perform extra passes to convert to or from
 * the source/destination file format.  The JPEG library does not know
 * about these passes, but we'd like them to be counted by the progress
 * monitor.  We use an expanded progress monitor object to hold the
 * additional pass count.
 */

struct cdjpeg_progress_mgr {
  struct jpeg_progress_mgr pub;	/* fields known to JPEG library */
  int completed_extra_passes;	/* extra passes completed */
  int total_extra_passes;	/* total extra */
  /* last printed percentage stored here to avoid multiple printouts */
  int percent_done;
};

typedef struct cdjpeg_progress_mgr * cd_progress_ptr;

#ifdef DONT_USE_B_MODE		/* define mode parameters for fopen() */
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif


#ifdef BMP_SUPPORTED


/*
 * To support 12-bit JPEG data, we'd have to scale output down to 8 bits.
 * This is not yet implemented.
 */

#if BITS_IN_JSAMPLE != 8
  Sorry, this code only copes with 8-bit JSAMPLEs. /* deliberate syntax err */
#endif

/*
 * Since BMP stores scanlines bottom-to-top, we have to invert the image
 * from JPEG's top-to-bottom order.  To do this, we save the outgoing data
 * in a virtual array during put_pixel_row calls, then actually emit the
 * BMP file during finish_output.  The virtual array contains one JSAMPLE per
 * pixel if the output is grayscale or colormapped, three if it is full color.
 */

/* Private version of data destination object */

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  boolean is_os2;		/* saves the OS2 format request flag */

  jvirt_sarray_ptr whole_image;	/* needed to reverse row order */
  JDIMENSION data_width;	/* JSAMPLEs per row */
  JDIMENSION row_width;		/* physical width of one row in the BMP file */
  int pad_bytes;		/* number of padding bytes needed per row */
  JDIMENSION cur_output_row;	/* next row# to write to virtual array */
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;


/* Forward declarations */
LOCAL(void) write_colormap
	JPP((j_decompress_ptr cinfo, bmp_dest_ptr dest,
	     int map_colors, int map_entry_size));


/*
 * Write some pixel data.
 * In this module rows_supplied will always be 1.
 */

METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
/* This version is for writing 24-bit pixels */
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

  /* Access next row in virtual array */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

  /* Transfer data.  Note destination values must be in BGR order
   * (even though Microsoft's own documents say the opposite).
   */
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    outptr[2] = *inptr++;	/* can omit GETJSAMPLE() safely */
    outptr[1] = *inptr++;
    outptr[0] = *inptr++;
    outptr += 3;
  }

  /* Zero out the pad bytes. */
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}

METHODDEF(void)
put_gray_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
	       JDIMENSION rows_supplied)
/* This version is for grayscale OR quantized color output */
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

  /* Access next row in virtual array */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

  /* Transfer data. */
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    *outptr++ = *inptr++;	/* can omit GETJSAMPLE() safely */
  }

  /* Zero out the pad bytes. */
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}


/*
 * Startup: normally writes the file header.
 * In this module we may as well postpone everything until finish_output.
 */

METHODDEF(void)
start_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /* no work here */
}

extern char *imageDataBuf;
extern int imageDataBufLen;

METHODDEF(void)
finish_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  //register FILE * outfile = dest->pub.output_file;
  JSAMPARRAY image_ptr;
  register JSAMPROW data_ptr;
  JDIMENSION row;
  register JDIMENSION col;
//  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

  for (row = 0; row < cinfo->output_height; row++) {
    image_ptr = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, dest->whole_image, row, (JDIMENSION) 1, FALSE);
    data_ptr = image_ptr[0];
    for (col = dest->data_width; col > 0; col-=3) {
			*imageDataBuf = (char)GETJSAMPLE(*(data_ptr+2));
			imageDataBuf++;
			*imageDataBuf = (char)GETJSAMPLE(*(data_ptr+1));
			imageDataBuf++;
			*imageDataBuf = (char)GETJSAMPLE(*(data_ptr));
			imageDataBuf++;
			imageDataBufLen+=3;
      data_ptr+=3;
    }
  }

//  if (progress != NULL)
//    progress->completed_extra_passes++;

}


/*
 * The module selection routine for BMP format output.
 */

GLOBAL(djpeg_dest_ptr)
jinit_write_bmp (j_decompress_ptr cinfo, boolean is_os2)
{
  bmp_dest_ptr dest;
  JDIMENSION row_width;

  /* Create module interface object, fill in method pointers */
  dest = (bmp_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  sizeof(bmp_dest_struct));
  dest->pub.start_output = start_output_bmp;
  dest->pub.finish_output = finish_output_bmp;
  dest->is_os2 = is_os2;

  if (cinfo->out_color_space == JCS_GRAYSCALE) {
    dest->pub.put_pixel_rows = put_gray_rows;
  } else if (cinfo->out_color_space == JCS_RGB) {
    if (cinfo->quantize_colors)
      dest->pub.put_pixel_rows = put_gray_rows;
    else
      dest->pub.put_pixel_rows = put_pixel_rows;
  } else {
	  return NULL;
//    ERREXIT(cinfo, JERR_BMP_COLORSPACE);
  }

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Determine width of rows in the BMP file (padded to 4-byte boundary). */
  row_width = cinfo->output_width * cinfo->output_components;
  dest->data_width = row_width;
  while ((row_width & 3) != 0) row_width++;
  dest->row_width = row_width;
  dest->pad_bytes = (int) (row_width - dest->data_width);

  /* Allocate space for inversion array, prepare for write pass */
  dest->whole_image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     row_width, cinfo->output_height, (JDIMENSION) 1);
  dest->cur_output_row = 0;
/*  if (cinfo->progress != NULL) {
    cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
    progress->total_extra_passes++; /* count file input as separate pass */
//  }

  /* Create decompressor output buffer. */
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, row_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}

#endif /* BMP_SUPPORTED */



/* Create the add-on message string table. */

#define JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = {
//#include "cderror.h"
  NULL
};


/*
 * This list defines the known output image formats
 * (not all of which need be supported by a given version).
 * You can change the default output format by defining DEFAULT_FMT;
 * indeed, you had better do so if you undefine PPM_SUPPORTED.
 */

typedef enum {
	FMT_BMP		/* BMP format (Windows flavor) */
} IMAGE_FORMATS;

#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_BMP
//#define DEFAULT_FMT	FMT_PPM
#endif

static IMAGE_FORMATS requested_fmt;


/*
 * Argument-parsing code.
 * The switch parser is designed to be useful with DOS-style command line
 * syntax, ie, intermixed switches and file names, where only the switches
 * to the left of a given file name affect processing of that file.
 * The main program in this file doesn't actually use this capability...
 */


static const char * progname;	/* program name for error messages */
static char * outfilename;	/* for -outfile switch */

/*
 * Marker processor for COM and interesting APPn markers.
 * This replaces the library's built-in processor, which just skips the marker.
 * We want to print out the marker as text, to the extent possible.
 * Note this code relies on a non-suspending data source.
 */

LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
/* Read next byte */
{
  struct jpeg_source_mgr * datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer) (cinfo))
      //ERREXIT(cinfo, JERR_CANT_SUSPEND);
	  return GETJOCTET(*datasrc->next_input_byte++);
  }
  datasrc->bytes_in_buffer--;
  return GETJOCTET(*datasrc->next_input_byte++);
}


METHODDEF(boolean)
print_text_marker (j_decompress_ptr cinfo)
{
  boolean traceit = (cinfo->err->trace_level >= 1);
  INT32 length;
  unsigned int ch;
  unsigned int lastch = 0;

  length = jpeg_getc(cinfo) << 8;
  length += jpeg_getc(cinfo);
  length -= 2;			/* discount the length word itself */

  if (traceit) {
    if (cinfo->unread_marker == JPEG_COM)
      fprintf(stderr, "Comment, length %ld:\n", (long) length);
    else			/* assume it is an APPn otherwise */
      fprintf(stderr, "APP%d, length %ld:\n",
	      cinfo->unread_marker - JPEG_APP0, (long) length);
  }

  while (--length >= 0) {
    ch = jpeg_getc(cinfo);
    if (traceit) {
      /* Emit the character in a readable form.
       * Nonprintables are converted to \nnn form,
       * while \ is converted to \\.
       * Newlines in CR, CR/LF, or LF form will be printed as one newline.
       */
      if (ch == '\r') {
	fprintf(stderr, "\n");
      } else if (ch == '\n') {
	if (lastch != '\r')
	  fprintf(stderr, "\n");
      } else if (ch == '\\') {
	fprintf(stderr, "\\\\");
      } else if (isprint(ch)) {
	putc(ch, stderr);
      } else {
	fprintf(stderr, "\\%03o", ch);
      }
      lastch = ch;
    }
  }

  if (traceit)
    fprintf(stderr, "\n");

  return TRUE;
}


/*
 * The main program.
 */

char *imageDataBuf;
int imageDataBufLen;
PDFImageJpeg *pdfImageJpeg = NULL;

int
CreatePDFImageFromFile(int argc, const char **argv, PDFImageJpeg *pdfImageJpegP )
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
/*#ifdef PROGRESS_REPORT
  struct cdjpeg_progress_mgr progress;
#endif*/
  int file_index;
  djpeg_dest_ptr dest_mgr = NULL;
  FILE * input_file;
//  FILE * output_file;
  JDIMENSION num_scanlines;

/*
  // On Mac, fetch a command line.
#ifdef USE_CCOMMAND
  argc = ccommand(&argv);
#endif
*/

  progname = argv[0];
  if (progname == NULL || progname[0] == 0)
    progname = "djpeg";		/* in case C library doesn't provide it */

  /* Initialize the JPEG decompression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  /* Add some application-specific error messages (from cderror.h) */
  jerr.addon_message_table = cdjpeg_message_table;
//  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
//  jerr.last_addon_message = JMSG_LASTADDONCODE;

  /* Insert custom marker processor for COM and APP12.
   * APP12 is used by some digital camera makers for textual info,
   * so we provide the ability to display it as text.
   * If you like, additional APPn marker types can be selected for display,
   * but don't try to override APP0 or APP14 this way (see libjpeg.doc).
   */
  jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
  jpeg_set_marker_processor(&cinfo, JPEG_APP0+12, print_text_marker);

  /* Now safe to enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  enable_signal_catcher((j_common_ptr) &cinfo);
#endif

  /* Scan command line to find file names. */
  /* It is convenient to use just one switch-parsing routine, but the switch
   * values read here are ignored; we will rescan the switches after opening
   * the input file.
   * (Exception: tracing level set here controls verbosity for COM markers
   * found during jpeg_read_header...)
   */

  //file_index = parse_switches(&cinfo, argc, argv, 0, FALSE);

  file_index = 1;

//#ifdef TWO_FILE_COMMANDLINE
  /* Must have either -outfile switch or explicit output file name */
/*  if (outfilename == NULL) {
    if (file_index != argc-2)
	{
		return -1;
//      fprintf(stderr, "%s: must name one input and one output file\n",
//	      progname);
//      usage();
    }
    outfilename = argv[file_index+1];
  } else
  {
    if (file_index != argc-1)
	{
		return -1;
//      fprintf(stderr, "%s: must name one input and one output file\n",
//	      progname);
//      usage();
    }
  }*/
//#else
  /* Unix style: expect zero or one file name */
/*  if (file_index < argc-1) {
    fprintf(stderr, "%s: only one input file\n", progname);
    usage();
  }*/
//#endif /* TWO_FILE_COMMANDLINE */

  /* Open the input file. */
	if (file_index < argc)
	{
		if ( argv[file_index][0] != 0 )
		{
			if ((input_file = fopen(argv[file_index], READ_BINARY)) == NULL)
			{
				return -1;
			}
		}
		else
			return -1;
	}
	else
		return -1;

/*#ifdef PROGRESS_REPORT
  start_progress_monitor((j_common_ptr) &cinfo, &progress);
#endif*/

  /* Specify data source for decompression */
  jpeg_stdio_src(&cinfo, input_file);

  // Test to see if the file is a jpeg or not...]
  char jpegFile[10];
  fpos_t pos;
  fgetpos( input_file, &pos );
  fread( jpegFile, 10, 1, input_file );
  fsetpos( input_file, &pos );

  //if ( jpegFile[6] != 'J' || jpegFile[7] != 'F' || jpegFile[8] != 'I' || jpegFile[9] != 'F' )
  //{
//	  return -1;
  //}

  /* Read file header, set default decompression parameters */
  (void) jpeg_read_header(&cinfo, TRUE);

  /* Initialize the output module now to let it override any crucial
   * option settings (for instance, GIF wants to force color quantization).
   */
  switch (requested_fmt) {
#ifdef BMP_SUPPORTED
  case FMT_BMP:
    dest_mgr = jinit_write_bmp(&cinfo, FALSE);
	if ( !dest_mgr )
		return -1;
    break;
#endif
	  return -1;
    break;
  }

  /* Start decompressor */
  (void) jpeg_start_decompress(&cinfo);

  imageDataBufLen = 0;
  pdfImageJpeg = pdfImageJpegP;
  int memReqLen = cinfo.image_width * cinfo.image_height * cinfo.num_components + 32;
  imageDataBuf = new char[memReqLen];
//  memset( imageDataBuf, 0xF9, memReqLen );
  char *startBuf;
  startBuf = imageDataBuf;

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height)
  {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }

//#ifdef PROGRESS_REPORT
  /* Hack: count final pass as done in case finish_output does an extra pass.
   * The library won't have updated completed_passes.
   */
//  progress.pub.completed_passes = progress.pub.total_passes;
//#endif

  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  (*dest_mgr->finish_output) (&cinfo, dest_mgr);
  pdfImageJpeg->SetImageData( startBuf, imageDataBufLen, cinfo.image_width, cinfo.image_height, 8);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  /* Close files, if we opened them */
  if (input_file != stdin)
    fclose(input_file);

/*#ifdef PROGRESS_REPORT
  end_progress_monitor((j_common_ptr) &cinfo);
#endif*/

  /* All done. */
  return 0;			/* suppress no-return-value warnings */
}

#ifdef __cplusplus
}
#endif
