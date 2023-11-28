#include "./main.h"
#include "./baseListView.h"

#include "./listInsertMark.h"
#include "./listDragScroll.h"
#include "./listDragData.h"

#include "./skinWindow.h"
#include "./guiObjects.h"

#include "./fileInfoInterface.h"
#include "./fileEnumInterface.h"
#include "./cfpInterface.h"

#include "./dataObject.h"
#include "./dropSource.h"
#include "./mediaLibraryDrop.h"

#include <strsafe.h>
#include <shlobj.h>

HBITMAP CreateDragImage(SHDRAGIMAGE *pDragImage, INT iCount);
BOOL CreateDragImageList(INT iCount);

static DropSource *pActiveDropSource = NULL;
static UINT dbcfViewItems = 0; 

static HRESULT PathToBuffer(LPTSTR pszBuffer, size_t cchBufferMax, IFileInfo **ppFileInfo, INT nCount)
{
	HRESULT hr;
	LPCTSTR pszPath;

	*pszBuffer = TEXT('\0');
	
	for (INT index = 0; index < nCount; index++)
	{
		hr = ppFileInfo[index]->GetPath(&pszPath);
		if (FAILED(hr)) 
			return hr;
		
		hr = StringCchCopyEx(pszBuffer, cchBufferMax, pszPath, &pszBuffer, &cchBufferMax, STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
		if (FAILED(hr)) 
			return hr;
			
		if (cchBufferMax > 0)
		{
			*pszBuffer = TEXT('\0');
			pszBuffer++;
			cchBufferMax--;
		}
	}
	
	if (cchBufferMax > 0)
	{
		*pszBuffer = TEXT('\0');
		pszBuffer++;
		cchBufferMax--;
	}

	return S_OK;

}

static HANDLE CreateHDrop(HANDLE hViewItems, POINT *pDropPoint, BOOL bNonClient, size_t **ppHdropItems, size_t *pcHdropItems)
{	
	DROPVIEWITEMS *pHeader = (DROPVIEWITEMS*)GlobalLock(hViewItems);
	if (NULL == pHeader)
		return NULL;

	HGLOBAL hg = NULL;
	if (pHeader->nCount > 0)
	{		
		HWND hParent = GetParent(pHeader->hwndList);
		Document *pDoc = DropboxWindow_GetActiveDocument(hParent);
		if (NULL != pDoc)
		{
			pDoc->AddRef();
			IFileInfo** ppFileInfo = (IFileInfo**)malloc(sizeof(IFileInfo*) * pHeader->nCount);
			size_t *filteredItems = (size_t*)malloc(sizeof(size_t) * pHeader->nCount);

			if (NULL != ppFileInfo)
			{				
				size_t cchLen = 0;
				LPCTSTR pszPath;
				INT count = 0;
				IFileInfo *pFile;
				INT *cursor = (INT*)(((BYTE*)pHeader) + pHeader->pItems);
				
				for (INT *end = cursor + pHeader->nCount; cursor < end; cursor++)
				{
					pFile = pDoc->GetItemDirect(*cursor);
					pFile->AddRef();

					if (S_OK == pFile->CanCopy() && SUCCEEDED(pFile->GetPath(&pszPath)))
					{
						ppFileInfo[count] = pFile;
						filteredItems[count] = *cursor;
						count++;
						cchLen += (lstrlen(pszPath) + 1);
					}
					else
						pFile->Release();
				}

				if (0 != count && 0 != cchLen)
				{
					cchLen++;
					hg = GlobalAlloc(GMEM_FIXED, sizeof(DROPFILES) + (cchLen) * sizeof(TCHAR));
					if (NULL != hg)
					{
						DROPFILES *pdf = (DROPFILES*)GlobalLock(hg);
						if (NULL != pdf)
						{
							pdf->fNC = bNonClient;
							pdf->fWide = sizeof(TCHAR);
							if (NULL != pDropPoint)
								pdf->pt = *pDropPoint;
							else 
							{
								pdf->pt.x = 0;
								pdf->pt.y = 0;
							}
							pdf->pFiles = sizeof(DROPFILES);
							PathToBuffer((LPTSTR)(((BYTE*)pdf) + pdf->pFiles), cchLen, ppFileInfo, count);
						}
						GlobalUnlock(hg);
					}
				}


				if (NULL != ppHdropItems && NULL != pcHdropItems)
				{
					*ppHdropItems = filteredItems;
					*pcHdropItems = count;
				}
				else
					free(filteredItems);

				while(count--) ppFileInfo[count]->Release();
				free(ppFileInfo);
			}
			pDoc->Release();
		}
	}

	GlobalUnlock(hViewItems);
	return hg;
}

static HANDLE CreateViewItems(HWND hList, INT iFirst)
{
	INT selectedCount = (INT)SendMessage(hList, LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 0) selectedCount = 0;

	HGLOBAL hg = GlobalAlloc(GMEM_FIXED, sizeof(DROPVIEWITEMS) + (selectedCount) * sizeof(INT));
	if (NULL == hg)
		return NULL;
	
	DROPVIEWITEMS *pHeader = (DROPVIEWITEMS*)GlobalLock(hg);
	if (NULL == pHeader)
	{
		GlobalFree(hg);
		return NULL;
	}

	pHeader->hwndList = hList;
	pHeader->nCount = selectedCount;
	pHeader->pItems = sizeof(DROPVIEWITEMS);
	pHeader->iTop = iFirst;

	if (selectedCount > 0)
	{
		INT *pItems = (INT*)(((BYTE*)pHeader) + pHeader->pItems);
		INT iFile = (-1 == iFirst) ? iFirst : (iFirst - 1);
		while (-1 != (iFile = (INT)SendMessage(hList, LVM_GETNEXTITEM, iFile, (LPARAM)LVNI_SELECTED)))
		{
			*pItems = iFile;
			pItems++;
		}
		if (-1 != iFirst)
		{
			iFile = -1;
			while (-1 != (iFile = (INT)SendMessage(hList, LVM_GETNEXTITEM, iFile, (LPARAM)LVNI_SELECTED)) && iFile < iFirst)
			{
				*pItems = iFile;
				pItems++;
			}

		}
	}

	if (pHeader->iTop < 0)
		pHeader->iTop = *(INT*)(((BYTE*)pHeader) + pHeader->pItems);

	GlobalUnlock(hg);
	return hg;
}


HRESULT BaseListView::MakeDataObject(INT iFirst, POINT *pDropPoint, BOOL bNonClient, IDataObject **ppDataObject, UINT useFormats, size_t **ppHdropItems, size_t *pcHdropItems)
{
	HRESULT hr = S_OK;
	FORMATETC   format;
	STGMEDIUM   storage;
	
	HANDLE hViewItems;
    
	if (0 == useFormats)
		return NULL;

	format.ptd = NULL;
    format.dwAspect = DVASPECT_CONTENT;
    format.lindex = -1;
    format.tymed = TYMED_HGLOBAL;

	storage.tymed = TYMED_HGLOBAL;
	storage.pUnkForRelease = NULL;

	*ppDataObject = NULL;

	DataObject *pdo = new DataObject();
	if (NULL == pdo)
		return E_OUTOFMEMORY;
	
	hViewItems = CreateViewItems(hwnd, iFirst);
	if (NULL == hViewItems)
		return E_OUTOFMEMORY;
	
	if (0 != (DATAOBJECT_HDROP & useFormats))
	{
		format.cfFormat = CF_HDROP;
		storage.hGlobal = CreateHDrop(hViewItems, pDropPoint, bNonClient, ppHdropItems, pcHdropItems);
		
		if (NULL != storage.hGlobal)
		{
			hr = pdo->SetData(&format, &storage, TRUE); 
			if (FAILED(hr))
				GlobalFree(storage.hGlobal);
		}
		else
			hr = S_FALSE;
	}
	
	if (0 != (DATAOBJECT_VIEWITEMS & useFormats))
	{
		if (0 == dbcfViewItems) 
			dbcfViewItems = RegisterClipboardFormat(DBCF_VIEWITEMS); 

		if (0 != dbcfViewItems)
		{
			format.cfFormat = dbcfViewItems;
			storage.hGlobal = hViewItems;
			if (NULL != storage.hGlobal)
			{
				hr = (0 !=  format.cfFormat) ? pdo->SetData(&format, &storage, TRUE) : E_UNEXPECTED; 
				if (FAILED(hr))
					GlobalFree(storage.hGlobal);
			}
			else
				hr = S_FALSE;
		}
		else
		{
			hViewItems = GlobalFree(hViewItems);
			hr = E_UNEXPECTED;
		}
	}
	else 
		hViewItems = GlobalFree(hViewItems);

	if (SUCCEEDED(hr))
		*ppDataObject = pdo;
	else
		pdo->Release();

	return hr;
}

HRESULT BaseListView::PerformDragDrop(INT iItem)
{
	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 1) return S_FALSE;
	
	HRESULT hr = S_OK;
	POINT pt;
	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);

	DropSource *pDropSource = new DropSource(hwnd);
	if (NULL != pDropSource) 
	{		
		IDataObject *pDataObject;
		HRESULT hr = MakeDataObject(iItem, &pt, FALSE, &pDataObject, DATAOBJECT_VIEWITEMS | DATAOBJECT_HDROP, NULL, NULL);
		if (SUCCEEDED(hr))		
		{		
			
			IDragSourceHelper *pDragSourceHelper = NULL;
			CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDragSourceHelper, (LPVOID*)&pDragSourceHelper);
			if(NULL != pDragSourceHelper)
			{ 
				SHDRAGIMAGE di;
				HBITMAP hbmp = CreateDragImage(&di, (INT)selectedCount);
				if (NULL != hbmp)
					hr = pDragSourceHelper->InitializeFromBitmap(&di, pDataObject);
				else
					hr = pDragSourceHelper->InitializeFromWindow(hwnd, &pt, pDataObject);
				
				if(FAILED(hr) && NULL != hbmp)
					DeleteObject(hbmp);
			}

			DWORD dwEffect;
			pActiveDropSource = pDropSource;
			hr = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY | DROPEFFECT_LINK, &dwEffect);
			pActiveDropSource = NULL;
			if(NULL != pDragSourceHelper)
				pDragSourceHelper->Release();
		
			pDataObject->Release();
		}
		pDropSource->Release();
	}
	else hr = E_OUTOFMEMORY;
	
	return hr;
}



