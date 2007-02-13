/*
    Universal CSV - support for csv files, divining field order from the header.

    Copyright (C) 2006 Robert Lipe,  robertlipe@usa.net

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
#include "csv_util.h"
#include "jeeps/gpsmath.h"

#define MYNAME "unicsv"

static gbfile *file_in;

/* This structure must contain only ints.  Firstval must be first.
 * This is block initialized.
 */
struct {
	int firstval;
	int latcol;
	int loncol;
	int namecol;
	int desccol;
	int notescol;
	int altcol;
	int urlcol;
	int utmzcol; /* Zone */
	int utmncol; /* Northing */
	int utmecol; /* Easting */
	int utmccol; /* Zone character */
} unicsv_fieldpos;

static double unicsv_altscale;
static char *unicsv_fieldsep;

static
arglist_t unicsv_args[] = {
	ARG_TERMINATOR
};

/* helpers */

// #define UNICSV_IS(f) (0 == strcmp(s, f))
#define UNICSV_CONTAINS(f) (0 != strstr(s, f))

static void
unicsv_fondle_header(char *ibuf)
{
	char *s;
	unsigned int i;
	int *ip = &unicsv_fieldpos.firstval;

	for (i = 0; i < sizeof(unicsv_fieldpos) / sizeof(int); i++, ip++) {
		*ip = -1;
	}

	/* Convert the entire header to lower case for convenience. 
	 * If we see a tab in that header, we decree it to be tabsep.
	 */
	unicsv_fieldsep = ",";	
	for (s = ibuf; *s; s++) {
		if (*s == '\t') {
			unicsv_fieldsep = "\t";
		}
		else if (*s == ';') {
			unicsv_fieldsep = ";";
		}
		else {
			*s = tolower(*s);
		}
	}

	s = csv_lineparse(ibuf, unicsv_fieldsep, "", 0);
	for (i=0; s; i++,s = csv_lineparse(NULL, unicsv_fieldsep, "", 0)) {
		if (UNICSV_CONTAINS("lat")) {
			unicsv_fieldpos.latcol = i;
		}
		else if (UNICSV_CONTAINS("lon")) {
			unicsv_fieldpos.loncol = i;
		}
		else if (UNICSV_CONTAINS("desc")) {
			unicsv_fieldpos.desccol = i;
		}
		else if (UNICSV_CONTAINS("name")) {
			unicsv_fieldpos.namecol = i;
		}
		else if (UNICSV_CONTAINS("notes")) {
			unicsv_fieldpos.notescol = i;
		}
		else if (UNICSV_CONTAINS("alt")) {
			unicsv_fieldpos.altcol = i;
			if (UNICSV_CONTAINS("ft") || UNICSV_CONTAINS("feet")) {
				unicsv_altscale = FEET_TO_METERS(1);
			}
		}
		else if (UNICSV_CONTAINS("url")) {
			unicsv_fieldpos.urlcol = i;
		}
		else if (UNICSV_CONTAINS("utm z")) {
			unicsv_fieldpos.utmzcol = i;
		}
		else if (UNICSV_CONTAINS("utm n")) {
			unicsv_fieldpos.utmncol = i;
		}
		else if (UNICSV_CONTAINS("utm e")) {
			unicsv_fieldpos.utmecol = i;
		}
		else if (UNICSV_CONTAINS("utm c")) {
			unicsv_fieldpos.utmccol = i;
		}
/* todo: speed, course, hdop, sat, date, time, maybe a few others */
	}
}

static void
unicsv_rd_init(const char *fname)
{
	char *c;
	unicsv_altscale = 1.0;

	file_in = gbfopen(fname, "rb", MYNAME);

	if ((c = gbfgetstr(file_in)))
		unicsv_fondle_header(c);
	else
		unicsv_fieldsep = NULL;
}

static void
unicsv_rd_deinit(void)
{
	gbfclose(file_in);
}

static void
unicsv_parse_one_line(char *ibuf)
{
	char *s;
	waypoint *wpt;
	int i;
	int  utmz = -9999;
	double utme = 0;
	double utmn = 0;
	char utmc = 'N';

	s = csv_lineparse(ibuf, unicsv_fieldsep, "\"", 0);
	if (s == NULL) return;
	
	wpt = waypt_new();

	for (i=0; s; i++, s = csv_lineparse(NULL, unicsv_fieldsep, "\"", 0)) {
		s = lrtrim(s);
		if (i == unicsv_fieldpos.latcol) {
			human_to_dec( s, &wpt->latitude, &wpt->longitude, 1 );
		}
		else if (i == unicsv_fieldpos.loncol) {
			human_to_dec( s, &wpt->latitude, &wpt->longitude, 2 );
		}
		else if (i == unicsv_fieldpos.namecol) {
			wpt->shortname = xstrdup(s);
		}
		else if (i == unicsv_fieldpos.desccol) {
			wpt->description = xstrdup(s);
		}
		else if (i == unicsv_fieldpos.notescol) {
			wpt->notes = xstrdup(s);
		}
		else if (i == unicsv_fieldpos.urlcol) {
			wpt->url = xstrdup(s);
		}
		else if (i == unicsv_fieldpos.altcol) {
			wpt->altitude = atof(s) * unicsv_altscale;
		}
		else if (i == unicsv_fieldpos.utmzcol) {
			utmz = atoi(s);
		}
		else if (i == unicsv_fieldpos.utmecol) {
			utme = atof(s);
		}
		else if (i == unicsv_fieldpos.utmncol) {
			utmn = atof(s);
		}
		else if (i == unicsv_fieldpos.utmccol) {
			utmc = toupper(s[0]);
		}
	}
	if (utmz  != -9999) {
		GPS_Math_UTM_EN_To_WGS84(&wpt->latitude, &wpt->longitude,
			utme, utmn, utmz, utmc);
	}
	waypt_add(wpt);
}

static void 
unicsv_rd(void)
{
	char *buff;

	if (unicsv_fieldsep == NULL) return;
	
	while ((buff = gbfgetstr(file_in))) {
	    	buff = lrtrim(buff);
		if (*buff)
			unicsv_parse_one_line(buff);
	}
}

/* --------------------------------------------------------------------------- */

ff_vecs_t unicsv_vecs = {
	ff_type_file,
	{ ff_cap_read, 0, 0},
	unicsv_rd_init,
	NULL,
	unicsv_rd_deinit, 
	NULL,
	unicsv_rd,
	NULL,
	NULL,
	unicsv_args,
	CET_CHARSET_ASCII, 0	/* can be changed with -c ... */
};
