#ifndef NULLSOFT_ML_DOWNLOADS_ERRORS_H
#define NULLSOFT_ML_DOWNLOADS_ERRORS_H

enum
{
	DOWNLOAD_SUCCESS = 0,
	DOWNLOAD_404,
	DOWNLOAD_TIMEOUT,
	DOWNLOAD_NOHTTP,
	DOWNLOAD_NOPARSER,
	DOWNLOAD_CONNECTIONRESET,
	DOWNLOAD_ERROR_PARSING_XML,
};

#endif