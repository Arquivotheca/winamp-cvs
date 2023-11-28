#pragma once
#include "webserver.h"


class BufferPageGenerator : public ifc_pagegenerator
{
  public:
		BufferPageGenerator();
    ~BufferPageGenerator();
		int Initialize(const void *buffer, size_t length);
    size_t WASABICALL PageGenerator_GetData(void *buf, size_t size); // return 0 when done
		int WASABICALL PageGenerator_IsNonBlocking() { return 0; }

  private:
    uint8_t *buffer;
		size_t length;
};