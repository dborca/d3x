/*
 *                 D3X(tm) DOS-Extender v0.90 unleash `h' alpha
 *                                stub manager
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

#define _16_ *(word16 *)&
#define _32_ *(word32 *)&

#define SIGN_MZ   0x5a4d    /* MZ */
#define SIGN_D3X1 0x31583344/* D3X1 */
#define SIGN_COFF 0x014c    /* COFF */
#define SIGN_LE   0x0000454c/* LE little endian */
#define SIGN_LX   0x0000584c/* LX little endian */
#define SIGN_PE   0x00004550/* PE */
#define SIGN_NE   0x454e    /* NE */
#define SIGN_BW   0x5742    /* BW */

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


static unsigned char d3x1[] = {
#include "d3x1.h"
};
static unsigned char d3xd[] = {
#include "d3xd.h"
};
static unsigned char d3xw[] = {
#include "d3xw.h"
};


#ifdef DJGPP
char **__crt0_glob_function (char *_argument)         { (void)_argument; return 0; }
void   __crt0_load_environment_file (char *_app_name) { (void)_app_name; }
#endif

static const char *bname (const char *path);
static int chkexe (const char *file, exetype *type, int *offset, int *ref);

static void cut_banners (void);
static void dump_extender (const char *filename, void *data, int size);
static void view_file (const char *file);
static void make_stubbed (const char *in, const char *out);
static void make_unstubbed (char *in, char *out);


/* usage:
 *  print mini-help
 */
static void NORET
usage (const char *myself)
{
   printf(
"StubX stub manager v0.90.h\n"
"Copyright (c) 1999-2004 Daniel Borca\n"
"\n"
"Usage: %s -d\n"
"   or  %s -v executable\n"
"   or  %s -{s|u} infile [outfile]\n"
"\n"
"   -d  dump extenders\n"
"   -v  view\n"
"   -s  stubify\n"
"   -u  unstubify\n"
, myself, myself, myself);

   exit(OK);
}


/* erra:
 *  print error
 */
static void
erra (const char *prefix, ERR code, const char *optional)
{
   switch (code) {
      case ERR_SYNTAX:
         printf("%sError %02x: syntax error. Try %s -h\n", prefix, code, optional);
         break;
      case ERR_CREATE:
         printf("%sError %02x: unable to create (%s)\n", prefix, code, optional);
         break;
      case ERR_OPEN:
         printf("%sError %02x: unable to open (%s)\n", prefix, code, optional);
         break;
      case ERR_READ:
         printf("%sError %02x: unable to read (%s)\n", prefix, code, optional);
         break;
      case ERR_WRITE:
         printf("%sError %02x: unable to write (%s)\n", prefix, code, optional);
         break;
      case ERR_MEM:
         printf("%sError %02x: not enough memory\n", prefix, code);
         break;
      case ERR_UNKNOWN:
         printf("%sError %02x: unknown input type\n", prefix, code);
         break;
      case ERR_UNSUPP:
         printf("%sError %02x: unsupported input type\n", prefix, code);
         break;
      case ERR_RELOC:
         printf("%sError %02x: relocation items detected\n", prefix, code);
         break;
      case ERR_STACK:
         printf("%sError %02x: input executable has no stack\n", prefix, code);
         break;
      default:
         ;
   }
}


/* bail:
 *  bail out
 */
static void NORET
bail (const char *prefix, ERR code, const char *optional)
{
   erra(prefix, code, optional);
   exit(code);
}


/* main:
 *
 */
int
main (int argc, char **argv)
{
   word32 opt;
   const char *myself = bname(argv[0]);

   if ((argc < 2) || (argc > 4)) {
      bail("", ERR_SYNTAX, myself);
   }

   opt = (_32_ argv[1][0]) & 0x00ffffff;
   switch (opt) {
      case 0x682d: /* -h */
         if (argc != 2) {
            bail("", ERR_SYNTAX, myself);
         } else {
            usage(myself);
         }
      case 0x642d: /* -d */
         if (argc != 2) {
            bail("", ERR_SYNTAX, myself);
         } else {
            dump_extender("d3x1.exe", d3x1, sizeof(d3x1));
            dump_extender("d3xd.exe", d3xd, sizeof(d3xd));
            dump_extender("d3xw.exe", d3xw, sizeof(d3xw));
         }
         break;
      case 0x762d: /* -v */
         if (argc != 3) {
            bail("", ERR_SYNTAX, myself);
         } else {
            view_file(argv[2]);
         }
         break;
      case 0x532d: /* -S */
         cut_banners();
      case 0x732d: /* -s */
         if ((argc != 3) && (argc != 4)) {
            bail("", ERR_SYNTAX, myself);
         } else {
            make_stubbed(argv[2], (argc==4) ? argv[3] : argv[2]);
         }
         break;
      case 0x752d: /* -u */
         if ((argc != 3) && (argc != 4)) {
            bail("", ERR_SYNTAX, myself);
         } else {
            make_unstubbed(argv[2], (argc==4) ? argv[3] : argv[2]);
         }
         break;
      default:
         bail("", ERR_SYNTAX, myself);
   }

   return 0;
}


