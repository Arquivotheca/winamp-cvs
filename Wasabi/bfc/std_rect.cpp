#include "std_rect.h"
#include <bfc/platform/minmax.h>
#include <bfc/std_mem.h>

bool Std::rectIntersect(const RECT *i1, const RECT *i2, RECT *intersection)
{
	RECT out;
	out.left   = MAX(i1->left, i2->left);
	out.right  = MIN(i1->right, i2->right);
	out.top    = MAX(i1->top, i2->top);
	out.bottom = MIN(i1->bottom, i2->bottom);

	if (intersection != NULL) *intersection = out;
	return (out.left < out.right && out.top < out.bottom);
}

bool Std::pointInRect(RECT *r, POINT &p)
{
	if (p.x < r->left ||
	    p.x >= r->right ||
	    p.y < r->top ||
	    p.y >= r->bottom) return 0;
	return 1;
}

void Std::setRect(RECT *r, int left, int top, int right, int bottom)
{
	r->left = left;
	r->top = top;
	r->right = right;
	r->bottom = bottom;

}

RECT Std::makeRect(int left, int top, int right, int bottom)
{
	RECT r;
	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;
	return r;

}

POINT Std::makePoint(int x, int y)
{
	POINT p = { x, y };
	return p;
}

void Std::offsetRect(RECT *r, int x, int y)
{
	r->left += x;
	r->right += x;
	r->top += y;
	r->bottom += y;
}

bool Std::rectEqual(const RECT &a, const RECT &b)
{
	return !MEMCMP(&a, &b, sizeof(RECT));
}

bool Std::rectEqual(const RECT *a, const RECT *b)
{
	return !MEMCMP(a, b, sizeof(RECT));
}

void Std::scaleRect(RECT *r, double scale)
{
  r->left =(long)(r->left * scale + 0.5);
  r->right = (long)(r->right * scale + 0.5);
  r->bottom = (long)(r->bottom * scale + 0.5);
  r->top = (long)(r->top * scale + 0.5);
}