HRESULT BaseListView::ProcessorDragEnter(IClipboardFormatProcessor *processor, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if (NULL != pDragData)
	{
		delete(pDragData);
		pDragData = NULL;
	}

	if (NULL == processor) 
	{
		*pdwEffect = DROPEFFECT_NONE;
		return E_NOTIMPL;
	}
		
	pDragData = new ListViewDragDropData(hwnd, processor);
	if (NULL == pDragData) 
	{
		*pdwEffect = DROPEFFECT_NONE;
		return E_OUTOFMEMORY; 
	}
		
	if (NULL != pActiveDropSource)
	{
		UINT sourceMode = (hwnd == pActiveDropSource->GetSourceHwnd()) ? 
					DROPSOURCE_REARRANGEMODE : DROPSOURCE_NORMALMODE;

		pActiveDropSource->SetFlags(sourceMode, DROPSOURCE_REARRANGEMODE);
	}
	*pdwEffect = pDragData->Process(&ptl, *pdwEffect);
	return S_OK;
}

HRESULT BaseListView::ProcessorDragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	*pdwEffect = (NULL != pDragData) ? pDragData->Process(&ptl, *pdwEffect) : DROPEFFECT_NONE;
	return S_OK;
}

HRESULT BaseListView::ProcessorDragLeave()
{
	if (NULL != pDragData)
	{
		delete pDragData;
		pDragData = NULL;
	}

	if (skinned)
		MLSkinnedScrollWnd_UpdateBars(hwnd, TRUE);

	if (NULL != pActiveDropSource)
		pActiveDropSource->SetFlags(DROPSOURCE_NORMALMODE, DROPSOURCE_REARRANGEMODE);
	return S_OK;
}

