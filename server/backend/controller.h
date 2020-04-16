#ifndef keyvalue_H
#define keyvalue_H

sds initdir_for_static_files(void);
sds serve_from_cache(void);
void add_to_cache(sds);
int path_traversal(void);
int isDirectory(const char);

#endif 
