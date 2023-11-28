#include "./imageLoader.h"
#include "./wasabiApi.h"


void* ImageLoader_LoadPngData(HINSTANCE hInstance, LPCTSTR pszResourceName, INT *pWidthOut, INT *pHeightOut)
{
	if (NULL == WASABI_API_MEMMNGR)
	{
		WASABI_API_MEMMNGR = QueryWasabiInterface(api_memmgr, memMgrApiServiceGuid);
		if (NULL == WASABI_API_MEMMNGR)
			return NULL;
	}

	if (NULL == WASABI_API_PNGLOADER)
	{
		WASABI_API_PNGLOADER = QueryWasabiInterface(svc_imageLoader, pngLoaderGUID);
		if (NULL == WASABI_API_PNGLOADER)
			return NULL;
	}

	void *resourceData;
	INT resourceSize;
	HANDLE resourceHandle;

	HRSRC hRes;
	hRes = FindResource(hInstance, pszResourceName, /*RT_RCDATA*/MAKEINTRESOURCEW(10));
	if (NULL != hRes)
	{
		resourceHandle = LoadResource(hInstance, hRes);
		resourceSize = SizeofResource(hInstance, hRes);
		resourceData = LockResource(resourceHandle);
	}
	else 
	{
		resourceHandle = NULL;
		resourceSize = 0;
		resourceData = NULL;
	}

	if (NULL != resourceData)
		resourceData = WASABI_API_PNGLOADER->loadImageData(resourceData, resourceSize, pWidthOut, pHeightOut);
	
	if (NULL != resourceHandle)
		FreeResource(resourceHandle);
	
	return resourceData;
}

BOOL ImageLoader_FreeData(void *data)
{
	if (NULL == data)
		return TRUE;

	if (NULL == WASABI_API_MEMMNGR)
	{
		WASABI_API_MEMMNGR = QueryWasabiInterface(api_memmgr, memMgrApiServiceGuid);
		if (NULL == WASABI_API_MEMMNGR)
			return FALSE;
	}

	WASABI_API_MEMMNGR->sysFree(data);
	return TRUE;
}

HBITMAP ImageLoader_LoadPng(HINSTANCE hInstance, LPCTSTR pszResourceName, INT *pWidthOut, INT *pHeightOut)
{
	BITMAPINFOHEADER header;
	void *pixels;
	HBITMAP bitmap =  ImageLoader_LoadPngEx(hInstance, pszResourceName, &header, &pixels);

	if (NULL != pWidthOut) 
		*pWidthOut = (NULL != bitmap) ? header.biWidth : 0;
	
	if (NULL != pHeightOut) 
		*pHeightOut = (NULL != bitmap) ? abs(header.biHeight) : 0;
	
	return bitmap;
}

HBITMAP ImageLoader_LoadPngEx(HINSTANCE hInstance, LPCTSTR pszResourceName, BITMAPINFOHEADER *pHeader, void **ppData)
{
	INT imageCX, imageCY;
	
	void *resourceData = ImageLoader_LoadPngData(hInstance, pszResourceName, &imageCX, &imageCY);
	if (NULL == resourceData)
		return NULL;
	
	ZeroMemory(pHeader, sizeof(BITMAPINFOHEADER));
	pHeader->biSize = sizeof(BITMAPINFOHEADER);
	pHeader->biCompression = BI_RGB;
	pHeader->biBitCount = 32;
	pHeader->biPlanes = 1;
	pHeader->biWidth = imageCX;
	pHeader->biHeight = -imageCY;

	HBITMAP hbmp = CreateDIBSection(NULL, (LPBITMAPINFO)pHeader, DIB_RGB_COLORS, ppData, NULL, 0);
	if (NULL != hbmp)
		CopyMemory(*ppData, resourceData, pHeader->biWidth * abs(pHeader->biHeight) * sizeof(DWORD));
	else
		*ppData = NULL;
	
	ImageLoader_FreeData(resourceData);
	return hbmp;
}

