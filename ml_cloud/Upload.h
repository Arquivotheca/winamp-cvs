#pragma once

class UploadCallbacks
{
public:
	virtual bool OnCount(unsigned int filecount)=0;
	virtual bool OnFile(unsigned int count, const wchar_t *filename)=0;
	virtual bool OnProgress(unsigned int percent)=0;
	virtual void OnDone()=0;
};
