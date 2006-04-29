/*
    Interpolate filter

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
#include "filterdefs.h"
#include "grtcirc.h"

#define MYNAME "Interpolate filter"

static char *opt_interval = NULL;
int interval = 0;
static char *opt_dist = NULL;
double dist = 0;

static
arglist_t interpfilt_args[] = {
	{"time", &opt_interval, "Time interval in seconds", NULL, 
		ARGTYPE_BEGIN_EXCL | ARGTYPE_BEGIN_REQ | ARGTYPE_INT, 
		"0", NULL },
	{"distance", &opt_dist, "Distance interval in miles or kilometers", 
		NULL, ARGTYPE_END_EXCL | ARGTYPE_END_REQ | ARGTYPE_STRING, 
		ARG_NOMINMAX },
	ARG_TERMINATOR
};

void 
interpfilt_process(void)
{
	queue *backuproute = NULL;
	queue *elem, *tmp, *elem2, *tmp2;
	route_head *rte_new;
	int count = 0;
	int first = 0;
	double lat1, lon1;
	int time1;
	int timen;
	double distn;
	double curdist;
	double rt1, rn1, rt2, rn2;
	
	track_backup( &count, &backuproute );
        route_flush_all_tracks();
        QUEUE_FOR_EACH( backuproute, elem, tmp )
	{
		route_head *rte_old = (route_head *)elem;
		
		rte_new = route_head_alloc();
		rte_new->rte_name = xstrdup( rte_old->rte_name );
		rte_new->rte_desc = xstrdup( rte_old->rte_desc );
		rte_new->fs = fs_chain_copy( rte_old->fs );
		rte_new->rte_num = rte_old->rte_num;
		track_add_head( rte_new );
		first = 1;
		QUEUE_FOR_EACH( &rte_old->waypoint_list, elem2, tmp2 ) 
		{
		    waypoint *wpt = (waypoint *)elem2;
		    if ( first ) {
			first = 0;
		    }
		    else {
			if ( opt_interval && 
			        wpt->creation_time - time1 > interval ) {
			    for ( timen = time1+interval; 
					timen < wpt->creation_time;
					timen += interval ) {
				waypoint *wpt_new = waypt_dupe(wpt);
				wpt_new->creation_time = timen;
				if (wpt_new->shortname) xfree(wpt_new->shortname);
				if (wpt_new->description) xfree(wpt_new->description);
				wpt_new->shortname = wpt_new->description = NULL;
				linepart( lat1, lon1, 
					  wpt->latitude, wpt->longitude,
					  (double)(timen-time1)/
					  (double)(wpt->creation_time-time1),
					  &wpt_new->latitude,
					  &wpt_new->longitude );
				track_add_wpt( rte_new, wpt_new);
			    }
			}
			else if ( opt_dist ) {
			    rt1 = lat1 * M_PI / 180;
			    rn1 = lon1 * M_PI / 180;
			    rt2 = wpt->latitude * M_PI / 180;
			    rn2 = wpt->longitude * M_PI / 180;
			    curdist = gcdist( rt1, rn1, rt2, rn2 );
		            curdist = radtomiles(curdist);
			    if ( curdist > dist ) {	
  			      for ( distn = dist; 
					distn < curdist;
					distn += dist ) {
				waypoint *wpt_new = waypt_dupe(wpt);
				wpt_new->creation_time = distn/curdist*
					(wpt->creation_time - time1) + time1;
				if (wpt_new->shortname) xfree(wpt_new->shortname);
				if (wpt_new->description) xfree(wpt_new->description);
				wpt_new->shortname = wpt_new->description = NULL;
				linepart( lat1, lon1, 
					  wpt->latitude, wpt->longitude,
					  distn/curdist,
					  &wpt_new->latitude,
					  &wpt_new->longitude );
				track_add_wpt( rte_new, wpt_new);
			      }
			    }
			}
		    }
		    track_add_wpt( rte_new, waypt_dupe(wpt));
		    
		    lat1 = wpt->latitude;
		    lon1 = wpt->longitude;
		    time1 = wpt->creation_time;
		}
	}
	route_flush( backuproute );
	xfree( backuproute );
}

void
interpfilt_init(const char *args) {
	
        char *fm;
	if ( opt_interval && opt_dist ) {
		fatal( MYNAME ": Can't interpolate on both time and distance.\n");
	}
	else if ( opt_interval ) {
		interval = atoi(opt_interval);
	}
	else if ( opt_dist ) {
                dist = strtod(opt_dist, &fm);
		if ((*fm == 'k') || (*fm == 'K')) {
	             /* distance is kilometers, convert to miles */
	             dist *= .6214;
		}
	}
	else {
		fatal( MYNAME ": No interval specified.\n");
	}
}

filter_vecs_t interpolatefilt_vecs = {
	interpfilt_init,
	interpfilt_process,
	NULL,
	NULL,
	interpfilt_args
};