BOOL BlendOnColorEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb)
{
	LONG pitch;
	WORD r, g, b, destR, destG, destB;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
		
	if (bpp != 32) 
		return TRUE;

	if (cy < 0) cy -= cy;

	r = GetRValue(rgb); g = GetGValue(rgb); b = GetBValue(rgb);
	
	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;
	
	if (premult)
	{
		for (; cy-- != 0; line += pitch)
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 4) 
			{
				if (0x00 == cursor[3]) 
				{
					cursor[0] = (BYTE)b;
					cursor[1] = (BYTE)g;
					cursor[2] = (BYTE)r;
					cursor[3] = 0xFF;
				}
				else if (cursor[3] != 0xFF)
				{
					WORD a = 255 - cursor[3];
					destB = (cursor[0] * 255 + a * b + 127) / 255;
					destG = (cursor[1] * 255 + a * g + 127) / 255;
					destR = (cursor[2] * 255 + a * r + 127) / 255;
					
					cursor[0] = (destB > 0xFF) ? 0xFF : destB;
					cursor[1] = (destG > 0xFF) ? 0xFF : destG;
					cursor[2] = (destR > 0xFF) ? 0xFF : destR;
					cursor[3] = 0xFF;
				}
			}
		}
	}
	else
	{
		for (; cy-- != 0; line += pitch)
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 4) 
			{
				if (0x00 == cursor[3]) 
				{
					cursor[0] = (BYTE)b;
					cursor[1] = (BYTE)g;
					cursor[2] = (BYTE)r;
					cursor[3] = 0xFF;
				}
				else if (cursor[3] != 0xFF)
				{
					WORD a = (((255 - cursor[3])*255 + 127)/255);
					cursor[0] = (cursor[0]*cursor[3] + a*b + 127)/255;
					cursor[1] = (cursor[1]*cursor[3] + a*g + 127)/255;
					cursor[2] = (cursor[2]*cursor[3] + a*r + 127)/255;
					cursor[3] = 0xFF;
				}
			}
		}
		
	}
	return TRUE;
}

BOOL BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return BlendOnColorEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, premult, rgb);
}


BOOL ColorOverImageEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb)
{
	LONG pitch;
	UINT a, r, g, b, t, ma, mr, mg, mb;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
	
	if (step < 3) 
		return TRUE;

	if (cy < 0) cy -= cy;

	a = (LOBYTE((rgb)>>24)); r = GetRValue(rgb); g = GetGValue(rgb); b = GetBValue(rgb);
	ma = 255 - a; mr = r * 255; mg = g * 255; mb = b * 255;

	if (0 == a)
		return TRUE;

	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;
	
	if (0xFF == a) 
	{
		for (; cy-- != 0; line += pitch)
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{				
				cursor[0] = (BYTE)b;
				cursor[1] = (BYTE)g;
				cursor[2] = (BYTE)r;
			//	cursor[3] = 0xFF;
			}
		}
		return TRUE;
	}
	

	if (premult)
	{
		for (; cy-- != 0; line += pitch)
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				t = (mb + ma * cursor[0] + 127) / 255;
				cursor[0] = (t > 0xFF) ? 0xFF : t;
				t = (mg + ma * cursor[1] + 127) / 255;
				cursor[1] = (t > 0xFF) ? 0xFF : t;
				t = (mr+ ma * cursor[2] + 127) / 255;
				cursor[2] = (t > 0xFF) ? 0xFF : t;
			}
		}
	}
	else
	{
		WORD k = (((255 - a)*255 + 127)/255);
		for (; cy-- != 0; line += pitch)
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				cursor[0] = (b*a + k*cursor[0] + 127)/255;
				cursor[1] = (g*a + k*cursor[1] + 127)/255;
				cursor[2] = (r*a + k*cursor[2] + 127)/255;
	//			cursor[3] = (a*a + k*cursor[3] + 127)/255;
			}
		}
		
	}
	return TRUE;
}

