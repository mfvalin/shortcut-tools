#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define MAX_ENTRIES 200

usage(char *str) {
  fprintf(stderr,"Usage: %s [--lib] [--debug] [--verbose] [--max maxdirsize] [--prefix prefix] [--optimize list:of:start:of:paths ] to_scan1 target1 ... to_scanN targetN \n",str);
  exit(1);
}

main(int argc, char **argv){

  int entries=0;
  int file_count = 0;
  DIR * dirp;
  struct dirent * entry;
  char old_path[4096], new_path[4096];
  int status;
  char *so_string=".so";
  int look_for_so=0;
  char *my_name = argv[0];
  char *dir_prefix="";
  char *opt_prefix="";
  int max_entries=MAX_ENTRIES;
  int debug=0;
  int verbose=0;

  if(argc <= 1) usage(my_name);
  while(*argv[1] == '-' ){   // process options

    if(strcmp("--help",argv[1]) == 0 || strcmp("-h",argv[1]) == 0 ) {
      usage(my_name);
    }
    if(strcmp("--lib",argv[1]) == 0) {  /* library path mode, look for .so */
      argc--;
      argv++;
      look_for_so=1;
      goto loop;
    }
    if(strcmp("--debug",argv[1]) == 0) {  /* library path mode, look for .so */
      argc--;
      argv++;
      debug=1;
      goto loop;
    }
    if(strcmp("--verbose",argv[1]) == 0) {  /* library path mode, look for .so */
      argc--;
      argv++;
      verbose=1;
      goto loop;
    }
    if(strcmp("--max",argv[1]) == 0) {  /* library path mode, look for .so */
      max_entries = atoi(argv[2]);
      if(argc <= 2) usage(my_name);
      argc-=2;
      argv+=2;
      if(verbose) fprintf(stderr,"DEBUG: max entries = %d\n",max_entries);
      goto loop;
    }
    if(strcmp("--prefix",argv[1]) == 0) {  /* library path mode, look for .so */
      if(argc <= 2) usage(my_name);
      dir_prefix = argv[2];
      argc-=2;
      argv+=2;
      if(verbose) fprintf(stderr,"DEBUG: target prefix = '%s'\n",dir_prefix);
      goto loop;
    }
    if(strcmp("--optimize",argv[1]) == 0) {  /* library path mode, look for .so */
      if(argc <= 2) usage(my_name);
      opt_prefix = argv[2];
      argc-=2;
      argv+=2;
      if(verbose) fprintf(stderr,"DEBUG: to optimize with links : '%s'\n",opt_prefix);
      goto loop;
    }
    fprintf(stderr,"ERROR: unrecognized option '%s' ignored\n",argv[1]);
    if(! debug) usage(my_name);
    argc--;
    argv++;
loop:
    if(argc <= 1) usage(my_name);
  }

  while(argc > 2) {   // process directory pairs

    if(debug) fprintf(stderr,"processing '%s' -> '%s'\n",argv[1],argv[2]);
    dirp = opendir(argv[1]);
    if(dirp == NULL) { /* error while trying to open directory, next */
      fprintf(stderr,"ERROR: cannot open '%s'\n",argv[1]);
      goto loop2;
    }

    entries = 0;
    while ((entry = readdir(dirp)) != NULL) {
      entries++;
      if(entries > max_entries){
      if(verbose) fprintf(stderr,"INFO: more than %d entries found in '%s', IGNORING IT\n",max_entries,argv[1]);
	closedir(dirp);
	goto loop2;  /* too many entries, next */
      }
    }
    rewinddir(dirp);
    if(verbose) fprintf(stderr,"DEBUG: %d entries found in '%s'\n",entries,argv[1]);

    while ((entry = readdir(dirp)) != NULL) {           // process directory
	if(strcmp("." ,entry->d_name) == 0) continue;
	if(strcmp("..",entry->d_name) == 0) continue;
#ifdef __linux
	if (entry->d_type == DT_DIR) { /* If the entry is a directory (linux only feature) */
	  continue;
	}
#endif
// 	if(debug) fprintf(stderr,"DEBUG: found '%s'\n",entry->d_name);
	snprintf(old_path,sizeof(old_path),"%s/%s",argv[1],entry->d_name);
	snprintf(new_path,sizeof(new_path),"%s/%s/%s",dir_prefix,argv[2],entry->d_name);

    /*    if(NULL != strstr(entry->d_name,so_string)) fprintf(stderr,".so file located\n");   */
	if(look_for_so & (NULL == strstr(entry->d_name,so_string))) continue;

	status = symlink(old_path,new_path);
//         if(debug) fprintf(stderr,"DEBUG: %d ln -s %s %s\n",status,old_path,new_path);
	if(status==0)file_count++;
    }
    /*fprintf(stderr,"Linked %d files from %s into %s\n",file_count,argv[1],argv[2]);   */
    closedir(dirp);
loop2:
    argc-=2;
    argv+=2;
  }
  exit(0);
}
