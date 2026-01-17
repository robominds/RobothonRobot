int val_set_init(int dbg);
int val_set_def_float(float *val, char *name, float min, float max);
int val_set_def_int(int *val, char *name, int min, int max);
int val_set_def_long(long *val, char *name, long min, long max);
int val_set_prc(int mode, char *line);
int val_set_list(char *name);
int val_set_save(char *filename);
int val_set_load(char *filename);