BOOL ColorOverImage(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return ColorOverImageEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, premult, rgb);
}

BOOL ColorizeImageEx(BYTE *pPixels, LONG cx, LONG cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha)
{
	LONG pitch, x;
	INT step;
	BYTE rFg, gFg, bFg;
	LPBYTE cursor, line;

	if (bpp < 24) return FALSE;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;
	
	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	
	INT  bK = (bFg - GetBValue(rgbBk));
	INT gK = (gFg - GetGValue(rgbBk));
	INT rK = (rFg - GetRValue(rgbBk));

	if (24 == bpp)
	{
		for (line = pPixels; cy-- != 0; line += pitch )
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 3) 
			{
				cursor[0] = bFg - (bK*(255 - cursor[0])>>8);
				cursor[1] = gFg - (gK*(255 - cursor[1])>>8);
				cursor[2] = rFg - (rK*(255 - cursor[2])>>8);
			}
		}
	}
	else if (32 == bpp)
	{
		if (removeAlpha)
		{
			BYTE rBk, gBk, bBk;
			rBk = GetRValue(rgbBk); gBk = GetGValue(rgbBk); bBk = GetBValue(rgbBk);
			for (line = pPixels; cy-- != 0; line += pitch )
			{	
				for (x = cx, cursor = line; x-- != 0; cursor += 4) 
				{
					if (0x00 == cursor[3]) 
					{
						cursor[0] = bBk;
						cursor[1] = gBk;
						cursor[2] = rBk;
						cursor[3] = 0xFF;
					}
					else if (0xFF == cursor[3])
					{
						cursor[0] = bFg - (bK*(255 - cursor[0])>>8);
						cursor[1] = gFg - (gK*(255 - cursor[1])>>8);
						cursor[2] = rFg - (rK*(255 - cursor[2])>>8);
					}
					else
					{
						cursor[0] = ((bFg - (bK*(255 - cursor[0])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*bBk + 127)/255;
						cursor[1] = ((gFg - (gK*(255 - cursor[1])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*gBk + 127)/255;
						cursor[2] = ((rFg - (rK*(255 - cursor[2])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*rBk + 127)/255;
						cursor[3] = 0xFF;
					}
				}
			}
		}
		else
		{
			for (line = pPixels; cy-- != 0; line += pitch )
			{	
				for (x = cx, cursor = line; x-- != 0; cursor += 4) 
				{
					cursor[0] = bFg - (bK*(255 - cursor[0])>>8);
					cursor[1] = gFg - (gK*(255 - cursor[1])>>8);
					cursor[2] = rFg - (rK*(255 - cursor[2])>>8);
				}
			}
		}
	}
	return TRUE;
}

BOOL ColorizeImage(BYTE *pPixels, LONG cx, LONG cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg)
{
	return 	ColorizeImageEx(pPixels, cx, cy, bpp, rgbBk, rgbFg, TRUE);

}


BOOL PremultiplyImage(BYTE *pPixels, LONG cx, LONG cy)
{
	LONG pitch, x;
	pitch = cx* 4;
	LPBYTE cursor, line;
		
	for (line = pPixels; cy-- != 0; line += pitch )
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += 4) 
		{
			if (0x00 == cursor[3]) 
			{
				cursor[0] = 0x00;
				cursor[1] = 0x00;
				cursor[2] = 0x00;
			}
			else if (0xFF != cursor[3])
			{
				cursor[0] = (cursor[0] * cursor[3]) >> 8;
				cursor[1] = (cursor[1] * cursor[3]) >> 8;
				cursor[2] = (cursor[2] * cursor[3]) >> 8;
			}
		}
	}

	return TRUE;
}

static BYTE ReadPixel(BYTE *data, INT w, INT h, INT d, INT x, INT y, INT z)
{
	if (x < 0) x = 0;
	else if (x >= w)  x = w - 1;
	if (y < 0) y = 0;
	else if (y >= h) y = h -1;
	
	
	int line = w * d;
	if (0 != line%4) 
		line += line%4;

    return *(data  + (line * y) + x*d + z);
}

static void WritePixel(BYTE *data, INT w, INT h, INT d, INT x, INT y, INT z, INT val)
{
	if ((x < 0 || x >= w) || (y < 0 || y >= h)) 
		return;
	
	int line = w * d;
	if (0 != line%4) 
		line += line%4;

	if (val < 0) val = 0;
	if (val > 255) val = 255;
    
	*(data  + (line * y) + x*d + z) = val;
}

static __forceinline BYTE GetByteVal(BYTE *cursor, BYTE *min, BYTE *max)
{
	if (cursor < min) 
		return *min;
	else if (cursor > max)
		return *max;
	else 
		return *cursor;
}

BOOL BoxBlur(BYTE *pixels, LONG cx, LONG cy, WORD bpp, INT blurRadius, BOOL ignoreAlpha, HANDLE hKill)
{
	return BoxBlurEx(pixels, 0, 0, cx - 1, cy - 1, cx, cy, bpp, blurRadius, ignoreAlpha, hKill);
}

BOOL BoxBlurEx(BYTE *pixels, LONG left, LONG top, LONG right , LONG bottom, LONG cx, LONG cy, WORD bpp, INT blurRadius, BOOL ignoreAlpha, HANDLE hKill)
{
	int Z = bpp>>3;
	int weight = blurRadius*2 + 1;
	
	int line = cx * Z;
	if (0 != line%4)
	{
		line += (4 -line%4);
	}

	int z;
	
	BYTE *tmp = (BYTE*)malloc(line * cy);
		
	BYTE *cursor;
	BYTE *max;
	BYTE *px;

	LONG oly = blurRadius * line;
	LONG ory = (blurRadius + 1) * line;

	LONG olx = blurRadius * Z;
	LONG orx = (blurRadius + 1) * Z;

	BYTE *limX, *limY;
	int sum;
	
	BYTE limZ = Z;
	
	if (4 == Z && ignoreAlpha)
		limZ--;

	for (z = 0; z < limZ; z++)
	{			
		cursor = pixels + max(left - blurRadius, 0)*Z + top * line + z;
		max = cursor + (bottom - top) * line;
		limX = cursor + min(right + blurRadius + 1, cx)*Z;

		if (WAIT_OBJECT_0 == WaitForSingleObject(hKill, 0))
			break; 

		for (; cursor < limX; cursor += Z, max += Z)
		{			
			sum = 0;
			limY = cursor + oly;
			for (px = cursor - ory; px < limY; px += line)
			{
				sum += GetByteVal(px, cursor, max);
			}
			
			for (px = cursor; px <= max; px += line)
			{
				sum += GetByteVal(px + oly, cursor, max) - GetByteVal(px - ory, cursor, max);
				tmp[px - pixels] = sum/weight;
			}
		
		}


		if (WAIT_OBJECT_0 == WaitForSingleObject(hKill, 0))
			break; 

		cursor = tmp + left * Z + max(top - blurRadius, 0) * line + z;
		max = cursor + (right - left) * Z;
		limY = cursor + min(bottom + blurRadius + 1, cy)*line;

		for (;cursor < limY; cursor += line, max += line)
		{
			sum = 0;
			limX = cursor + olx;
			for (px = cursor - orx; px < limX; px += Z)
			{
				sum += GetByteVal(px, cursor, max);
			}
			for (px = cursor; px <= max; px += Z)
			{
				sum += GetByteVal(px + olx, cursor, max) - GetByteVal(px - orx, cursor, max);
				pixels[px - tmp] = sum/weight;
			}
		
		}
	}

	free(tmp);
	return TRUE;
}