HRESULT BaseListView::ProcessorDrop(IClipboardFormatProcessor *processor, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	INT iInsert;
	
	if (NULL != pDragData)
	{
		iInsert = pDragData->GetInsertPoint(&ptl);
		pDragData->RemoveMark(FALSE);
		delete pDragData;
		pDragData = NULL;
	}
	else 
		iInsert = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	

	if (iInsert < 0) iInsert = 0;

	if (NULL != pActiveDocument)
	{
		if ((size_t)iInsert > pActiveDocument->GetItemCount())
			iInsert = (INT)pActiveDocument->GetItemCount(); 
	}

	
	if (NULL != processor && SUCCEEDED(processor->Process(iInsert)))
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = DROPEFFECT_NONE;
			
	if (skinned)
		MLSkinnedScrollWnd_UpdateBars(hwnd, TRUE);

	return S_OK;
}

STDMETHODIMP BaseListView::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{		
	IClipboardFormatProcessor *processor;
	if (0 != ((DROPEFFECT_COPY | DROPEFFECT_LINK)& *pdwEffect))
	{		
		HRESULT hr = CreateDataObectProcessor(pDataObject, GetParent(hwnd), &processor, DATAOBJECT_VIEWITEMS | DATAOBJECT_HDROP);
		if (SUCCEEDED(hr) && NULL != processor)
		{
			hr = ProcessorDragEnter(processor, grfKeyState, ptl, pdwEffect);
			processor->Release();
			if (FAILED(hr))
			{
				*pdwEffect = DROPEFFECT_NONE;
				return hr;
			}
		}
	}
	return DropboxView::DragEnter(pDataObject, grfKeyState, ptl, pdwEffect);
}

