#ifndef _ANIMATE_H
#define _ANIMATE_H

#include <bfc/std.h>

class AnimatedRects {
public:
  static void draw(const RECT *src, const RECT *dst, int nsteps=5);
};

#endif
