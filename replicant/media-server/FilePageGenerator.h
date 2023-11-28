#pragma once
#include "webserver.h"
#include "nx/nxfile.h"

/* TODO: hook up to file lock API */

class FilePageGenerator : public ifc_pagegenerator
{
  public:
		FilePageGenerator();
    ~FilePageGenerator();
		int Initialize(nx_uri_t filename, jnl_http_request_t serv);
    size_t WASABICALL PageGenerator_GetData(void *buf, size_t size); // return 0 when done
		int WASABICALL PageGenerator_IsNonBlocking() { return 0; }

  private:
    nx_file_t fp;
};