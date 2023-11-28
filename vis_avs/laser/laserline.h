#ifdef LASER
#include "linelist.h"

void LineDrawList(C_LineListBase *list, int *fb, int w, int h);

void laser_connect(void);
void laser_disconnect(void);
void laser_sendframe(void *data, int datalen);

static void __inline laser_drawpoint(float x, float y, int current_color)
{
  LineType line;
  line.color=current_color;
  line.mode=1;
  line.x2=0;
  line.x1=x;
  line.y2=0;
  line.y1=y;
  g_laser_linelist->AddLine(&line);
}

#else
#ifndef laser_drawpoint
#define laser_drawpoint(x,y,c)
#endif
#endif