#include <stdio.h>
#include <bfc/platform/types.h>
#if (_MSC_VER < 1500)
int _fseeki64(FILE *stream, int64_t offset, int origin)
{
	switch(origin)
	{
		case SEEK_END:
						fseek(stream, 0, SEEK_END);
						// fall through
	case SEEK_CUR:
		{
			fpos_t pos;
			fgetpos(stream, &pos);
			pos += offset;
			return fsetpos(stream, &pos);
		}
	case SEEK_SET:
		{
			return fsetpos(stream, &offset);
		}
	}
	return 1;
}
#endif