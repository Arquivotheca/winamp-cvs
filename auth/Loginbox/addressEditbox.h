#ifndef NULLSOFT_AUTH_LOGINBOX_ADDRESS_EDITBOX_HEADER
#define NULLSOFT_AUTH_LOGINBOX_ADDRESS_EDITBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define REPLACE_MARKER_BEGIN	L'<'
#define REPLACE_MARKER_END		L'>'

BOOL AddressEditbox_AttachWindow(HWND hEditbox);

#endif // NULLSOFT_AUTH_LOGINBOX_ADDRESS_EDITBOX_HEADER