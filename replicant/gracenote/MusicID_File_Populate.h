#pragma once
#include "nx/nxuri.h"
#include "gnsdk.h"

int MusicID_File_Populate(gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo, nx_uri_t filename);
int MusicID_File_Fingerprint(nx_uri_t filename, gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo);
int MusicID_File_Metadata(nx_uri_t filename, gnsdk_musicidfile_fileinfo_handle_t gn_fileinfo);