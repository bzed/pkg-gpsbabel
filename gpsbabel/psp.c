/*
    PocketStreets 2002 Pushpin Files
    Contributed to gpsbabel by Alex Mottram (geo_alexm at cox-internet.com)
    
    Copyright (C) 2002 Robert Lipe, robertlipe@usa.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

 */

#include "defs.h"
#include <ctype.h>
#include <math.h>  /* for M_PI */

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

#define MYNAME	"PSP"

#define MAXPSPSTRINGSIZE	256
#define MAXPSPOUTPUTPINS	8192   /* Any more points than this is ludicrous */

static FILE *psp_file_in;
static FILE *psp_file_out;
static FILE *mkshort_handle;

static int i_am_little_endian;
static int endianness_tested;

static void
test_endianness(void)
{
        union {
                long l;
                unsigned char uc[sizeof (long)];
        } u;

        u.l = 1;
        i_am_little_endian = u.uc[0];

        endianness_tested = 1;

}

static int
psp_fread(void *buff, size_t size, size_t members, FILE * fp) 
{
    size_t br;

    br = fread(buff, size, members, fp);

    if (br != members) {
        fatal(MYNAME ": requested to read %d bytes, read %d bytes.\n", members, br);
    }

    return (br);
}

static double
psp_fread_double(FILE *fp)
{
	unsigned char buf[8];
	unsigned char sbuf[8];

	if (!endianness_tested) {
		test_endianness();
	}

	psp_fread(buf, 1, 8, psp_file_in);
	if (i_am_little_endian) {
		return *(double *) buf;
	}
	sbuf[0] = buf[7];
	sbuf[1] = buf[6];
	sbuf[2] = buf[5];
	sbuf[3] = buf[4];
	sbuf[4] = buf[3];
	sbuf[5] = buf[2];
	sbuf[6] = buf[1];
	sbuf[7] = buf[0];
	return *(double *)sbuf;
}

static void
psp_fwrite_double(double x, FILE *fp)
{
	unsigned char *cptr = (unsigned char *)&x;
	unsigned char cbuf[8];

	if (!endianness_tested) {
		test_endianness();
	}
	if (i_am_little_endian) {
		fwrite(&x, 8, 1, fp);
	} else {
                cbuf[0] = cptr[7];
                cbuf[1] = cptr[6];
                cbuf[2] = cptr[5];
                cbuf[3] = cptr[4];
                cbuf[4] = cptr[3];
                cbuf[5] = cptr[2];
                cbuf[6] = cptr[1];
                cbuf[7] = cptr[0];
                fwrite(cbuf, 8, 1, fp);
	}

}
#if 0
static void
psp_fwrite_word(unsigned int x, FILE *fp)
{
	char *cptr = &x;
	char *cbuf[4];

	if (!endianness_tested) {
		test_endianness();
	}
	if (i_am_little_endian) {
		fwrite(&x, 4, 1, fp);
	} else {
                cbuf[0] = cptr[3];
                cbuf[1] = cptr[2];
                cbuf[2] = cptr[1];
                cbuf[3] = cptr[0];
                fwrite(cbuf, 4, 1, fp);
	}
}
#endif


/* Implement the grid in ascii art... This makes a bit of sense if you stand
   on a point over the north pole and look down on the earth.

-180   -90        0       90      180 
------------------------------------    /\
| 0x03  U|S 0x02 U|k 0x00  | 0x01   |   90
|--------|--------|--------|--------|   0
| 0x07   |  0x06  |  0x04  | 0x05   |   -90
------------------------------------    \/
*/    
static 
char grid_byte(double lat, double lon)
{
    char c = 0x00;
    
    if ((lon >= 0.0) && (lon < 90.0)) {
        if (lat >= 0.0) {
            c = 0x00;
        } else {
            c = 0x04;
        }
    } else
    if (lon >= 90.0) {
        if (lat >= 0.0) {
            c = 0x01;
        } else {
            c = 0x05;
        }
    } else
    if ((lon < 0.0) && (lon >= -90.0)) {
        if (lat >= 0.0) {
            c = 0x02;
        } else {
            c = 0x06;
        }
    } else 
    if (lon < -90.0) {
        if (lat >= 0.0) {
            c = 0x03;
        } else {
            c = 0x07;
        }
    }
        
    return (c);
}    

