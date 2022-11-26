/*
 *                 D3X(tm) DOS-Extender v0.90 unleash `h' alpha
 *                   utility for shrinking executable headers
 *                                      *
 *                      Copyright (c) 1998-2004 Daniel Borca
 *                                      *
 *                               dborca@yahoo.com
 *                       http://www.geocities.com/dborca
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stubs.h"


#ifdef __GNUC__
#define NORET __attribute__((noreturn))
#else
#define NORET
#endif

typedef enum {
   OK          = 0,
   ERR_SYNTAX  = 1,
   ERR_INPUT   = 2,
   ERR_CREATE  = 0x10,
   ERR_OPEN    = 0x11,
   ERR_READ    = 0x12,
   ERR_WRITE   = 0x13,
   ERR_MEM     = 0x20,
   ERR_UNKNOWN = 0x40,
   ERR_UNSUPP  = 0x41,
   ERR_RELOC   = 0x42,
   ERR_STACK   = 0x43,
   ERR_FPARA   = 0x44
} ERR;


static char shameless0[] = "\r\nD3X(c)dborca\r\n";
static char shameless1[] = "\r\nD3X v0.90.h (c) Daniel Borca\r\n";
static char shameless2[] = "\r\n"
"D3X v0.90.h Copyright (C) 2004 Daniel Borca (dborca 'at' yahoo 'dot' com)\r\n"
"Permission granted to use for any purpose provided this copyright remains\r\n"
"present and unmodified. This clause only applies to the extender, and not\r\n"
"necessarily the whole program.\r\n";

static char *myself;


#ifdef DJGPP
char **__crt0_glob_function (char *_argument)         { (void)_argument; return 0; }
void   __crt0_load_environment_file (char *_app_name) { (void)_app_name; }
#endif

static int fix (char *infile, int array, int page, int newformat);


/* usage:
 *  print mini-help
 */
static void NORET
usage (void)
{
   printf(
"Usage: %s [-e] [-p] [-n] executable\n"
"\n"
"   -e  generate C array (stdout)\n"
"   -p  make output page aligned\n"
"   -n  new format output header\n"
, myself);

   exit(OK);
}


/* syntaxerr:
 *  bail with `syntax error'
 */
static void NORET
syntaxerr (void)
{
   printf("Error 01: syntax error. Try %s -h\n", myself);

   exit(ERR_SYNTAX);
}


/* main:
 *
 */
int
main (int argc, char **argv)
{
   int i;
   char *infile = NULL;
   int array = 0, page = 0, newformat = 0;
   char *error;

   myself = argv[0];

   if (argc < 2) {
      syntaxerr();
   }

   for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-h")) {
         if (argc == 2) {
            usage();
         } else {
            syntaxerr();
         }
      }
      if (!strcmp(argv[i], "-e")) {
         array = 1;
         continue;
      }
      if (!strcmp(argv[i], "-p")) {
         page = 1;
         continue;
      }
      if (!strcmp(argv[i], "-n")) {
         newformat = 1;
         continue;
      }
      if (!infile) {
         infile = argv[i];
      } else {
         syntaxerr();
      }
   }

   switch (i = fix(infile, array, page, newformat)) {
      case ERR_INPUT:
         error = "no input file\n";
         break;
      case ERR_CREATE:
         error = "unable to create (%s)\n";
         break;
      case ERR_OPEN:
         error = "unable to open (%s)\n";
         break;
      case ERR_READ:
         error = "unable to read (%s)\n";
         break;
      case ERR_WRITE:
         error = "unable to write (%s)\n";
         break;
      case ERR_MEM:
         error = "not enough memory\n";
         break;
      case ERR_UNKNOWN:
         error = "invalid EXE (%s)\n";
         break;
      case ERR_FPARA:
         error = "image not para aligned (%s)\n";
         break;
      default:
         error = "";
   }

   if (i) printf("Error %02x: ", i); printf(error, infile);

   return i;
}


/* fix:
 *
 */
static int
fix (char *infile, int array, int page, int newformat)
{
   FILE *f;
   exe_hdr inh, *outh;
   word32 newhdrlen, newsize, binsize, hdrslack = 0;
   int sizeof_exe_hdr;

   if (infile == NULL) return ERR_INPUT;
   if ((f=fopen(infile, "rb")) == NULL) return ERR_OPEN;
   if (!fread(&inh, sizeof(exe_hdr), 1, f)) return ERR_READ;
   if (inh.sign != 0x5a4d) return ERR_UNKNOWN;

   sizeof_exe_hdr = newformat ? 0x40 : sizeof(exe_hdr);
   newhdrlen = (sizeof_exe_hdr + inh.relocnt * 4 + 15) & ~15;
   binsize = (inh.pagecnt << 9) - ((512 - inh.partpag) & 511) - inh.hdrsize * 16;
   if (page) {
      if (binsize & 0xf) return ERR_FPARA;
      hdrslack = (512 - (newhdrlen + binsize)) & 511;
      newhdrlen += hdrslack;
   }
   newsize = binsize + newhdrlen;
   if ((outh=malloc(newsize)) == NULL) return ERR_MEM;

   memset(outh, 0, newhdrlen);
   outh->sign = inh.sign;
   outh->relocnt = inh.relocnt;
   outh->hdrsize = newhdrlen / 16;
   outh->minmem = inh.minmem;
   outh->maxmem = inh.maxmem;
   outh->reloss = inh.reloss;
   outh->exesp = inh.exesp;
   outh->exeip = inh.exeip;
   outh->relocs = inh.relocs;
   outh->tabloff = sizeof_exe_hdr;
   outh->pagecnt = newsize / 512;
   if ((outh->partpag = newsize % 512)) outh->pagecnt++;
   if (newformat) ((word32 *)outh)[15] = newsize;

   fseek(f, inh.tabloff, SEEK_SET);
   if (fread((word8 *)outh + outh->tabloff, 4, inh.relocnt, f) != inh.relocnt) return ERR_READ;
   fseek(f, inh.hdrsize * 16, SEEK_SET);
   if (hdrslack >= strlen(shameless2)) {
      strcpy((char *)outh + newhdrlen - hdrslack, shameless2);
   } else if (hdrslack >= strlen(shameless1)) {
      strcpy((char *)outh + newhdrlen - hdrslack, shameless1);
   } else if (hdrslack >= strlen(shameless0)) {
      strcpy((char *)outh + newhdrlen - hdrslack, shameless0);
   }
   if (!fread((word8 *)outh + newhdrlen, binsize, 1, f)) return ERR_READ;
   fclose(f);

   if (array) {
      word32 i;
      for (i = 0; i < newsize; i++) {
         fprintf(stdout, "0x%02x,%c", ((word8 *)outh)[i], ((i&15)==15) ? '\n' : ' ');
      }
   } else {
      if ((f=fopen(infile,"wb")) == NULL) return ERR_CREATE;
      if (!fwrite(outh, newsize, 1, f)) return ERR_WRITE;
      fclose(f);
   }

   return OK;
}
