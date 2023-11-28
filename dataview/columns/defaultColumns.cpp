#include "main.h"
#include "./defaultColumns.h"

#include "./ifc_viewcolumninfo.h"

#include "./albumArtistColumn.h"
#include "./albumArtistIndexColumn.h"
#include "./albumColumn.h"
#include "./artistColumn.h"
#include "./artistIndexColumn.h"
#include "./bitrateColumn.h"
#include "./composerColumn.h"
#include "./discNumberColumn.h"
#include "./genreColumn.h"
#include "./lastPlayedColumn.h"
#include "./lastUpdatedColumn.h"
#include "./lengthColumn.h"
#include "./nextFilterCountColumn.h"
#include "./playCountColumn.h"
#include "./publisherColumn.h"
#include "./ratingColumn.h"
#include "./sizeColumn.h"
#include "./titleColumn.h"
#include "./trackCountColumn.h"
#include "./trackNumberColumn.h"
#include "./typeColumn.h"
#include "./yearColumn.h"

typedef HRESULT (*ColumnInfoCreator)(ifc_viewcolumninfo ** /*instance*/);


static const ColumnInfoCreator defaultColumns[] = 
{
	AlbumArtistColumn::CreateInfo,
	AlbumArtistIndexColumn::CreateInfo,
	AlbumColumn::CreateInfo,
	ArtistColumn::CreateInfo,
	ArtistIndexColumn::CreateInfo,
	BitrateColumn::CreateInfo,
	ComposerColumn::CreateInfo,
	DiscNumberColumn::CreateInfo,
	GenreColumn::CreateInfo,
	LastPlayedColumn::CreateInfo,
	LastUpdatedColumn::CreateInfo,
	LengthColumn::CreateInfo,
	NextFilterCountColumn::CreateInfo,
	PlayCountColumn::CreateInfo,
	PublisherColumn::CreateInfo,
	RatingColumn::CreateInfo,
	SizeColumn::CreateInfo,
	TitleColumn::CreateInfo,
	TrackCountColumn::CreateInfo,
	TrackNumberColumn::CreateInfo,
	TypeColumn::CreateInfo,
	YearColumn::CreateInfo,
};


HRESULT 
RegisterDefaultColumns(ifc_viewcolumnmanager *manager)
{
	HRESULT hr;
	size_t index, count;
	ifc_viewcolumninfo *columns[ARRAYSIZE(defaultColumns)];

	if (NULL == manager)
		return E_INVALIDARG;

	count = 0;
	for (index = 0; index < ARRAYSIZE(defaultColumns); index++)
	{
		if (SUCCEEDED(defaultColumns[index](&columns[count])))
			count++;
	}

	index = manager->Register(columns, count);
	if (index == count)
		hr = S_OK;
	else
		hr = E_FAIL;

	while(count--)
		columns[count]->Release();
	
	return hr;
}
