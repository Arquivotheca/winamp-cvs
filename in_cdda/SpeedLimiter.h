#ifndef NULLSOFT_SPEEDLIMITERH
#define NULLSOFT_SPEEDLIMITERH
#include <windows.h>
class SpeedLimiter
{
public:
	SpeedLimiter()
	{
		Reset();
	}

	void Reset()
	{
		total_extract_len = 0;
		extract_start_time = 0;
		limited = false;
	}

	void SetLimit(int two_pow_x)
	{
		do_limit = two_pow_x;
		limited = true;
	}
	void NoLimit()
	{
		limited = false;
	}
	void Start()
	{
		extract_start_time = GetTickCount();
	}

	void Limit(int *killswitch)
	{
		if (!extract_start_time)
			Start();

		if (limited)
		{
			for (;;)
			{
				DWORD now = GetTickCount() - extract_start_time;
				if (!now) now = 1; //extra safe mode :)
				double kbspeed = (((double)total_extract_len * 1000.0) / (double)now);
				double speed = (double) (88200 << (do_limit - 1));
				if (*killswitch || kbspeed <= speed) break;
				Sleep(1);
			}
		}
	}

	void MoreBytesRead(int delta_extract_len)
	{
		total_extract_len += delta_extract_len;
	}

private:
	int total_extract_len;
	DWORD extract_start_time;
	int do_limit;
	bool limited;
};

#endif
