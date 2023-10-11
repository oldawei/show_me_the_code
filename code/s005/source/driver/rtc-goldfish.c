#include <stddef.h>
#include <string.h>
#include "types.h"
#include "riscv-virt.h"
#include "rtc-goldfish.h"

/*
 * TIMER_TIME_LOW	     get low bits of current time and update TIMER_TIME_HIGH
 * TIMER_TIME_HIGH	     get high bits of time at last TIMER_TIME_LOW read
 * TIMER_ALARM_LOW	     set low bits of alarm and activate it
 * TIMER_ALARM_HIGH	     set high bits of next alarm
 * TIMER_IRQ_ENABLED	 enable alarm interrupt
 * TIMER_CLEAR_ALARM	 disarm an existing alarm
 * TIMER_ALARM_STATUS	 alarm status (running or not)
 * TIMER_CLEAR_INTERRUPT clear interrupt
 */
#define TIMER_TIME_LOW		    0x00
#define TIMER_TIME_HIGH		    0x04
#define TIMER_ALARM_LOW		    0x08
#define TIMER_ALARM_HIGH	    0x0c
#define TIMER_IRQ_ENABLED	    0x10
#define TIMER_CLEAR_ALARM	    0x14
#define TIMER_ALARM_STATUS	    0x18
#define TIMER_CLEAR_INTERRUPT	0x1c

#define RTC_ADDR(addr)          (gs_rtc_base + (addr))
#define RTC_REG(reg) 			(*((volatile u32 *)RTC_ADDR(reg)))

static u32 gs_rtc_base = 0;
void goldfish_rtc_init(u32 base_addr)
{
	gs_rtc_base = base_addr;
}

int goldfish_rtc_read_alarm(struct rtc_wkalrm *alrm)
{
	u64 rtc_alarm;
	u64 rtc_alarm_low;
	u64 rtc_alarm_high;

	rtc_alarm_low = RTC_REG(TIMER_ALARM_LOW);
	rtc_alarm_high = RTC_REG(TIMER_ALARM_HIGH);
	rtc_alarm = (rtc_alarm_high << 32) | rtc_alarm_low;

    rtc_alarm /= NSEC_PER_SEC;
	memset(alrm, 0, sizeof(struct rtc_wkalrm));

	rtc_time64_to_tm(rtc_alarm, &alrm->time);

	if (RTC_REG(TIMER_ALARM_STATUS))
		alrm->enabled = 1;
	else
		alrm->enabled = 0;

	return 0;
}

int goldfish_rtc_set_alarm(struct rtc_wkalrm *alrm)
{
	u64 rtc_alarm64;
	u64 rtc_status_reg;

	if (alrm->enabled) {
		rtc_alarm64 = rtc_tm_to_time64(&alrm->time) * NSEC_PER_SEC;
		RTC_REG(TIMER_ALARM_HIGH) = (u32)(rtc_alarm64 >> 32);
		RTC_REG(TIMER_ALARM_LOW) = (u32)rtc_alarm64;
		RTC_REG(TIMER_IRQ_ENABLED) = 1;
	} else {
		/*
		 * if this function was called with enabled=0
		 * then it could mean that the application is
		 * trying to cancel an ongoing alarm
		 */
		rtc_status_reg = RTC_REG(TIMER_ALARM_STATUS);
		if (rtc_status_reg)
			RTC_REG(TIMER_CLEAR_ALARM) = 1;
	}

	return 0;
}

int goldfish_rtc_alarm_irq_enable(unsigned int enabled)
{
	RTC_REG(TIMER_IRQ_ENABLED) = enabled ? 1 : 0;
	return 0;
}

int goldfish_rtc_interrupt(void)
{
	RTC_REG(TIMER_CLEAR_INTERRUPT) = 1;

	return 0;
}

int goldfish_rtc_read_time(struct rtc_time *tm)
{
	u64 time_high;
	u64 time_low;
	u64 time;

	time_low = RTC_REG(TIMER_TIME_LOW);
	time_high = RTC_REG(TIMER_TIME_HIGH);
	time = (time_high << 32) | time_low;

    time /= NSEC_PER_SEC;
	rtc_time64_to_tm(time, tm);

	return 0;
}

u64 goldfish_rtc_read_time_sec(void)
{
	u64 time_high;
	u64 time_low;
	u64 time;

	time_low = RTC_REG(TIMER_TIME_LOW);
	time_high = RTC_REG(TIMER_TIME_HIGH);
	time = (time_high << 32) | time_low;

    time /= NSEC_PER_SEC;

	return time;
}