/* cut_banners:
 *  prevent banners in extenders
 */
static void
cut_banners (void)
{
   _16_ d3x1[((_16_ d3x1[8])<<4) + sizeof(_D3X1_StubInfo) + 3] = 0x0beb;
   _16_ d3xd[((_16_ d3xd[8])<<4) + sizeof(_GO32_StubInfo) + 3] = 0x0beb;
   _16_ d3xw[((_16_ d3xw[8])<<4) + sizeof(_o2LE_StubInfo) + 3] = 0x0beb;
}


/* dump_extender:
 *
 */
static void
dump_extender (const char *filename, void *data, int size)
{
   FILE *f;

   printf("Dumping (%s)...", filename);
   if ((f=fopen(filename, "wb")) != NULL) {
      if (fwrite(data, size, 1, f)) {
         printf(" done.\n");
      } else {
         erra("\n", ERR_WRITE, filename);
      }
      fclose(f);
   } else {
      erra("\n", ERR_CREATE, filename);
   }
}


/* analyze_mz:
 *
 */
static void
analyze_mz (const char *file, int offset)
{
   FILE *f;
   exe_hdr header;

   if ((f=fopen(file, "rb")) == NULL) {
      bail("", ERR_OPEN, file);
   }

   if (offset) {
      printf("Offset  : %#x\n", offset);
      fseek(f, offset, SEEK_SET);
   }

   if (!fread(&header, sizeof(header), 1, f)) {
      fclose(f);
      bail("", ERR_READ, file);
   }
   printf("MZ hdr  :\n");
   printf("\tbinary size\t= %d\n", ((header.pagecnt<<9) - ((512 - header.partpag) & 511)) - header.hdrsize * 16);
   printf("\textra mem\t= %d\n", header.minmem * 16);
   printf("\tentry\t\t= %04x:%04x (base+%08x)\n", header.relocs, header.exeip, header.relocs * 16 + header.exeip);
   printf("\tstack\t\t= %04x:%04x (base+%08x)\n", header.reloss, header.exesp, header.reloss * 16 + header.exesp);
   printf("\trelocs\t\t= %d\n", header.relocnt);

   fclose(f);
}


/* analyze_d3x1:
 *
 */
static void
analyze_d3x1 (const char *file, int offset)
{
   FILE *f;
   _D3X1_StubInfo d3x1_si;
   exe_hdr header;
   d3x1_hdr subheader;
   char tmp[32];

   if ((f=fopen(file, "rb")) == NULL) {
      bail("", ERR_OPEN, file);
   }

   if (offset) {
      printf("Offset  : %#x\n", offset);
      if (!fread(&header, sizeof(header), 1, f)) {
         fclose(f);
         bail("", ERR_READ, file);
      }
      fseek(f, header.hdrsize * 16, SEEK_SET);
      if (!fread(&d3x1_si, sizeof(d3x1_si), 1, f)) {
         fclose(f);
         bail("", ERR_READ, file);
      }
      if (!memcmp(d3x1_si.magic, "d3x1stub", 8)) {
         printf("StubInfo:\n");
         strncpy(tmp, d3x1_si.magic, 16); tmp[16] = '\0';
         printf("\tmagic\t\t= %s\n", tmp);
         printf("\tstubinfo size\t= %lxh\n", d3x1_si.size);
         printf("\ttransferbuffer\t= %d\n", d3x1_si.minkeep);
      }
      fseek(f, offset, SEEK_SET);
   }

   if (!fread(&subheader, sizeof(subheader), 1, f)) {
      fclose(f);
      bail("", ERR_READ, file);
   }
   printf("D3X1 hdr:\n");
   printf("\tbinary size\t= %lu\n", subheader.binsize);
   printf("\tadditional mem\t= %lu\n", subheader.addsize);
   printf("\tentry point\t= 0x%08lx\n", subheader.entry);
   printf("\ttop of stack\t= 0x%08lx\n", subheader.tos);

   fclose(f);
}


