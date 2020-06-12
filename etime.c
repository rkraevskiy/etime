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
#define ETIME_BASE_YEAR 1970 /* 1970 ->  1970-1900=70 */
#define ETIME_EPOCH_WDAY 4 /* 1970.01.01 00:00:00 was Thursday */
#define ETIME_BASE_LEAP_DELTA 2 /* leap year is 1972 -> 1972-1970=2 */
#define ETIME_EPOCH_DELTA 0 /* no delta */

#warning "1970.01.01 00:00:00 is used as a timebase"
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
#define DAYS_PER_COMMON_CYCLE (4*(DAYS_PER_YEAR))
#define LEAP_YEARS_PERIOD   4 /* years */
#define CYCLE_100_YEARS  100 /* years */
#define CYCLE_100_DAYS ((24*DAYS_PER_LEAP_YEAR) + (76*DAYS_PER_YEAR))
#define CYCLE_400_YEARS  400 /* years */
#define CYCLE_400_DAYS  (CYCLE_100_DAYS * 4 + 1) /* (24 leap years per 100 years * 366 + 76*365) *4 + 1  */
#define ETIME_BASE_YEAR_DELTA ((ETIME_BASE_YEAR)-1900)


#define BASE_LEAP_DELTA_DAYS ((ETIME_BASE_LEAP_DELTA)*(DAYS_PER_YEAR)) 

#define YEARS_TO_100_CYCLE (((((ETIME_BASE_YEAR_DELTA)+CYCLE_100_YEARS-1)/CYCLE_100_YEARS)*CYCLE_100_YEARS)-(ETIME_BASE_YEAR_DELTA))

#define YEARS_TO_400_CYCLE  ((((300+(ETIME_BASE_YEAR_DELTA)+CYCLE_400_YEARS-1)/CYCLE_400_YEARS)*CYCLE_400_YEARS)-300-(ETIME_BASE_YEAR_DELTA))

#define DAYS_TO_400_CYCLE   (((YEARS_TO_400_CYCLE)/LEAP_YEARS_PERIOD)*DAYS_PER_LEAP_CYCLE + ((YEARS_TO_400_CYCLE)%LEAP_YEARS_PERIOD)*DAYS_PER_YEAR - (YEARS_TO_400_CYCLE)/(CYCLE_100_YEARS))

#define DAYS_TO_100_CYCLE ( ((YEARS_TO_100_CYCLE)/LEAP_YEARS_PERIOD)*DAYS_PER_LEAP_CYCLE + ((YEARS_TO_100_CYCLE)%LEAP_YEARS_PERIOD)*DAYS_PER_YEAR )