void decode_psp_coordinates(double * lat, double * lon, const char lonbyte)
{
    /* This is some sort of 1/2 Polar,  1/2 Cartesian coordinate mess in  */
    /* the pin file.  I really shouldn't have to do this. Zones 02 and 03 */
    /* work properly.  The other zones are assumptions based on 02 and 03 */

    if ((lonbyte == 0x02) || (lonbyte == 0x06)) {
        /* one step west of zero longitude */
        if (*lon > 0.0) 
            *lon *= -1.0;
    } else 
    if ((lonbyte == 0x03) || (lonbyte == 0x07)) {
        /* two steps west of zero longitude */
        if (*lon > 0.0)
            *lon -= 180.0;
    } else 
    if ((lonbyte == 0x00) || (lonbyte == 0x04)) {
        /* one step east of zero longitude */
        if (*lon < 0.0)
            *lon *= -1.0;
    } else 
    if ((lonbyte == 0x01) || (lonbyte == 0x05)) {
        /* two steps east of zero longitude */
        if (*lon < 0.0)
            *lon += 180.0;
    }
}

static int
valid_psp_header(char * header)
{
    char header_bytes[] = { 0x31, 0x6E, 0x69, 0x50 }; /* 1niP <stop> */

    return (memcmp(header_bytes, header, 4));
    
}

static char *
buffer_washer(char * buff, int buffer_len)
{
    int i;

    for (i = 0 ; i < buffer_len - 1; i++) {
	if (buff[i] == '\0') {
	    memcpy(&buff[i], &buff[i+1], buffer_len - i);
	    buffer_len--;
	    buff[buffer_len] = '\0';
	}
    }

    return (buff);
}

static void
psp_rd_init(const char *fname)
{
	psp_file_in = fopen(fname, "rb");
	if (psp_file_in == NULL) {
		fatal(MYNAME ": Cannot open %s for reading\n", fname);
	}
}

static void
psp_rd_deinit(void)
{
	fclose(psp_file_in);
}

static void
psp_wr_init(const char *fname)
{
	psp_file_out = fopen(fname, "wb");
	mkshort_handle = mkshort_new_handle();

	if (psp_file_out == NULL) {
		fatal(MYNAME ": Cannot open %s for writing\n", fname);
	}
}

static void
psp_wr_deinit(void)
{
	mkshort_del_handle(mkshort_handle);
	fclose(psp_file_out);
}

