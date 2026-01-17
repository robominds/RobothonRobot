int tpu_init(void);
int tpu_set_cier(int chan, int cie);
int tpu_set_hsqr(int chan, int hsq);
int tpu_set_hsrr(int chan, int hsr);
int tpu_set_cpr(int chan, int pri);
int tpu_set_cfsr(int chan, int fn);
int tpu_get_cisr(int chan);