static const unsigned int      month_days[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static const unsigned int month_days_leap[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
/*===================================================================*/

#ifndef numberof
#define numberof(x) (sizeof(x)/(sizeof((x)[0])))
#endif

#ifndef etime_t
typedef unsigned long long etime_t;
#define etime_t etime_t
#endif


//#define IFOPT(x) if (x)
#define IFOPT(x)

#include <stdio.h>


#ifndef ETIME_LOG_DEBUG
#define ETIME_LOG_DEBUG(...)
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
	int alligned = 0;
	etime_t work;
	etime_t days;
	const unsigned int *mdptr;
	int c100 = 0; // 100 year cycle begining
	int c400 = 0; // 400 year cycle begining
	int i;

	leap = 0;

	days = sec / SECS_PER_DAY;
	work = sec % SECS_PER_DAY;

	tm->tm_hour = work / SECS_PER_HOUR;
	work %= SECS_PER_HOUR;

	tm->tm_min = work / SECS_PER_MIN;
	tm->tm_sec = work % SECS_PER_MIN;

	tm->tm_wday = ((ETIME_EPOCH_WDAY) + days) % DAYS_PER_WEEK;

	tm->tm_year = (ETIME_BASE_YEAR_DELTA)+300;

	ETIME_LOG_DEBUG("=[%lld]======================\n",sec);
	ETIME_LOG_DEBUG("%d days:%lld y:%d\n",__LINE__,days,tm->tm_year);


#if DAYS_TO_400_CYCLE > 0
	if (days >= DAYS_TO_400_CYCLE){
		c100 = 1;
		c400 = 1;
		alligned = 1;
		days -= DAYS_TO_400_CYCLE;
		tm->tm_year +=	YEARS_TO_400_CYCLE;
		ETIME_LOG_DEBUG("%d DAYS_TO_400_CYCLE days:%lld y:%d\n",__LINE__,days,tm->tm_year);
	}
#else
	if (1) {
		c100=1;
		c400=1;
		alligned = 1;
	}
#endif

#if (DAYS_TO_100_CYCLE > 0) && (DAYS_TO_400_CYCLE > 0)
	else if (days >= DAYS_TO_100_CYCLE){
		days -= DAYS_TO_100_CYCLE;
		tm->tm_year +=	YEARS_TO_100_CYCLE;
		ETIME_LOG_DEBUG("%d DAYS_TO_100_CYCLE days:%lld y:%d\n",__LINE__,days,tm->tm_year);
		c100 = 1;
		alligned = 1;
	}
#else
	if (1) {
		c100 = 1;
		alligned = 1;
	}
#endif

#if (BASE_LEAP_DELTA_DAYS) > 0 && (DAYS_TO_100_CYCLE > 0) && (DAYS_TO_400_CYCLE > 0)
	else if (days >= BASE_LEAP_DELTA_DAYS){
		days -= BASE_LEAP_DELTA_DAYS;
		tm->tm_year +=	ETIME_BASE_LEAP_DELTA;
		ETIME_LOG_DEBUG("%d days:%lld y:%d\n",__LINE__,days,tm->tm_year);
		alligned = 1;
	}
#else
	else {
		alligned = 1;
	}
#endif


	ETIME_LOG_DEBUG("%d days:%lld y:%d\n",__LINE__,days,tm->tm_year);

	IFOPT(days >=CYCLE_400_DAYS){
		/* iterate over all 400 years cycles */
		tm->tm_year +=	(days/CYCLE_400_DAYS)*CYCLE_400_YEARS;
		days %= CYCLE_400_DAYS;
	}

	ETIME_LOG_DEBUG("%d CYCLE_400_DAYS days:%lld y:%d\n",__LINE__,days,tm->tm_year);

	/* only 100 years cycles are left */
	if (days > CYCLE_100_DAYS){
		if (c400){
			days --;
		}
		tm->tm_year +=	(days/CYCLE_100_DAYS)*CYCLE_100_YEARS;
		days %= CYCLE_100_DAYS;
		c400 = 0;
	}


	ETIME_LOG_DEBUG("%d  l100 days:%lld y:%d l:%d c100:%d c400:%d\n",__LINE__,days,tm->tm_year,leap,c100,c400);

	/* adjust first "leap" years of 100 years cycle */
	if (c100 && !c400 && days>=DAYS_PER_COMMON_CYCLE){
		days -= DAYS_PER_COMMON_CYCLE;
		tm->tm_year += LEAP_YEARS_PERIOD;
		c100 = 0;
	}

	ETIME_LOG_DEBUG("%d LEAP_YEARS_PERIOD l100 days:%lld y:%d l:%d c100:%d c400:%d\n",__LINE__,days,tm->tm_year,leap,c100,c400);
	/* adjust with leap cycles */
	if(days >= DAYS_PER_LEAP_CYCLE){
		tm->tm_year +=	(days/DAYS_PER_LEAP_CYCLE)*LEAP_YEARS_PERIOD;
		days %= DAYS_PER_LEAP_CYCLE;
		c100 = 0;
		c400 = 0;
	}

	ETIME_LOG_DEBUG("%d DAYS_PER_LEAP_CYCLE days:%lld y:%d\n",__LINE__,days,tm->tm_year);

	if (alligned && ((c400) || (!c100))){
		if  (days >= DAYS_PER_LEAP_YEAR){
			days -= DAYS_PER_LEAP_YEAR;
			tm->tm_year++;
			leap = 0;
		}else{
			leap = 1;
		}
	}

	if ( leap ) {
		ETIME_LOG_DEBUG("%d leap days:%lld y:%d\n",__LINE__,days,tm->tm_year);
		mdptr = month_days_leap;
	}else{
		ETIME_LOG_DEBUG("%d common days:%lld y:%d +%lld\n",__LINE__,days,tm->tm_year,days / DAYS_PER_YEAR);
		IFOPT(days>=DAYS_PER_YEAR){
			tm->tm_year += days / DAYS_PER_YEAR;
			days %= DAYS_PER_YEAR;
		}
		mdptr = month_days;
	}

	ETIME_LOG_DEBUG("%d days:%lld y:%d\n",__LINE__,days,tm->tm_year);
	tm->tm_yday = days;
	i = 0;
	while (i < (int)numberof(month_days) && days >= mdptr[i]){
		i++;
	}
	i--;
	tm->tm_mday = days+1-mdptr[i];
	tm->tm_mon = i;
	tm->tm_isdst = 0;
	tm->tm_year -= 300;
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
   etime_t days = 0;
	int y = year - ETIME_BASE_YEAR_DELTA;
	int c400 = 0;
	int c100 = 0;
	int alligned = 0;


	ETIME_LOG_DEBUG("===============\n");
	ETIME_LOG_DEBUG("Y:%d m:%d d:%d\n",y,mon+1,day);

	if (y >= YEARS_TO_400_CYCLE){
		y -= YEARS_TO_400_CYCLE;
		days += DAYS_TO_400_CYCLE;
		alligned = 1;
		c100 =1;
		c400 = 1;
		ETIME_LOG_DEBUG("YEARS_TO_400_CYCLE y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}else if (y >= YEARS_TO_100_CYCLE){
		y -= YEARS_TO_100_CYCLE;
		days += DAYS_TO_100_CYCLE;
		c100 =1;
		ETIME_LOG_DEBUG("YEARS_TO_400_CYCLE y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}else if (y >= ETIME_BASE_LEAP_DELTA){
		y -= ETIME_BASE_LEAP_DELTA;
		days += BASE_LEAP_DELTA_DAYS;
		alligned = 1;
		ETIME_LOG_DEBUG("YEARS_TO_400_CYCLE y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}


	IFOPT(y>=CYCLE_400_YEARS){
		days += (y/CYCLE_400_YEARS)*CYCLE_400_DAYS;
		y = (y%CYCLE_400_YEARS);
		ETIME_LOG_DEBUG("YEARS_TO_400_CYCLE y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}



	if (y >= CYCLE_100_YEARS){
		days += (y/CYCLE_100_YEARS)*CYCLE_100_DAYS;
		y = (y%CYCLE_100_YEARS);
		if (c400){
			days ++;
			c400 = 0;
		}
		alligned = 0;
		ETIME_LOG_DEBUG("CYCLE_100_YEARS y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}



	if (c100 && !c400 && y >= LEAP_YEARS_PERIOD){
		y-= LEAP_YEARS_PERIOD;
		days += DAYS_PER_COMMON_CYCLE;
		c100 = 0;
		alligned = 1;
		ETIME_LOG_DEBUG("CYCLE_100_YEARS2 y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}



	days += y * DAYS_PER_YEAR;
	ETIME_LOG_DEBUG("DAYS_PER_YEAR y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);

	if(y>=LEAP_YEARS_PERIOD){
		days += ((y)/LEAP_YEARS_PERIOD);
		y = (y%LEAP_YEARS_PERIOD);
		alligned = 1;
		ETIME_LOG_DEBUG("LEAP_YEARS_PERIOD y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
	}


	if (alligned){
		if ((y==0 && mon > 1) || y > 0 ){
			ETIME_LOG_DEBUG("ALLIGNED +1day y:%d days:%lld alligned:%d c100:%d c400:%d\n",y,days,alligned,c100,c400);
			days += 1;
		}
	}

	days += month_days[mon];
	ETIME_LOG_DEBUG("mon:%d day:%d y:%d days:%lld\n",mon,day,y,days);

	days+=day-1;

	days = ((days*24+hour)*60+min)*60+sec;

	return days;
}


#ifdef ETIME_DEBUG_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifndef ETIME_TEST_LOG
#define ETIME_TEST_LOG(...)  printf(__VA_ARGS__)
#endif

static void print_time(struct tm *tm){
	char str[1024];

	//strftime(str,sizeof(str),"%Y.%m.%d %H:%M.%S %j %w",tm);
	snprintf(str,sizeof(str),"%04d.%02d.%02d %02d:%02d:%02d %d",
			tm->tm_year+1900,
			tm->tm_mon+1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			tm->tm_yday
			);

	ETIME_TEST_LOG("%s\n",str);
}


static int check_tm(struct tm *tm1, struct tm *tm2)
{
	int errors = 0;

	if (tm1->tm_sec != tm2->tm_sec){
		ETIME_TEST_LOG(" tm->tm_sec %d vs %d ",tm1->tm_sec,tm2->tm_sec);
		errors++;
	}

	if (tm1->tm_min != tm2->tm_min){
		ETIME_TEST_LOG(" tm->tm_min %d vs %d ",tm1->tm_min,tm2->tm_min);
		errors++;
	}

	if (tm1->tm_hour != tm2->tm_hour){
		ETIME_TEST_LOG(" tm->tm_hour %d vs %d ",tm1->tm_hour,tm2->tm_hour);
		errors++;
	}
	
	if (tm1->tm_mday != tm2->tm_mday){
		ETIME_TEST_LOG(" tm->tm_mday %d vs %d ",tm1->tm_mday,tm2->tm_mday);
		errors++;
	}
	
	if (tm1->tm_mon != tm2->tm_mon){
		ETIME_TEST_LOG(" tm->tm_mon %d vs %d ",tm1->tm_mon,tm2->tm_mon);
		errors++;
	}

	if (tm1->tm_year != tm2->tm_year){
		ETIME_TEST_LOG(" tm->tm_year %d vs %d ",tm1->tm_year,tm2->tm_year);
		errors++;
	}
	
	if (tm1->tm_wday != tm2->tm_wday){
		ETIME_TEST_LOG(" tm->tm_wday %d vs %d ",tm1->tm_wday,tm2->tm_wday);
		errors++;
	}

	if (0 && tm1->tm_yday != tm2->tm_yday){
		ETIME_TEST_LOG(" tm->tm_yday %d vs %d ",tm1->tm_yday,tm2->tm_yday);
		errors++;
	}

	if (errors){
		ETIME_TEST_LOG("\n");
	}
	return (0 != errors);
}

static int check_time(etime_t time1, long i)
{
	struct tm tm1;
	struct tm *tm2;
	time_t time2;
	etime_t time3;

	time2 = time1 + ETIME_EPOCH_DELTA;

	gmetime(&tm1,time1);
	tm2 = gmtime(&time2);
	if (check_tm(&tm1, tm2)){
		ETIME_TEST_LOG("Error detected for %lld iteration #%ld\n", time1, i);
		ETIME_TEST_LOG("  etime: ");
		print_time(&tm1);
		ETIME_TEST_LOG("correct: ");
		print_time(tm2);
		return 0;
	}

	time3 = mketime(tm2->tm_year,tm2->tm_mon,tm2->tm_mday,tm2->tm_hour,tm2->tm_min,tm2->tm_sec);
	if (time3 != time1){
		ETIME_TEST_LOG("Error detected for %lld vs %lld iteration #%ld\n",time1,time3,i);
		ETIME_TEST_LOG("  etime: ");
		print_time(&tm1);
		gmetime(&tm1,time3);
		ETIME_TEST_LOG("correct: ");
		print_time(&tm1);
		return 0;
	}
	return !0;
}

int main(int argc, char *argv[])
{
	etime_t time1 = 0;
	etime_t time2 = 0;
	long i = 0;

	srand(time(NULL));


	ETIME_TEST_LOG("ETIME_BASE_YEAR %d\n",ETIME_BASE_YEAR);
	ETIME_TEST_LOG("YEARS_TO_100_CYCLE %d\n",YEARS_TO_100_CYCLE);
	ETIME_TEST_LOG("DAYS_TO_100_CYCLE %d\n",DAYS_TO_100_CYCLE);
	ETIME_TEST_LOG("DAYS_TO_100_CYCLE*x %d\n",DAYS_TO_100_CYCLE*60*60*24);
	ETIME_TEST_LOG("YEARS_TO_400_CYCLE %d\n",YEARS_TO_400_CYCLE);
	ETIME_TEST_LOG("DAYS_TO_400_CYCLE %d\n",DAYS_TO_400_CYCLE);
	ETIME_TEST_LOG("CYCLE_400_DAYS %d\n",CYCLE_400_DAYS);
	ETIME_TEST_LOG("CYCLE_100_DAYS %d\n",CYCLE_100_DAYS);
	ETIME_TEST_LOG("DAYS_PER_LEAP_CYCLE %d\n",DAYS_PER_LEAP_CYCLE);
	ETIME_TEST_LOG("BASE_LEAP_DELTA_DAYS %d\n",BASE_LEAP_DELTA_DAYS);
	ETIME_TEST_LOG("=================\n");

	while(i<100000000){
		time1 = ((etime_t)rand())*(1+rand()%1000);
		if (!check_time(time1,i)){
			ETIME_TEST_LOG("Problems were found\n");
			return 1;
		}

		time2+=SECS_PER_DAY-3;
		if (!check_time(time2,i)){
			ETIME_TEST_LOG("Problems were found\n");
			return 1;
		}
		i++;
	}
	ETIME_TEST_LOG("No problems were found\n");
	return 0;
}


#endif