static void
psp_read(void)
{
	char buff[MAXPSPSTRINGSIZE + 1];
	double radians;
	double lat, lon;
	waypoint *wpt_tmp;
	int stringsize;
	short int pincount;
	short int pindex;
        char gridbyte = 0x00;

        /* 32 bytes - file header */
        psp_fread(&buff[0], 1, 32, psp_file_in);

        if (valid_psp_header(buff) != 0) {
            fatal(MYNAME ": input file does not appear to be a valid .PSP file.\n");
        }

	pincount = le_read16(&buff[12]);

	while (pincount--) {
	    wpt_tmp = xcalloc(sizeof(*wpt_tmp),1);

            wpt_tmp->position.altitude.altitude_meters = unknown_alt;
            
            /* offset 0x20 - 0x21 pin index */
    	    psp_fread(&pindex, 1, 2, psp_file_in);

            /* offset 0x22 - 0x23 */
    	    psp_fread(&buff[0], 1, 2, psp_file_in);

            /* offset 0x24 */
            /* 1 byte, the grid byte - needed for sign corrections later*/
            psp_fread(&gridbyte, 1, 1, psp_file_in);

            /* 8 bytes - latitude in radians */
	    radians = psp_fread_double(psp_file_in);
            lat = (radians * 180.0) / M_PI;

            /* 8 bytes - longitude in radians */
	    radians = psp_fread_double(psp_file_in);
            lon = (radians * 180.0) / M_PI;

            /* since we don't know the origin of this PSP file, we use  */
            /* the grid byte adjust longitude, if necessary, mimicing   */
            /* the behavior of pocketstreets correcting the data.  This */
            /* does not correct the fact that points in eastern US are  */
            /* written with the wrong coordinates by S&T. (MS bug)      */

            decode_psp_coordinates(&lat, &lon, gridbyte);

            wpt_tmp->position.latitude.degrees = lat;
            wpt_tmp->position.longitude.degrees = lon;
           
            /* 1 byte - pin display properties */
            psp_fread(&buff[0], 1, 1, psp_file_in);

	    /* 3 bytes - unknown */
            psp_fread(&buff[0], 1, 3, psp_file_in);

            /* 1 bytes - icon (values: 0x00 - 0x27) */
            psp_fread(&buff[0], 1, 1, psp_file_in);

	    /* 3 bytes - unknown */
    	    psp_fread(&buff[0], 1, 3, psp_file_in);

            /* 1 byte - string size */
    	    psp_fread(&buff[0], 1, 1, psp_file_in);

    	    stringsize = buff[0];
    	    stringsize *= 2;

    	    if (stringsize > MAXPSPSTRINGSIZE) {
		fatal(MYNAME ": variable string size (%d) in PSP file exceeds MAX (%d).\n", stringsize, MAXPSPSTRINGSIZE);
    	    }

            /* stringsize bytes - string data */
    	    psp_fread(&buff[0], 1, stringsize, psp_file_in);

	    buffer_washer(buff, stringsize);

	    wpt_tmp->shortname = xstrdup(buff);

            /* 1 bytes string size */
    	    psp_fread(&buff[0], 1, 1, psp_file_in);

    	    stringsize = buff[0];
    	    stringsize *= 2;

    	    if (stringsize > MAXPSPSTRINGSIZE) {
		fatal(MYNAME ": variable string size (%d) in PSP file exceeds MAX (%d).\n", stringsize, MAXPSPSTRINGSIZE);
    	    }

            /* stringsize bytes - string data */
    	    psp_fread(&buff[0], 1, stringsize, psp_file_in);

	    buffer_washer(buff, stringsize);

	    wpt_tmp->description = xstrdup(buff);

            /* 1 bytes - string size */
    	    psp_fread(&buff[0], 1, 1, psp_file_in);

    	    stringsize = buff[0];
    	    stringsize *= 2;

    	    if (stringsize > MAXPSPSTRINGSIZE) {
		fatal(MYNAME ": variable string size (%d) in PSP file exceeds MAX (%d).\n", stringsize, MAXPSPSTRINGSIZE);
    	    }

            /* stringsize bytes - string data (address?) */
    	    psp_fread(&buff[0], 1, stringsize, psp_file_in);

	    buffer_washer(buff, stringsize);

	    waypt_add(wpt_tmp);
	}
}

