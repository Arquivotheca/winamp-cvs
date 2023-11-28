#ifndef _FILENAME_H
#define _FILENAME_H

#include <bfc/string/StringW.h>
#include <bfc/string/playstring.h>
#include <bfc/dispatch.h>

// a simple class to drag-and-drop filenames around

#define DD_FILENAME L"DD_Filename v1"

class Filename : public Dispatchable {
public:
  const wchar_t *getFilename();
  operator const wchar_t *() { return getFilename(); }

  static const wchar_t *dragitem_getDatatype() { return DD_FILENAME; }

protected:
  enum {
    GETFILENAME=100,
  };
};

inline const wchar_t *Filename::getFilename() {
  return _call(GETFILENAME, (const wchar_t *)NULL);
}

// a basic implementation to use
class FilenameI : public Filename, public StringW
{
public:
  FilenameI(const wchar_t *nf) : StringW(nf) { }
  const wchar_t *getFilename() { return getValue(); }

protected:
  FilenameI(const FilenameI &fn) {}
  FilenameI& operator =(const FilenameI &ps) { return *this; }
  RECVS_DISPATCH;
};

// another implementation that doesn't provide its own storage
class FilenameNC : public Filename {
public:
  FilenameNC(const wchar_t *str) { fn = str; }
  const wchar_t *getFilename() { return fn; }

protected:
  RECVS_DISPATCH;

private:
  const wchar_t *fn;
};

// another implementation that uses the central playstring table
class FilenamePS : public Filename, private Playstring
{
public:
  FilenamePS(const wchar_t *str) : Playstring(str) {}
  const wchar_t *getFilename() { return getValue(); }

protected:
  FilenamePS(const FilenamePS &fn) {}
  FilenamePS& operator =(const FilenamePS &ps) { return *this; }

  RECVS_DISPATCH;
};

#endif