STDMETHODIMP BaseListView::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{	
	ProcessorDragOver(grfKeyState, ptl, pdwEffect);
	
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VSCROLL & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VSCROLL);

	HRESULT hr = DropboxView::DragOver(grfKeyState, ptl, pdwEffect);

	if (0 != (WS_VSCROLL & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VSCROLL);

	return hr;
}

STDMETHODIMP BaseListView::DragLeave(void)
{	
	ProcessorDragLeave();
	return DropboxView::DragLeave();
}

STDMETHODIMP BaseListView::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	IClipboardFormatProcessor *processor;
	processor = (NULL != pDragData) ? pDragData->GetProcessor() : NULL;

	if (NULL != processor)	
		processor->AddRef();

	ProcessorDrop(processor, grfKeyState, ptl, pdwEffect);

	if (NULL != processor)	
		processor->Release();

	return DropboxView::Drop(pDataObject, grfKeyState, ptl, pdwEffect);
}



void BaseListView::OnMediaLibraryDragDrop(INT code, mlDropItemStruct *pdis)
{	
	HRESULT hr;
	POINTL ptl;
	DWORD dwEffect;

	if (NULL != pdis) { ptl.x = pdis->p.x; ptl.y = pdis->p.y; }
    else { ptl.x = 0; ptl.y = 0; }

	switch(code)
	{
		case DRAGDROP_DRAGENTER:
			if (MlDropItemProcessor::CanProcess(pdis))
			{
				MlDropItemProcessor *processor = new MlDropItemProcessor(pdis, GetParent(hwnd));
				dwEffect = DROPEFFECT_COPY;
				hr = ProcessorDragEnter(processor, 0, ptl, &dwEffect);
				if (SUCCEEDED(hr) && 0 != ((DROPEFFECT_COPY | DROPEFFECT_LINK) & dwEffect))
				{
					pdis->result = 1;
					SetCursor(GetOleCursorFromDropEffect(DROPEFFECT_COPY));
					pdis->flags |= ML_HANDLEDRAG_FLAG_NOCURSOR;
				}
			}
			break;
		case DRAGDROP_DRAGLEAVE:
			ProcessorDragLeave();
			break;
		case DRAGDROP_DRAGOVER:
			dwEffect = DROPEFFECT_COPY;
			if (SUCCEEDED(ProcessorDragOver(0, ptl, &dwEffect)) &&
				0 != ((DROPEFFECT_COPY | DROPEFFECT_LINK) & dwEffect))
			{
				pdis->result = 1;
				SetCursor(GetOleCursorFromDropEffect(dwEffect));
				pdis->flags |= ML_HANDLEDRAG_FLAG_NOCURSOR;
			}
			break;
		case DRAGDROP_DROP:
			{
				MlDropItemProcessor *processor = (NULL != pDragData) ? (MlDropItemProcessor*)pDragData->GetProcessor() : NULL;
				if (NULL != processor)
				{
					processor->SetDropItem(pdis);
					processor->AddRef();
				}
				dwEffect = DROPEFFECT_COPY;
				ProcessorDrop(processor, 0, ptl, &dwEffect);
				if (NULL != processor)
					processor->Release();
			}
			break;
	}
}
