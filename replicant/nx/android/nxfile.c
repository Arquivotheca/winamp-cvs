#include "nxfile.h"
#include "foundation/error.h"


#include <fcntl.h>
#include <unistd.h>

int copy_file(const char *to, const char *from)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}
/* Android implementation */
int NXFile_move(nx_uri_t destination, nx_uri_t source)
{
	if (rename(source->string, destination->string) == 0)
		return NErr_Success;
	else
	{
		if (copy_file(destination->string, source->string) == 0)
			return NErr_Error;
		unlink(source->string);
		return NErr_Success;
	}
		
}

int NXFile_unlink(nx_uri_t filename)
{
	if (unlink(filename->string) == 0)
		return NErr_Success;
	
	/* TODO: check errno */
	return NErr_Error;
}

int NXFile_stat(nx_uri_t filename, nx_file_stat_t file_stats)
{
	struct stat buffer;

	if (stat(filename->string, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atime;
		file_stats->creation_time = buffer.st_ctime;
		file_stats->modified_time = buffer.st_mtime;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int NXFile_statFILE(FILE *f, nx_file_stat_t file_stats)
{
	int fd = fileno(f);
	if (fd == -1)
		return NErr_Error;

	return NXFile_fstat(fd, file_stats);
}

int NXFile_fstat(int file_descriptor, nx_file_stat_t file_stats)
{
	struct stat buffer; 

	if (fstat(file_descriptor, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atime;
		file_stats->creation_time = buffer.st_ctime;
		file_stats->modified_time = buffer.st_mtime;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}	

#if 0
int mkdirp(const char *pathname, mode_t mode)
{
  char parent[PATH_MAX], *p;
  /* make a parent directory path */
  strncpy(parent, pathname, sizeof(parent));
  parent[sizeof(parent) - 1] = '\0';
  for(p = parent + strlen(parent); *p != '/' && p != parent; p--);
  *p = '\0';
  /* try make parent directory */
  if(p != parent && mkdirp(parent, mode) != 0)
    return -1;
  /* make this one if parent has been made */
  if(mkdir(pathname, mode) == 0)
    return 0;
  /* if it already exists that is fine */
  if(errno == EEXIST)
    return 0;
  return -1;
}
#endif