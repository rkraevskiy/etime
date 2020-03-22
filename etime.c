/*-
 * Copyright (c) 2004-2020 Roman Kraievskyi <rkraevskiy@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */




#include <etime.h>
#include <stdint.h>

/*===================================================================*/
/* Configuration                                                     */
/*===================================================================*/

#ifndef ETIME_BASE_YEAR

/* UNIX epoch time */
#define ETIME_BASE_YEAR 70 /* 1970 ->  1970-1900=70 */
#define ETIME_EPOCH_WDAY 4 /* 1970.01.01 00:00:00 was Thursday */
#define ETIME_BASE_LEAP_DELTA 2 /* leap year is 1972 -> 1972-1970=2 */
#define ETIME_EPOCH_DELTA 0 /* no delta */


#endif


/*===================================================================*/
/* Constants                                                         */
/*===================================================================*/
#define SECS_PER_MIN 60
#define SECS_PER_HOUR (60*(SECS_PER_MIN))
#define SECS_PER_DAY (24*(SECS_PER_HOUR))

#define DAYS_PER_WEEK 7
#define DAYS_PER_YEAR 365
#define DAYS_PER_LEAP_YEAR 366
#define DAYS_PER_LEAP_CYCLE (3*(DAYS_PER_YEAR)+(DAYS_PER_LEAP_YEAR))
#define LEAP_YEARS_PERIOD  4 /* years */

#define BASE_LEAP_DELTA_DAYS ((ETIME_BASE_LEAP_DELTA)*(DAYS_PER_YEAR)) 

