#ifndef __UMMFP_H__
#define __UMMFP_H__
#include "dap_vfs.h"

typedef void (*create_init_files_cb)();

rt_err_t register_ummfp_partition(const char* partition_name);
rt_err_t ummfp_init(create_init_files_cb create_files_fn, partition_writted_done_cb written_done_fn);

#endif //__UMMFP_H__