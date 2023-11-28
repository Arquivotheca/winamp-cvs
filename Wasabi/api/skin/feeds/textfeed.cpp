#include <precomp.h>
#include "textfeed.h"
#include <bfc/pair.h>

int TextFeed::registerFeed(const wchar_t *feedid, const wchar_t *initial_text, const wchar_t *description)
{
	if (feeds.getItem(StringW(feedid))) return FALSE;
	Pair<StringW, StringW> pair(initial_text, description);
	feeds.addItem(StringW(feedid), pair);

	dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)feedid, (void*)initial_text, wcslen(initial_text) + 1);
	return TRUE;
}

int TextFeed::sendFeed(const wchar_t *feedid, const wchar_t *text)
{
	Pair <StringW, StringW> ft(L"", L"");
	if (!feeds.getItem(StringW(feedid), &ft))
	{
		//CUT    ASSERTALWAYS("hey, you're trying to send a feed you didn't register. stop it.");
		DebugString("TextFeed::sendFeed(), feedid '%s' not registered", feedid);
		return FALSE;
	}
	StringW id(feedid);
	feeds.getItem(id, &ft);
	ft.a = StringW(text);
	feeds.setItem(StringW(feedid), ft);
	dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)feedid, (void*)text, wcslen(text) + 1);
	return TRUE;
}

const wchar_t *TextFeed::getFeedText(const wchar_t *name)
{
	const Pair<StringW, StringW> *ft = feeds.getItemRef(StringW(name));
	if (ft == NULL) return NULL;
	return ft->a.getValue();
}

const wchar_t *TextFeed::getFeedDescription(const wchar_t *name)
{
	const Pair<StringW, StringW> *ft = feeds.getItemRef(StringW(name));
	if (ft == NULL) return NULL;
	return ft->b.getValue();
}

int TextFeed::hasFeed(const wchar_t *name)
{
	return feeds.getItem(name);
}

void TextFeed::dependent_onRegViewer(api_dependentviewer *viewer, int add)
{
	if (add)
	{
		for (int i = 0; i < feeds.getNumItems(); i++)
		{
			StringW a = feeds.enumIndexByPos(i, StringW(L""));
			Pair<StringW, StringW> sp(L"", L"");
			StringW b = feeds.enumItemByPos(i, sp).a;
			dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)a.getValue(), (void*)b.getValue(), b.len() + 1, viewer); //send to this viewer only
		}
	}

	if (add) onRegClient();
	else onDeregClient();
}

void *TextFeed::dependent_getInterface(const GUID *classguid)
{
	HANDLEGETINTERFACE(svc_textFeed);
	return NULL;
}
