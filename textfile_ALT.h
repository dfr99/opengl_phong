// textfile.h: interface for reading and writing text files
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//
// Slightly modified by Emilio J. Padrón
//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

char *textFileRead(const char *fn);
int textFileWrite(const char *fn, const char *s);

#ifdef __cplusplus
}
#endif
