#ifndef __RTC_GOLDFISH_H__
#define __RTC_GOLDFISH_H__

#include "stdint.h"
#include "types.h"

#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define FSEC_PER_SEC	1000000000000000LL

typedef uint64_t time64_t;

/*
 * The struct used to pass data via the following ioctl. Similar to the
 * struct tm in <time.h>, but it needs to be here so that the kernel
 * source is self contained, allowing cross-compiles, etc. etc.
 */

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

/*
 * This data structure is inspired by the EFI (v0.92) wakeup
 * alarm API.
 */
struct rtc_wkalrm {
	unsigned char enabled;	/* 0 = alarm disabled, 1 = alarm enabled */
	unsigned char pending;  /* 0 = alarm not pending, 1 = alarm pending */
	struct rtc_time time;	/* time the alarm is set to */
};

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((u32)((n) & 0xffffffff))


void rtc_time64_to_tm(time64_t time, struct rtc_time *tm);
time64_t rtc_tm_to_time64(struct rtc_time *tm);
time64_t mktime64(const unsigned int year0, const unsigned int mon0,
		const unsigned int day, const unsigned int hour,
		const unsigned int min, const unsigned int sec);

void goldfish_rtc_init(u32 base_addr);
int goldfish_rtc_read_alarm(struct rtc_wkalrm *alrm);
int goldfish_rtc_set_alarm(struct rtc_wkalrm *alrm);
int goldfish_rtc_alarm_irq_enable(unsigned int enabled);
int goldfish_rtc_interrupt(void);
int goldfish_rtc_read_time(struct rtc_time *tm);
u64 goldfish_rtc_read_time_sec(void);
int goldfish_rtc_set_time(struct rtc_time *tm);

#define YEAR(year)	((year) + 1900)
#define MONTH(mon)	((mon) + 1)
#define HOUR(hr)	((hr) + 8) // 东8区

#endif /* __RTC_GOLDFISH_H__ */
