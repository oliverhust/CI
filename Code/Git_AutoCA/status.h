#ifndef __status_h__
#define __status_h__


//状态设置，状态有hello,fine,start,CAing,CA_finish,stop,lock_all几种，平常是stop
errno_t status_set(char *status);

//状态设置
errno_t status_get(char *status);


#endif