/* analyze_coff:
 *
 */
static void
analyze_coff (const char *file, int offset)
{
   FILE *f;
   _GO32_StubInfo go32_si;
   exe_hdr header;
   coff_hdr subheader;
   char tmp[32];

   if ((f=fopen(file, "rb")) == NULL) {
      bail("", ERR_OPEN, file);
   }

   if (offset) {
      printf("Offset  : %#x\n", offset);
      if (!fread(&header, sizeof(header), 1, f)) {
         fclose(f);
         bail("", ERR_READ, file);
      }
      fseek(f, header.hdrsize * 16, SEEK_SET);
      if (!fread(&go32_si, sizeof(go32_si), 1, f)) {
         fclose(f);
         bail("", ERR_READ, file);
      }
      if (!memcmp(go32_si.magic, "go32stub", 8)) {
         printf("StubInfo:\n");
         strncpy(tmp, go32_si.magic, 16); tmp[16] = '\0';
         printf("\tmagic\t\t= %s\n", tmp);
         printf("\tstubinfo size\t= %lxh\n", go32_si.size);
         printf("\tstack size\t= 0x%08lx\n", go32_si.minstack);
         printf("\ttransferbuffer\t= %d\n", go32_si.minkeep);
         strncpy(tmp, go32_si.basename, 8); tmp[8] = '\0';
         printf("\trun file\t= %s\n", tmp[0] ? tmp : "<self>");
         strncpy(tmp, go32_si.argv0, 16); tmp[16] = '\0';
         printf("\targv0\t\t= %s\n", tmp[0] ? tmp : "<default>");
         printf("\tdpmi server\t= %s\n", go32_si.dpmi_server[0] ? go32_si.dpmi_server : "<none>");
      }
      fseek(f, offset, SEEK_SET);
   }

   if (!fread(&subheader, sizeof(subheader), 1, f)) {
      fclose(f);
      bail("", ERR_READ, file);
   }

   printf("COFF hdr:\n");
   if (subheader.coff.f_flags & 2) {
      printf("\ttext\t\t= %lu\n", subheader.text_sect.s_size);
      printf("\tdata\t\t= %lu\n", subheader.data_sect.s_size);
      printf("\tbss\t\t= %lu\n", subheader.bss_sect.s_size);
      printf("\tstart eip\t= text:0x%08lx\n", subheader.aout.entry - subheader.text_sect.s_vaddr);
   } else {
      printf("\ttype\t\t= unresolved\n");
   }

   fclose(f);
}


/* analyze_le:
 *
 */
static void
analyze_le (const char *file, int offset, int ref)
{
   FILE *f;
   le_hdr header;
   obj_tbl *table;
   word32 mem;
   int i;

   if ((f=fopen(file, "rb")) == NULL) {
      bail("", ERR_OPEN, file);
   }
   if (offset) {
      printf("Offset  : %#x\n", offset);
      if (ref) {
         printf("X-Base  : %#x\n", ref);
      }
      fseek(f, offset, SEEK_SET);
   }

   if (!fread(&header, sizeof(header), 1, f)) {
      fclose(f);
      bail("", ERR_READ, file);
   }

   if ((table=malloc(header.LEObjectEntries * sizeof(obj_tbl))) == NULL) {
      fclose(f);
      bail("", ERR_MEM, NULL);
   }
   fseek(f, offset+header.LEObjectTable, SEEK_SET);
   if (!fread(table, sizeof(obj_tbl), header.LEObjectEntries, f)) {
      free(table);
      fclose(f);
      bail("", ERR_READ, file);
   }

   printf("LE hdr  :\n");
   printf("\tloader size\t= 0x%lx\n", header.LELoaderSize);
   printf("\tdata offset\t= 0x%08lx\n", header.LEDataPages);
   printf("\tentry point\t= %08lx:%08lx\n", header.LEEntrySection, header.LEEntryOffset);
   printf("\ttop of stack\t= %08lx:%08lx\n", header.LEStackSection, header.LEInitialESP);
   for (mem = 0, i = 0; (unsigned)i < header.LEObjectEntries; i++) {
      mem += (table[i].OTVirtualSize + 4095) & ~4095;
      printf("\tobject %04d\t= (%08lx+%08lx)", i+1, table[i].OTRelocBase, table[i].OTVirtualSize);
      if (table[i].OTObjectFlags & 1) printf(" readable");
      if (table[i].OTObjectFlags & 2) printf(" writable");
      if (table[i].OTObjectFlags & 4) printf(" executbl");
      if (table[i].OTObjectFlags & 0x2000) printf(" big");
      printf("\n");
   }
   printf("\tprogram mem\t= 0x%lx\n", mem);

   free(table);
   fclose(f);
}


