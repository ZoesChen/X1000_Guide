#include "headers.h"
#include "format.h"

struct str_fmt {
	char *str;
	__u32 format;
};

#define STR_FMT(s, fmt) \
	{.str = s, .format = fmt }

static struct str_fmt str_fmt[] = {
	STR_FMT("yuv444", DISP_FMT_YCbCr444),
	STR_FMT("yuv422", DISP_FMT_YCbCr422),
	STR_FMT("yuv420", DISP_FMT_YCbCr420),
	STR_FMT("itu656p", DISP_FMT_ITU656P),
	STR_FMT("itu656i", DISP_FMT_ITU656I),
};

__u32 str_to_fmt(char *str) {
	int i = 0;

	for (i = 0; i < sizeof(str_fmt) / sizeof(struct str_fmt); i++)
		if (strcmp(str_fmt[i].str, str) == 0)
			return str_fmt[i].format;

	return 0;
}