static void
psp_waypt_pr(const waypoint *wpt)
{
	double lon, lat;
	char tbuf[64];
	char c;
	int i;
	static short int pindex = 0;
	char *shortname;
	char *description;

        if ((! wpt->shortname) || (global_opts.synthesize_shortnames)) {
            if (wpt->description) {
                if (global_opts.synthesize_shortnames)
                    shortname = mkshort(mkshort_handle, wpt->description);
                else
                    shortname = xstrdup(wpt->description);
            } else {
                /* no description available */
                shortname = xstrdup("");
            }
        } else{
            shortname = xstrdup(wpt->shortname);
        }

        if (! wpt->description) {
            if (shortname) {
                description = xstrdup(shortname);
            } else {
                description = xstrdup("");
            }
        } else{
            description = xstrdup(wpt->description);
        }

        /* convert lat/long back to radians */
	lat = (wpt->position.latitude.degrees * M_PI) / 180.0;
        lon = (wpt->position.longitude.degrees * M_PI) / 180.0;
        
	pindex++;
	le_write16(tbuf, pindex);
        /* 2 bytes - pin index */
        fwrite(tbuf, 1, 2, psp_file_out);
        
        /* 2 bytes - null bytes */
        memset(tbuf, '\0', sizeof(tbuf));
        fwrite(tbuf, 1, 2, psp_file_out);
        

        /* set the grid byte */
	c = grid_byte(wpt->position.latitude.degrees, 
	              wpt->position.longitude.degrees);

	/* since the grid byte matches with what pocketstreets does to   */
	/* input files, our output appears identical to a pin file that  */
        /* has already been processed and corrected by pocketstreets.    */
        /* Due to the grid and signs, it'll look different than one that */
        /* comes straight from S&T.                                      */
	
        /* the grid byte */
        fwrite(&c, 1, 1, psp_file_out);

        /* 8 bytes - latitude/radians */
        psp_fwrite_double(lat, psp_file_out);

        /* 8 bytes - longitude/radians */
        psp_fwrite_double(lon, psp_file_out);

        /* 1 byte - pin properties */
        c = 0x14; /* display pin name on, display notes on. 0x04 = no notes */
        fwrite(&c, 1, 1, psp_file_out);

        memset(tbuf, '\0', sizeof(tbuf));

        /* 3 unknown bytes */
        fwrite(tbuf, 1, 3, psp_file_out);

        /* 1 icon byte 0x00 = PIN */
        fwrite(tbuf, 1, 1, psp_file_out);

        /* 3 unknown bytes */
        fwrite(tbuf, 1, 3, psp_file_out); /* 3 junk */

        c = strlen(shortname);
        /* 1 string size */
        fwrite(&c, 1, 1, psp_file_out);

        for (i = 0 ; shortname[i] ; i++) {
             fwrite(&shortname[i], 1, 1, psp_file_out);    /* char */
             fwrite(&tbuf[0], 1, 1, psp_file_out);              /* null */
        }

        c = strlen(description);
        /*  1 byte string size */
        fwrite(&c, 1, 1, psp_file_out);

        for (i = 0 ; description[i] ; i++) {
             fwrite(&description[i], 1, 1, psp_file_out);  /* char */
             fwrite(&tbuf[0], 1, 1, psp_file_out);              /* null */
        }

        /* just for the hell of it, we'll scrap the third string. */
        c = strlen(tbuf);
        /* 1 byte string size */
        fwrite(&c, 1, 1, psp_file_out);

        for (i = 0 ; tbuf[i] ; i++) {
             fwrite(&tbuf[i], 1, 1, psp_file_out);              /* char */
             fwrite(&tbuf[0], 1, 1, psp_file_out);              /* null */
        }
        
        xfree(shortname);
        xfree(description);
}

static void
psp_write(void)
{
        short int s;
	unsigned char header_bytes[] = { 0x31, 0x6E, 0x69, 0x50, 0x20, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        /* the header: */
        /* 31 6E 69 50 20 00 00 00 08 00 00 00 11 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 */
        /* offset 0x0C - 0x0D = 2 byte pin count */

        s = waypt_count();
        
        if (global_opts.synthesize_shortnames) {
            setshort_length(mkshort_handle, 32);
            setshort_whitespace_ok(mkshort_handle, 1);
        }

        if (s > MAXPSPOUTPUTPINS) {
            fatal(MYNAME ": attempt to output too many pushpins (%d).  The max is %d.  Sorry.\n", s, MAXPSPOUTPUTPINS);
        }

        /* insert waypoint count into header */
	le_write16(&header_bytes[12], s);

        fwrite(header_bytes, 1,  32, psp_file_out);

        waypt_disp_all(psp_waypt_pr);
}

ff_vecs_t psp_vecs = {
	psp_rd_init,
	psp_wr_init,
	psp_rd_deinit,
	psp_wr_deinit,
	psp_read,
	psp_write,
	NULL
};
