/*
* The contents of this file are subject to the Mozilla Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS
* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
* implied. See the License for the specific language governing
* rights and limitations under the License.
*
* The Original Code is MPEG4IP.
*
* The Initial Developer of the Original Code is Cisco Systems Inc.
* Portions created by Cisco Systems Inc. are
* Copyright (C) Cisco Systems Inc. 2001.  All Rights Reserved.
*
* Contributor(s):
*      M. Bakker     mbakker at nero.com
*
* Apple iTunes META data
*/

#include <mp4common.h>

MP4Meta1Atom::MP4Meta1Atom(MP4Atom *parent, const char *name) : MP4Atom(name)
{
	AddVersionAndFlags(); /* 0, 1 */

	AddProperty(new MP4BytesProperty("metadata")); /* 2 */
}

void MP4Meta1Atom::Read() 
{
	// calculate size of the metadata from the atom size
	((MP4BytesProperty*)m_pProperties[2])->SetValueSize(m_size - 4);

	MP4Atom::Read();
}

MP4DataAtom::MP4DataAtom(MP4Atom *parent, const char *type) : MP4Atom(parent, type)
{
	AddVersionAndFlags(); /* 0, 1 */
	AddReserved("reserved2", 4); /* 2 */

	AddProperty(new MP4BytesProperty("metadata")); /* 3 */
}

MP4DataAtom::MP4DataAtom() : MP4Atom("data")
{
	AddVersionAndFlags(); /* 0, 1 */
	AddReserved("reserved2", 4); /* 2 */

	AddProperty(new MP4BytesProperty("metadata")); /* 3 */
}

void MP4DataAtom::Read() 
{
	// calculate size of the metadata from the atom size
	((MP4BytesProperty*)m_pProperties[3])->SetValueSize(m_size - 8);
	MP4Atom::Read();
}

MP4MetaItunes::MP4MetaItunes(MP4Atom *parent, const char *name) : MP4Atom(parent, name)
{
	ExpectChildAtom<MP4DataAtom>("data", Required, OnlyOne);
}

MP4Meta2Atom::MP4Meta2Atom(MP4Atom *parent, const char *name) : MP4Atom(parent, name)
{
	// add data property
	AddReserved("reserved2", 4); /* 0 */

	AddProperty(new MP4BytesProperty("metadata")); /* 1 */
}

void MP4Meta2Atom::Read()
{
	((MP4BytesProperty*)m_pProperties[1])->SetValueSize(m_size - 4);
	MP4Atom::Read();
}


MP4Meta3Atom::MP4Meta3Atom(MP4Atom *parent, const char *name) : MP4Atom(name)
{
	// add data property
	AddReserved("reserved2", 4); /* 0 */
	AddProperty(new MP4Integer16Property("language"));

	MP4StringProperty *strProp = new MP4StringProperty("metadata");
	strProp->SetUnicode(true);
	AddProperty(strProp); /* 3 */
}

void MP4Meta3Atom::Read ()
{
	MP4Atom::Read();
}

MP4Meta4Atom::MP4Meta4Atom (const char *name) : MP4Atom(name)
{	
	// add data property
	AddReserved("reserved2", 4); /* 0 */

	AddProperty(new MP4Integer16Property("metadata")); /* 1 */
}