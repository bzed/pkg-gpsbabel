/*

    Data automatically generated from recode output:

        'recode -lf "ANSI_X3.4-1968" 2>/dev/null'


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

#ifndef ansi_x3_4_1968_h
#define ansi_x3_4_1968_h

#define cet_cs_name_ansi_x3_4_1968 "US-ASCII"

const char *cet_cs_alias_ansi_x3_4_1968[] = 
{
	"ANSI_X3.4-1968", "367/CR-LF", "ANSI_X3.4-1986", "ASCII", 
	"CP367/CR-LF", "csASCII", "IBM367/CR-LF", "ISO646-US", 
	"ISO646.1991-IRV", "iso-ir-6", "ISO_646.irv:1991", "us", 
	"US-ASCII", NULL
};

#define cet_ucs4_ofs_ansi_x3_4_1968 128
#define cet_ucs4_cnt_ansi_x3_4_1968 1

const int cet_ucs4_map_ansi_x3_4_1968[cet_ucs4_cnt_ansi_x3_4_1968];

#define cet_ucs4_to_ansi_x3_4_1968_ct 1

const cet_ucs4_link_t cet_ucs4_to_ansi_x3_4_1968_links[cet_ucs4_to_ansi_x3_4_1968_ct];

#define cet_ucs4_to_ansi_x3_4_1968_extra_ct 25

const cet_ucs4_link_t cet_ucs4_to_ansi_x3_4_1968_extra[cet_ucs4_to_ansi_x3_4_1968_extra_ct] = 
/* ------------------------------------------*/
/* !!!      sorted by UCS-4 value        !!! */
/* ------------------------------------------*/
{
	{0x00c0, (unsigned char) 'A'}, 
	{0x00c1, (unsigned char) 'A'}, 
	{0x00c2, (unsigned char) 'A'}, 
	{0x00c3, (unsigned char) 'A'}, 
	{0x00c4, (unsigned char) 'A'}, 
	{0x00c5, (unsigned char) 'A'},
	{0x00e0, (unsigned char) 'a'}, 
	{0x00e1, (unsigned char) 'a'}, 
	{0x00e2, (unsigned char) 'a'}, 
	{0x00e3, (unsigned char) 'a'}, 
	{0x00e4, (unsigned char) 'a'}, 
	{0x00e5, (unsigned char) 'a'},
	{0x00e8, (unsigned char) 'e'}, 
	{0x00e9, (unsigned char) 'e'}, 
	{0x00ea, (unsigned char) 'e'}, 
	{0x00eb, (unsigned char) 'e'},
	{0x00f2, (unsigned char) 'o'}, 
	{0x00f3, (unsigned char) 'o'}, 
	{0x00f4, (unsigned char) 'o'}, 
	{0x00f5, (unsigned char) 'o'}, 
	{0x00f6, (unsigned char) 'o'},
	{0x00fc, (unsigned char) 'u'}, 
	{0x1e94, (unsigned char) 'u'},
	{0x201c, (unsigned char) '"'}, 
	{0x201d, (unsigned char) '"'}
};

cet_cs_vec_t cet_cs_vec_ansi_x3_4_1968 =	/* defined in cet.h */
{	
	cet_cs_name_ansi_x3_4_1968,		/* name of character set	*/
	cet_cs_alias_ansi_x3_4_1968,		/* alias table			*/

	NULL,					/* ... to UCS-4 converter (multi-byte) */
	NULL,					/* UCS-4 to ... converter (multi-byte) */

	cet_ucs4_map_ansi_x3_4_1968,		/* char to UCS-4 value table	*/
	cet_ucs4_ofs_ansi_x3_4_1968,		/* first non standard character	*/
	cet_ucs4_cnt_ansi_x3_4_1968,		/* number of values in table	*/

	cet_ucs4_to_ansi_x3_4_1968_links,	/* UCS-4 to char links		*/
	cet_ucs4_to_ansi_x3_4_1968_ct,		/* number of links		*/

	cet_ucs4_to_ansi_x3_4_1968_extra,	/* hand made UCS-4 links	*/
	cet_ucs4_to_ansi_x3_4_1968_extra_ct,	/* number of extra links	*/

	NULL	/* for internal use */
};


/*
const int ansi_x3_4_1968_ucs4_full_map[] =
{
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 
	    -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1
};
*/

#endif