/* analyze_unknown:
 *
 */
static void
analyze_unknown (const char *file, int offset, int ref)
{
   if (offset) {
      printf("Offset  : %#x\n", offset);
      if (ref) {
         printf("X-Base  : %#x\n", ref);
      }
   }
   (void)file;
}


/* view_file:
 *
 */
static void
view_file (const char *file)
{
   int result, offset, ref;
   exetype type;

   result = chkexe(file, &type, &offset, &ref);

   switch (result) {
      case 0:
         printf("Filename: %s\nType    : %s", file, offset ? "stubbed " : "");
         switch (type) {
            case X_UNKNOWN:
               printf("<unknown>\n");
               analyze_unknown(file, offset, ref);
               break;
            case X_MZ:
               printf("MZ\n");
               analyze_mz(file, offset);
               break;
            case X_D3X1:
               printf("D3X1\n");
               analyze_d3x1(file, offset);
               break;
            case X_COFF:
               printf("COFF\n");
               analyze_coff(file, offset);
               break;
            case X_LE:
               printf("LE\n");
               analyze_le(file, offset, ref);
               break;
            case X_LX:
               printf("LX\n");
               analyze_unknown(file, offset, ref);
               break;
            case X_PE:
               printf("PE\n");
               analyze_unknown(file, offset, ref);
               break;
            case X_NE:
               printf("NE\n");
               analyze_unknown(file, offset, ref);
               break;
            case X_BW:
               printf("BW\n");
               analyze_unknown(file, offset, ref);
               break;
         }
         break;
      case ERR_OPEN:
         bail("", ERR_OPEN, file);
      case ERR_READ:
         bail("", ERR_READ, file);
   }
}


/* build_d3x1:
 *  build D3X1 from MZ file
 */
static void
build_d3x1 (const char *in, const char *out)
{
   FILE *f;
   exe_hdr header;
   d3x1_hdr subheader;
   word8 *buffer;

   if ((f=fopen(in, "rb")) == NULL) {
      bail("\n", ERR_OPEN, in);
   }
   if (!fread(&header, sizeof(header), 1, f)) {
      fclose(f);
      bail("\n", ERR_READ, in);
   }

   subheader.sign = SIGN_D3X1;
   subheader.hdrsize = sizeof(d3x1_hdr);
   subheader.binsize = ((header.pagecnt<<9) - ((512 - header.partpag) & 511)) - header.hdrsize * 16;
   subheader.addsize = header.minmem * 16;
   subheader.entry = header.relocs * 16 + header.exeip;
   subheader.tos = header.reloss * 16 + header.exesp;
   memset(&subheader.filler, 0, sizeof(subheader.filler));
   if (header.relocnt) {
      fclose(f);
      bail("\n", ERR_RELOC, NULL);
   }
   if (subheader.tos == 0) {
      fclose(f);
      bail("\n", ERR_STACK, NULL);
   }
   if ((buffer=malloc(subheader.binsize)) == NULL) {
      fclose(f);
      bail("\n", ERR_MEM, NULL);
   }

   fseek(f, header.hdrsize * 16, SEEK_SET);
   if (!fread(buffer, subheader.binsize, 1, f)) {
      free(buffer);
      fclose(f);
      bail("\n", ERR_READ, in);
   }
   fclose(f);

   if ((f=fopen(out, "wb")) == NULL) {
      free(buffer);
      bail("\n", ERR_CREATE, out);
   }
   if (!fwrite(&subheader, sizeof(subheader), 1, f)) {
      fclose(f);
      free(buffer);
      bail("\n", ERR_WRITE, out);
   }
   if (!fwrite(buffer, subheader.binsize, 1, f)) {
      fclose(f);
      free(buffer);
      bail("\n", ERR_WRITE, out);
   }
   fclose(f);

   free(buffer);
   printf(" done.\n");
}


