#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define MAX_ENTRIES 200

usage(char *str) {
  fprintf(stderr,"Usage: %s [--lib] [--debug] [--verbose] [--strict] [--multi] [--max maxdirsize] [--prefix prefix] \\\n");
  fprintf(stderr,"          [--optimize list:of:start:of:paths ] [--ignore list:of:start:of:paths] \\\n");
  fprintf(stderr,"          to_scan1 target1 ... to_scanN targetN \n",str);
  exit(1);
}

main(int argc, char **argv){

  int entries=0;
  int file_count = 0;
  DIR * dirp;
  struct dirent * entry;
  char old_path[4096], new_path[4096], target_subdir[128] ;
  int status;
  char *so_string=".so";
  int look_for_so=0;
  char *my_name = argv[0];
  char *dir_prefix="";
  char *opt_prefix="";
  char *opt_prefixes[128];
  char *noopt_prefix="";
  char *noopt_prefixes[128];
  char *temp, *cprefix;
  int i;
  int max_entries=MAX_ENTRIES;
  int debug=0;
  int verbose=0;
  int strict=0;
  int multi=0;
  int optimize;
  int create=0;
  int created=-1;

  if(argc <= 1) usage(my_name);

  for(i=0 ; i<128; i++) opt_prefixes[i] = NULL ;
  for(i=0 ; i<128; i++) noopt_prefixes[i] = NULL ;
/*============================== process options ==============================*/
  while(argc > 1){   // process options

    if(*argv[1] != '-' ) break ;

    if(strcmp("--help",argv[1]) == 0 || strcmp("-h",argv[1]) == 0 ) {
      usage(my_name);
    }
    else if(strcmp("--lib",argv[1]) == 0) {  /* library path mode, look for .so */
      argc--;
      argv++;
      look_for_so=1;
      continue;
    }
    else if(strcmp("--multi",argv[1]) == 0) {  // multiple sources ito same target
      argc--;
      argv++;
      multi=1;
      continue;
    }
    else if(strcmp("--strict",argv[1]) == 0) {  // strict mode, abort on more errors
      argc--;
      argv++;
      strict=1;
      continue;
    }
    else if(strcmp("--debug",argv[1]) == 0) {  // debug mode
      argc--;
      argv++;
      debug=1;
      continue;
    }
    else if(strcmp("--verbose",argv[1]) == 0) {  // verbose mode
      argc--;
      argv++;
      verbose=1;
      continue;
    }
    else if(strcmp("--max",argv[1]) == 0) {  // max number of links from a single directory
      max_entries = atoi(argv[2]);
      if(argc <= 2) usage(my_name);
      argc-=2;
      argv+=2;
      if(verbose) fprintf(stderr,"DEBUG: max entries = %d\n",max_entries);
      continue;
    }
    else if(strcmp("--prefix",argv[1]) == 0) {  // prefix for target directories
      if(argc <= 2) usage(my_name);
      dir_prefix = argv[2];
      argc-=2;
      argv+=2;
      if(verbose) fprintf(stderr,"DEBUG: target prefix = '%s'\n",dir_prefix);
      continue;
    }
    else if(strcmp("--optimize",argv[1]) == 0) {  // directory start pattern to optimize
      if(argc <= 2) usage(my_name);
      opt_prefix = argv[2];
      if(verbose) fprintf(stderr,"DEBUG: paths to optimize with links : '%s'\n",argv[2]);
      argc-=2;
      argv+=2;
      i = 0; temp = opt_prefix; opt_prefixes[0] = temp ;
      while(*temp) {
	if(*temp == ':') { *temp = '\0' ; temp++ ; opt_prefixes[++i] = temp; }
	temp++;
      }
      if(debug) fprintf(stderr,"DEBUG: %d elements to optimize found\n",i+1);
      continue;
    }
    else if(strcmp("--ignore",argv[1]) == 0) {  // directory start pattern to ignore
      if(argc <= 2) usage(my_name);
      noopt_prefix = argv[2];
      if(verbose) fprintf(stderr,"DEBUG: paths to ignore : '%s'\n",argv[2]);
      argc-=2;
      argv+=2;
      i = 0; temp = noopt_prefix; noopt_prefixes[0] = temp ;
      while(*temp) {
	if(*temp == ':') { *temp = '\0' ; temp++ ; noopt_prefixes[++i] = temp; }
	temp++;
      }
      if(debug) fprintf(stderr,"DEBUG: %d elements to ignore found\n",i+1);
      continue;
    }
    else {
      fprintf(stderr,"ERROR: unrecognized option '%s' ignored\n",argv[1]);
      if(! debug) usage(my_name);
      argc--;
      argv++;
    }
  }   // process options loop end
/*=============================================================================*/
/*============================ process directories ============================*/
  cprefix = "";  // separator for output list
  for( ; argc > 1 ; argc--, argv++, cprefix = ":" ) {   // process directories in path

    if(debug) fprintf(stderr,"processing '%s' -> '%s'\n",argv[1],argv[2]);
    optimize = 1;    // optimize a priori
//  --ignore option
    for(i=0 ; noopt_prefixes[i] ; i++) {   // scan the prefix to ignore list
      if( strstr(argv[1],noopt_prefixes[i]) == argv[1] ) {      // we have a hit
	if(verbose) fprintf(stderr,"INFO: ignoring '%s'\n",argv[1]);
	optimize = 0 ;
	break ;
      }
    }
//  --optimize option
    if(optimize && opt_prefixes[0] != NULL) {    // if no "optimize prefix" list, process everything not already ignored
      optimize = 0;
      for(i=0 ; opt_prefixes[i] ; i++) {   // scan the prefix to optimize list
	if( strstr(argv[1],opt_prefixes[i]) == argv[1] ) {      // we have a hit
	  if(verbose) fprintf(stderr," optimizing '%s'\n",argv[1]);
	  optimize = 1;
	  break;
	}
      }
      if(optimize == 0 && verbose) fprintf(stderr,"INFO: ignoring '%s'\n",argv[1]);
    }

    if(optimize == 0){
      fprintf(stdout,"%s%s",cprefix,argv[1]); // print ignored path directory
      continue ;
    }

    dirp = opendir(argv[1]);
    if(dirp == NULL) { /* error while trying to open directory, next */
      fprintf(stderr,"ERROR: cannot open '%s'\n",argv[1]);
      fprintf(stdout,"%s%s",cprefix,argv[1]); // print ignored path directory
      if(strict) exit(1);
      continue;
    }

    entries = 0;
    while ((entry = readdir(dirp)) != NULL) {   // count entries in directory
#ifdef __linux
      if (entry->d_type == DT_DIR) continue ;   // ignore sub-directory entries (linux only feature)
#endif
      entries++;
      if(entries > max_entries){
	if(verbose) fprintf(stderr,"INFO: more than %d entries found in '%s', IGNORING IT\n",max_entries,argv[1]);
	optimize = 0;
	break;  /* too many entries, next */
      }
    }
    if(verbose) fprintf(stderr,"DEBUG: %d entries found in '%s'\n",entries,argv[1]);

    if(optimize == 0) {
      fprintf(stdout,"%s%s",cprefix,argv[1]); // print ignored path directory
      create++ ;
    }else{
      if(! multi) create++ ;
      rewinddir(dirp) ;
      snprintf(target_subdir,sizeof(target_subdir),"cache_%4.4d",create) ;
      snprintf(new_path,sizeof(new_path),"%s/%s",dir_prefix,target_subdir) ;  // target sub directory
      if(create != created) {                     // if not already done
	if(mkdir(new_path,0700)) {                    // create target sub directory
	  fprintf(stdout,"%s%s",cprefix,argv[1]);     // mkdir failed, keep original path
	  optimize = 0 ;
	}else{
	  fprintf(stdout,"%s%s",cprefix,new_path);  // print optimized path directory
	}
      }
      created = create ;
      if(optimize) {                                      // will be false if mkdir failed
	while ((entry = readdir(dirp)) != NULL) {         // process "directory to optimize" entries
	  if(strcmp("." ,entry->d_name) == 0) continue;   // ignore . and ..
	  if(strcmp("..",entry->d_name) == 0) continue;
#ifdef __linux
	  if (entry->d_type == DT_DIR) { /* If the entry is a directory (linux only feature) */
	    continue;
	  }
#endif
	  snprintf(old_path,sizeof(old_path),"%s/%s",argv[1],entry->d_name);                // source
	  snprintf(new_path,sizeof(new_path),"%s/%s/%s",dir_prefix,target_subdir,entry->d_name);  // target

//        if(NULL != strstr(entry->d_name,so_string)) fprintf(stderr,".so file located\n");   */
	  if(look_for_so & (NULL == strstr(entry->d_name,so_string))) continue;

	  status = symlink(old_path,new_path);
	  if(status==0)file_count++;
	}
      }
    }
    /*fprintf(stderr,"Linked %d files from %s into %s\n",file_count,argv[1],argv[2]);   */
    closedir(dirp);
  }
/*=============================================================================*/
  exit(0);
}
