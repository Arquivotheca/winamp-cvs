#pragma once

extern int logMode;

bool Config_GetLogging();
bool Config_GetSlimLogging();
bool Config_GetLogBinary();

extern int MetadataKey_CloudID;
extern int MetadataKey_CloudIDHash;
extern int MetadataKey_CloudMetaHash;
extern int MetadataKey_CloudMediaHash;
extern int MetadataKey_CloudAlbumHash;
extern int MetadataKey_CloudDevice;
extern int MetadataKey_CloudArtHashAlbum;