int goldfish_rtc_set_time(struct rtc_time *tm)
{
	u64 now64;

	now64 = rtc_tm_to_time64(tm) * NSEC_PER_SEC;
	RTC_REG(TIMER_TIME_HIGH) = (u32)(now64 >> 32);
	RTC_REG(TIMER_TIME_LOW) = (u32)now64;

	return 0;
}

/**
 * rtc_time64_to_tm - converts time64_t to rtc_time.
 *
 * @time:	The number of seconds since 01-01-1970 00:00:00.
 *		(Must be positive.)
 * @tm:		Pointer to the struct rtc_time.
 */
void rtc_time64_to_tm(time64_t time, struct rtc_time *tm)
{
	unsigned int secs;
	int days;

	u64 u64tmp;
	u32 u32tmp, udays, century, day_of_century, year_of_century, year,
		day_of_year, month, day;
	bool is_Jan_or_Feb, is_leap_year;

	/* time must be positive */
    days = time / 86400;
    secs = time - 86400 * days;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	/*
	 * The following algorithm is, basically, Proposition 6.3 of Neri
	 * and Schneider [1]. In a few words: it works on the computational
	 * (fictitious) calendar where the year starts in March, month = 2
	 * (*), and finishes in February, month = 13. This calendar is
	 * mathematically convenient because the day of the year does not
	 * depend on whether the year is leap or not. For instance:
	 *
	 * March 1st		0-th day of the year;
	 * ...
	 * April 1st		31-st day of the year;
	 * ...
	 * January 1st		306-th day of the year; (Important!)
	 * ...
	 * February 28th	364-th day of the year;
	 * February 29th	365-th day of the year (if it exists).
	 *
	 * After having worked out the date in the computational calendar
	 * (using just arithmetics) it's easy to convert it to the
	 * corresponding date in the Gregorian calendar.
	 *
	 * [1] "Euclidean Affine Functions and Applications to Calendar
	 * Algorithms". https://arxiv.org/abs/2102.06959
	 *
	 * (*) The numbering of months follows rtc_time more closely and
	 * thus, is slightly different from [1].
	 */

	udays		= ((u32) days) + 719468;

	u32tmp		= 4 * udays + 3;
	century		= u32tmp / 146097;
	day_of_century	= u32tmp % 146097 / 4;

	u32tmp		= 4 * day_of_century + 3;
	u64tmp		= 2939745ULL * u32tmp;
	year_of_century	= upper_32_bits(u64tmp);
	day_of_year	= lower_32_bits(u64tmp) / 2939745 / 4;

	year		= 100 * century + year_of_century;
	is_leap_year	= year_of_century != 0 ?
		year_of_century % 4 == 0 : century % 4 == 0;

	u32tmp		= 2141 * day_of_year + 132377;
	month		= u32tmp >> 16;
	day		= ((u16) u32tmp) / 2141;

	/*
	 * Recall that January 01 is the 306-th day of the year in the
	 * computational (not Gregorian) calendar.
	 */
	is_Jan_or_Feb	= day_of_year >= 306;

	/* Converts to the Gregorian calendar. */
	year		= year + is_Jan_or_Feb;
	month		= is_Jan_or_Feb ? month - 12 : month;
	day		= day + 1;

	day_of_year	= is_Jan_or_Feb ?
		day_of_year - 306 : day_of_year + 31 + 28 + is_leap_year;

	/* Converts to rtc_time's format. */
	tm->tm_year	= (int) (year - 1900);
	tm->tm_mon	= (int) month;
	tm->tm_mday	= (int) day;
	tm->tm_yday	= (int) day_of_year + 1;

	tm->tm_hour = secs / 3600;
	secs -= tm->tm_hour * 3600;
	tm->tm_min = secs / 60;
	tm->tm_sec = secs - tm->tm_min * 60;

	tm->tm_isdst = 0;
}

/*
 * rtc_tm_to_time64 - Converts rtc_time to time64_t.
 * Convert Gregorian date to seconds since 01-01-1970 00:00:00.
 */
time64_t rtc_tm_to_time64(struct rtc_time *tm)
{
	return mktime64(((unsigned int)tm->tm_year + 1900), tm->tm_mon + 1,
			tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

/*
 * mktime64 - Converts date to seconds.
 * Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * A leap second can be indicated by calling this function with sec as
 * 60 (allowable under ISO 8601).  The leap second is treated the same
 * as the following second since they don't exist in UNIX time.
 *
 * An encoding of midnight at the end of the day as 24:00:00 - ie. midnight
 * tomorrow - (allowable under ISO 8601) is supported.
 */
time64_t mktime64(const unsigned int year0, const unsigned int mon0,
		const unsigned int day, const unsigned int hour,
		const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((time64_t)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours - midnight tomorrow handled here */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}