/* stubify:
 *
 */
static void
stubify (const char *in, int offset, word8 *stubaddress, int stubsize, exetype type, const char *out, int ref)
{
   FILE *f;
   word32 size;
   word8 *buffer;

   if ((offset|stubsize) == 0) goto done;

   if ((f=fopen(in, "rb")) == NULL) {
      bail("\n", ERR_OPEN, in);
   }
   fseek(f, 0, SEEK_END);
   size = ftell(f) - offset;
   fseek(f, offset, SEEK_SET);
   if ((buffer=malloc(size)) == NULL) {
      fclose(f);
      bail("\n", ERR_MEM, NULL);
   }
   if (!fread(buffer, size, 1, f)) {
      free(buffer);
      fclose(f);
      bail("\n", ERR_READ, in);
   }
   fclose(f);

   if (type == X_LE) {
      word8 *p;
      le_hdr *b;
      word32 before, after, loadersize;

      printf(" patching LE...");
      b = (le_hdr *)buffer;
      loadersize = b->LELoaderSize;
      before = b->LEDataPages - offset + ref;
      /* OpenWatcom woes */ {
         if (b->LEPPchecksumTab) {
            loadersize = b->LEPPchecksumTab - b->LEObjectTable + b->LENumberPages * 4;
         } else {
            loadersize = b->LEImpProcTable - b->LEObjectTable + 1;
            p = buffer + b->LEImpProcTable;
            while (*p) {
               p += 1 + *p;
            }
            loadersize += p - (buffer + b->LEImpProcTable);
         }
      }
      /* fix broken executables */ {
         word32 i;
         p = buffer + b->LEObjectTable + loadersize;
         for (i = 0; i < (size - (b->LEObjectTable + loadersize)); i++) {
            if (p[i]) {
               before = b->LEObjectTable + loadersize + i;
               break;
            }
         }
      }
      after = (((stubsize + b->LEObjectTable + loadersize) + 511) & ~511) - stubsize;
      if (after != before) {
         if (after < before) {
            memmove(&buffer[after], &buffer[before], size-before);
         } else {
            if ((p=(word8 *)realloc(buffer, size+after-before)) == NULL) {
               free(buffer);
               bail("\n", ERR_MEM, NULL);
            }
            buffer = p;
            b = (le_hdr *)buffer;
            memmove(&buffer[after], &buffer[before], size-before);
            memset(&buffer[before], 0, after-before);
         }
         size += after - before;
         if (b->LENonResTable) b->LENonResTable += stubsize + after - b->LEDataPages;
         if (b->LEDebugStart) b->LEDebugStart += stubsize + after - b->LEDataPages;
      }
      b->LEDataPages = stubsize + after;
   }

   if (offset) {
      printf(" removing old stub...");
   }

   if ((f=fopen(out, "wb")) == NULL) {
      free(buffer);
      bail("\n", ERR_CREATE, out);
   }
   if (stubsize) {
      if (!fwrite(stubaddress, stubsize, 1, f)) {
         fclose(f);
         free(buffer);
         bail("\n", ERR_WRITE, out);
      }
   }
   if (!fwrite(buffer, size, 1, f)) {
      fclose(f);
      free(buffer);
      bail("\n", ERR_WRITE, out);
   }
   fclose(f);

   free(buffer);
done:
   printf(" done.\n");
}


/* make_stubbed:
 *
 */
static void
make_stubbed (const char *in, const char *out)
{
   int result, offset, ref;
   exetype type;

   result = chkexe(in, &type, &offset, &ref);

   switch (result) {
      case 0:
         switch (type) {
            case X_UNKNOWN:
               bail("", ERR_UNKNOWN, NULL);
            case X_MZ:
               if (offset) {
                  bail("", ERR_UNSUPP, NULL);
               }
               printf("Building D3X1...");
               build_d3x1(in, out);
               in = out;
            case X_D3X1:
               printf("Stubifying D3X1...");
               stubify(in, offset, d3x1, sizeof(d3x1), type, out, 0);
               break;
            case X_COFF:
               printf("Stubifying COFF...");
               stubify(in, offset, d3xd, sizeof(d3xd), type, out, 0);
               break;
            case X_LE:
               printf("Stubifying LE...");
               stubify(in, offset, d3xw, sizeof(d3xw), type, out, ref);
               break;
            case X_LX:
            case X_PE:
            case X_NE:
            case X_BW:
               bail("", ERR_UNSUPP, NULL);
         }
         break;
      case ERR_OPEN:
         bail("", ERR_OPEN, in);
      case ERR_READ:
         bail("", ERR_OPEN, in);
   }
}