static const unsigned int      month_days[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static const unsigned int month_days_leap[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
/*===================================================================*/

#ifndef numberof
#define numberof(x) (sizeof(x)/(sizeof((x)[0])))
#endif

#ifndef etime_t
typedef unsigned long etime_t;
#define etime_t etime_t
#endif


/*
 * Convert timestamp value to broken-down time
 *
 * tm  [out] - result of conversion, see https://pubs.opengroup.org/onlinepubs/007908799/xsh/time.h.html
 * sec  [in] - timestamp
 */

void gmetime(struct tm *tm, etime_t sec)
{
	int leap;
	etime_t work;
   uint32_t days;
	const unsigned int *mdptr;
	int i;


	leap = 0;

   days = sec / SECS_PER_DAY;
   work = sec % SECS_PER_DAY;

   tm->tm_hour = work / SECS_PER_HOUR;
   work %= SECS_PER_HOUR;

   tm->tm_min = work / SECS_PER_MIN;
   tm->tm_sec = work % SECS_PER_MIN;

   tm->tm_wday = (ETIME_EPOCH_WDAY + days) % DAYS_PER_WEEK;

   tm->tm_year = ETIME_BASE_YEAR+(4*(days/DAYS_PER_LEAP_CYCLE));

   days = days % DAYS_PER_LEAP_CYCLE;

	if (days >= BASE_LEAP_DELTA_DAYS){
	   tm->tm_year += ETIME_BASE_LEAP_DELTA;

		days -= BASE_LEAP_DELTA_DAYS;
		if (days >= DAYS_PER_LEAP_YEAR){
			days -= DAYS_PER_LEAP_YEAR;
			tm->tm_year++;
		}else{
			leap = 1;
		}
	}

	if (!leap){
		tm->tm_year += days / DAYS_PER_YEAR;
		days %= DAYS_PER_YEAR;
		mdptr = month_days;
	}else{
		mdptr = month_days_leap;
	}
	
	tm->tm_yday = days;
   i = 0;
   while (i < (int)numberof(month_days) && days >= mdptr[i]){
		i++;
   }
	i--;
   tm->tm_mday = days+1-mdptr[i];
	tm->tm_mon = i;
	tm->tm_isdst = 0;
}

/*
 * Create timestamp from year, month, etc...
 *
 * year [in]  - years since 1900
 * mon  [in]  - month of year [0,11]
 * day  [in]  - day of month [1,31]
 * hour [in]  - hour [0,23]
 * min  [in]  - minutes [0,59]
 * sec  [in]  - seconds [0,61]
 *
 * return     - timestamp relative to epoch start ETIME_BASE_YEAR
 */
etime_t mketime(int year, int mon, int day, int hour, int min, int sec)
{
   etime_t days;
	int y = year - ETIME_BASE_YEAR;

	days = y * DAYS_PER_YEAR + ((y)/LEAP_YEARS_PERIOD);
	

	y = (y%LEAP_YEARS_PERIOD);


	if (y == ETIME_BASE_LEAP_DELTA){
		/* leap */
		if (mon>1){
			days++;
		}
	}else if (y>ETIME_BASE_LEAP_DELTA){
		days++;
	}

	/* printf("mon:%d leap:%d day:%d y:%d days:%d\n",mon,leap,day,y,days); */
	days += month_days[mon];

	days+=day-1;
	
	days = ((days*24+hour)*60+min)*60+sec;

	return days;
}

//#define TIME_DEBUG_TEST

#ifdef TIME_DEBUG_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static void print_time(struct tm *tm){
	char str[1024];

	strftime(str,sizeof(str),"%Y.%m.%d %H:%M.%S %j %w",tm);
	printf("%s\n",str);
}


static int check(struct tm *tm1, struct tm *tm2)
{
	int errors = 0;

	if (tm1->tm_sec != tm2->tm_sec){
		printf(" tm->tm_sec %d vs %d ",tm1->tm_sec,tm2->tm_sec);
		errors++;
	}

	if (tm1->tm_min != tm2->tm_min){
		printf(" tm->tm_min %d vs %d ",tm1->tm_min,tm2->tm_min);
		errors++;
	}

	if (tm1->tm_hour != tm2->tm_hour){
		printf(" tm->tm_hour %d vs %d ",tm1->tm_hour,tm2->tm_hour);
		errors++;
	}
	
	if (tm1->tm_mday != tm2->tm_mday){
		printf(" tm->tm_mday %d vs %d ",tm1->tm_mday,tm2->tm_mday);
		errors++;
	}
	
	if (tm1->tm_mon != tm2->tm_mon){
		printf(" tm->tm_mon %d vs %d ",tm1->tm_mon,tm2->tm_mon);
		errors++;
	}

	if (tm1->tm_year != tm2->tm_year){
		printf(" tm->tm_year %d vs %d ",tm1->tm_year,tm2->tm_year);
		errors++;
	}
	
	if (tm1->tm_wday != tm2->tm_wday){
		printf(" tm->tm_wday %d vs %d ",tm1->tm_wday,tm2->tm_wday);
		errors++;
	}

	if (tm1->tm_yday != tm2->tm_yday){
		printf(" tm->tm_yday %d vs %d ",tm1->tm_yday,tm2->tm_yday);
		errors++;
	}

	if (errors){
		printf("\n");
	}
	return 0!=errors;
}


int main(int argc, char *argv[])
{
	struct tm tm1;
	struct tm *tm2;
	time_t time2;
	etime_t time1 = 0;
	etime_t time3;
	long i = 0;

	srand(time(NULL));

/*	gmetime(&tm1,48184971);
	print_time(&tm1);
	time3 = mketime(tm1.tm_year,tm1.tm_mon,tm1.tm_mday,tm1.tm_hour,tm1.tm_min,tm1.tm_sec);
	gmetime(&tm1,time3);
	print_time(&tm1);
	return 0; */

	while(i<100000000){
		time1 = rand();
		time2 = time1 + ETIME_EPOCH_DELTA;

		gmetime(&tm1,time1);
		tm2 = gmtime(&time2);
		if (check(&tm1, tm2)){
			printf("Error detected for %ld iteration #%ld\n", time1, i);
			print_time(&tm1);
			print_time(tm2);
			break;
		}
		time3 = mketime(tm2->tm_year,tm2->tm_mon,tm2->tm_mday,tm2->tm_hour,tm2->tm_min,tm2->tm_sec);
		if (time3 != time1){
			printf("Error detected for %ld vs %ld iteration #%ld\n",time1,time3,i);
			print_time(&tm1);
			gmetime(&tm1,time3);
			print_time(&tm1);
			break;
		}
		time1++;
		i++;
	}
	return 0;
}


#endif


