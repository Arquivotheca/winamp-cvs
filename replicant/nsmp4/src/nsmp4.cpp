#include "nsmp4.h"
#include "mp4file.h"
#include "foundation/error.h"


int NSMP4_Metadata_iTunes_Enumerate(MP4FileHandle mp4_file, uint32_t index, nsmp4_metadata_itunes_atom_t *atom)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4 || !atom)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_Enumerate(index, (MP4Atom **)atom);	
}

int NSMP4_Metadata_iTunes_FindKey(MP4FileHandle mp4_file, const char *key, nsmp4_metadata_itunes_atom_t *atom)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4 || !atom)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_FindKey(key, (MP4Atom **)atom);	
}

int NSMP4_Metadata_iTunes_EnumerateKey(MP4FileHandle mp4_file, const char *key, unsigned int index, nsmp4_metadata_itunes_atom_t *atom)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4 || !atom)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_EnumerateKey(key, index, (MP4Atom **)atom);	
}


int NSMP4_Metadata_iTunes_NewKey(MP4FileHandle mp4_file, const char *key, nsmp4_metadata_itunes_atom_t *atom, uint32_t flags)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4 || !atom)
		return NErr_NullPointer;

	try 
	{
		return mp4->Metadata_iTunes_NewKey(key, (MP4Atom **)atom, flags);
	}
	catch (MP4Error* e) 
	{
		return NErr_Error;
	}
}

int NSMP4_Metadata_iTunes_FindFreeform(MP4FileHandle mp4_file, const char *name, const char *mean, nsmp4_metadata_itunes_atom_t *atom)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4 || !atom)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_FindFreeform(name, mean, (MP4Atom **)atom);	
}

int NSMP4_Metadata_iTunes_NewFreeform(MP4FileHandle mp4_file, const char *name, const char *mean, nsmp4_metadata_itunes_atom_t *atom, uint32_t flags)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4 || !atom)
		return NErr_NullPointer;

	try 
	{
		return mp4->Metadata_iTunes_NewFreeform(name, mean, (MP4Atom **)atom, flags);
	}
	catch (MP4Error* e) 
	{
		return NErr_Error;
	}

}
int NSMP4_Metadata_iTunes_GetInformation(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t metadata_atom, char atom[5], uint32_t *flags)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_GetInformation((MP4Atom *)metadata_atom, atom, flags);	
}

int NSMP4_Metadata_iTunes_GetString(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, nx_string_t *value)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_GetString((MP4Atom *)atom, value);	
}

int NSMP4_Metadata_iTunes_GetBinary(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, const uint8_t **value, size_t *value_length)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_GetBinary((MP4Atom *)atom, value, value_length);
}

int NSMP4_Metadata_iTunes_GetSigned(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, int64_t *value)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_GetSigned((MP4Atom *)atom, value);
}

int NSMP4_Metadata_iTunes_GetUnsigned(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, uint64_t *value)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_GetUnsigned((MP4Atom *)atom, value);
}

int NSMP4_Metadata_iTunes_GetFreeform(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, nx_string_t *name, nx_string_t *mean)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	return mp4->Metadata_iTunes_GetFreeform((MP4Atom *)atom, name, mean);
}

int NSMP4_Metadata_iTunes_SetString(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, nx_string_t value)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	try 
	{
		return mp4->Metadata_iTunes_SetString((MP4Atom *)atom, value);
	}
	catch (MP4Error* e) 
	{
		return NErr_Error;
	}
}

int NSMP4_Metadata_iTunes_SetUnsigned(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, uint64_t value, size_t byte_count)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	try 
	{
		return mp4->Metadata_iTunes_SetUnsigned((MP4Atom *)atom, value, byte_count);
	}
	catch (MP4Error* e) 
	{
		return NErr_Error;
	}
}

int NSMP4_Metadata_iTunes_SetBinary(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom, const void *data, size_t length)
	{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	try 
	{
		return mp4->Metadata_iTunes_SetBinary((MP4Atom *)atom, data, length);
	}
	catch (MP4Error* e) 
	{
		return NErr_Error;
	}
}


int NSMP4_Metadata_iTunes_DeleteAtom(MP4FileHandle mp4_file, nsmp4_metadata_itunes_atom_t atom)
{
	MP4File *mp4 = (MP4File *)mp4_file;
	if (!mp4)
		return NErr_NullPointer;

	try 
	{
		return mp4->Metadata_iTunes_DeleteAtom((MP4Atom *)atom);
	}
	catch (MP4Error* e) 
	{
		return NErr_Error;
	}
}