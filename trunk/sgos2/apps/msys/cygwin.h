#include <sgos.h>
/* External interface stuff */

typedef enum
{
	CW_LOCK_PINFO,
	CW_UNLOCK_PINFO,
	CW_GETTHREADNAME,
	CW_GETPINFO,
	CW_SETPINFO,
	CW_SETTHREADNAME,
	CW_GETVERSIONINFO,
	CW_READ_V1_MOUNT_TABLES,
	CW_USER_DATA,
	CW_PERFILE,
	CW_GET_CYGDRIVE_PREFIXES,
	CW_GETPINFO_FULL,
	CW_INIT_EXCEPTIONS,
	CW_GET_CYGDRIVE_INFO
} cygwin_getinfo_types;

struct per_process
{
  char *initial_sp;

  /* The offset of these 3 values can never change. */
  /* magic_biscuit is the size of this class and should never change. */
  unsigned long magic_biscuit;
  unsigned long dll_major;
  unsigned long dll_minor;

  struct _reent **impure_ptr_ptr;
  char ***envptr;

  /* Used to point to the memory machine we should use.  Usually these
     point back into the dll, but they can be overridden by the user. */
  void *(*malloc)(size_t);
  void (*free)(void *);
  void *(*realloc)(void *, size_t);

  int *fmode_ptr;

  int (*main)(int, char **, char **);
  void (**ctors)(void);
  void (**dtors)(void);

  /* For fork */
  void *data_start;
  void *data_end;
  void *bss_start;
  void *bss_end;

  void *(*calloc)(size_t, size_t);
  /* For future expansion of values set by the app. */
  void (*premain[4]) (int, char **, struct per_process *);

  /* The rest are *internal* to cygwin.dll.
     Those that are here because we want the child to inherit the value from
     the parent (which happens when bss is copied) are marked as such. */

  /* non-zero of ctors have been run.  Inherited from parent. */
  int run_ctors_p;

  uint unused[7];

  /* Non-zero means the task was forked.  The value is the pid.
     Inherited from parent. */
  int forkee;

  uint hmodule;

  uint api_major;		/* API version that this program was */
  uint api_minor;		/*  linked with */
  /* For future expansion, so apps won't have to be relinked if we
     add an item. */
  uint unused2[5];

  void *resourcelocks;
  void *threadinterface;
  struct _reent *impure_ptr;
};
#define per_process_overwrite ((unsigned) &(((struct per_process *) NULL)->resourcelocks))


void build_argv (char *cmd, char ***argv_ptr, int *argc, char sep);