/* make_unstubbed:
 *
 */
static void
make_unstubbed (char *in, char *out)
{
   int result, offset, ref;
   exetype type;

   result = chkexe(in, &type, &offset, &ref);

   switch (result) {
      case 0:
         switch (type) {
            case X_UNKNOWN:
               bail("", ERR_UNKNOWN, NULL);
            case X_MZ:
            case X_LX:
            case X_PE:
            case X_NE:
            case X_BW:
               bail("", ERR_UNSUPP, NULL);
            case X_D3X1:
               printf("Un-stubifying D3X1...");
               stubify(in, offset, NULL, 0, type, out, 0);
               break;
            case X_COFF:
               printf("Un-stubifying COFF...");
               stubify(in, offset, NULL, 0, type, out, 0);
               break;
            case X_LE:
               printf("Un-stubifying LE...");
               stubify(in, offset, NULL, 0, type, out, ref);
               break;
         }
         break;
      case ERR_OPEN:
         bail("", ERR_OPEN, in);
      case ERR_READ:
         bail("", ERR_OPEN, in);
   }
}


/* bname:
 *  return basename of the file
 */
static const char *
bname (const char *path)
{
   int i = 0;

   while (path[i]) {
      i++;
   }

   while ((i>=0) && (path[i]!=':') && (path[i]!='\\') && (path[i]!='/')) {
      i--;
   }

   return path + i + 1;
}


/* chksig:
 *  check signatures
 */
static exetype
chksig (word32 sign)
{
   switch (sign) {
      case SIGN_D3X1:
         return X_D3X1;
      case SIGN_LE:
         return X_LE;
      case SIGN_LX:
         return X_LX;
      case SIGN_PE:
         return X_PE;
      default:
         switch (sign & 0xffff) {
            case SIGN_MZ:
               return X_MZ;
            case SIGN_COFF:
               return X_COFF;
            case SIGN_NE:
               return X_NE;
            case SIGN_BW:
               return X_BW;
            default:
               return X_UNKNOWN;
         }
   }
}


/* chkexe:
 *  check executable
 */
static int
chkexe (const char *file, exetype *type, int *offset, int *ref)
{
   FILE *f;
   word32 header[16];
   exe_hdr *mz = (void *)header;
   word32 pos, size;

   if ((f=fopen(file, "rb")) == NULL) {
      return ERR_OPEN;
   }
   fseek(f, 0, SEEK_END);
   size = ftell(f);
   fseek(f, 0, SEEK_SET);

   if (!fread(header, sizeof(header), 1, f)) {
      fclose(f);
      return ERR_READ;
   }

   *ref = pos = 0;
check:
   if (mz->sign == SIGN_MZ) {
      *ref = pos;
      pos += (mz->pagecnt<<9) - ((512 - mz->partpag) & 511);

      *type = X_UNKNOWN;
      if ((mz->hdrsize >= 4) && (mz->tabloff >= 0x40) && header[15] && (header[15] <= (size - 4))) {
         fseek(f, header[15], SEEK_SET);
         fread(header, 4, 1, f);
         *type = chksig(header[0]);
         if (*type != X_UNKNOWN) {
            pos = header[15];
         }
      }
      if (*type == X_UNKNOWN) {
         if (pos <= (size - sizeof(header))) {
            fseek(f, pos, SEEK_SET);
            fread(header, sizeof(header), 1, f);
            header[15] += pos;
            if (mz->sign == SIGN_MZ) {
               goto check;
            }
         } else if (pos <= (size - 4)) {
            fseek(f, pos, SEEK_SET);
            fread(header, 4, 1, f);
         } else {
            header[0] = SIGN_MZ;
            pos = *ref;
         }
      }

      while (mz->sign == SIGN_BW) {
         fseek(f, pos, SEEK_SET);
         if (!fread(header, sizeof(header), 1, f)) {
            fclose(f);
            return ERR_READ;
         }

         if (mz->sign == SIGN_BW) {
            if (header[7] == size) {
               break;
            }
            pos = header[7];
         } else {
            header[15] += pos;
            goto check;
         }
      }
   }

   *offset = pos;
   *type = chksig(header[0]);

   fclose(f);
   return 0;
}
