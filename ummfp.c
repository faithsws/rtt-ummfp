
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <fal.h>
#include "dap_vfs.h"
#include "ummfp.h"

#define LOG_TAG "ummfp"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

struct partition_name_t{
	char name[RT_NAME_MAX];
	rt_slist_t                 object_list;  
};

struct ummfp_t;

struct ummfp_t{
	struct rt_device_blk_geometry geometry;
	struct rt_device dev;
}UMMFP;


static rt_slist_t partition_names = RT_SLIST_OBJECT_INIT(partition_names);

/***************************************************************************
* Function     : check_allowed_file
* Description  : 检查文件名是否是注册列表里 允许的名称
* Author       : songwensheng@hc
* Date         : 2020/11/6
* Return       : 如果允许，返回分区名称，否则返回NULL
* Parameters   : 文件名
* Notes        : 只检查跟分区名 相同长度部分
***************************************************************************/

static const char* check_allowed_file(vfs_filename_t filename)
{
	//filename in allowed list
	rt_slist_t * l = RT_NULL;
	struct partition_name_t * pname = RT_NULL;
	LOG_D("check_allowed_file filename = [%8s]",filename);

	rt_slist_for_each(l,&partition_names)
	{
		pname = rt_slist_entry(l,struct partition_name_t,object_list);
		LOG_D("allowed name = [%s]",pname->name);
		if(strncasecmp(pname->name,filename,strlen(pname->name)) == 0)
		{
			return pname->name;
		}
	
	}	
	return NULL;
}

#define PKG_UMMFP_README

#ifdef PKG_UMMFP_README

static uint32_t build_readme_text(uint8_t *buf)
{
	//char buf_temp[512] = "You can only write files BELOW according to your partition Config.\r\n";
	char buf_temp[512] = "根据分区配置，你可以写入以下名称的文件。文件名最长8个字符，必须带3字母后缀，后缀任意。\r\n以rst为后缀的文件写入后会自动重启设备\r\n";
	char line_temp[128];
	rt_slist_t * l = RT_NULL;
	struct partition_name_t * pname = RT_NULL;

	sprintf(line_temp,"\t%08s\t%08s\r\n", "FileName","Size");
	strcat(buf_temp,line_temp);
	rt_slist_for_each(l,&partition_names)
	{
		pname = rt_slist_entry(l,struct partition_name_t,object_list);
		
		const struct fal_partition * p = fal_partition_find(pname->name);
		sprintf(line_temp,"\t%08s\t%08d(%dK)\r\n", pname->name,p->len,p->len/1024);
		strcat(buf_temp,line_temp);
	}

	if(buf)
	{
		strcpy((char*)buf,buf_temp);
	}
	return strlen(buf_temp);
}

static uint32_t read_readme(struct virtual_media* vm, uint32_t sector_offset, uint8_t *data, uint32_t num_sectors)
{
	uint32_t len = build_readme_text(data);
	return len;
}
#endif

/***************************************************************************
* Function     : create_init_files
* Description  : 创建硬盘中初始的文件
* Author       : songwensheng@hc
* Date         : 2020/11/6
* Return       : 
* Parameters   : 
* Notes        : 一般都是说明性的 只读文件
***************************************************************************/

static rt_err_t create_init_files(create_init_files_cb create_files_fn)
{
#ifdef PKG_UMMFP_README
	uint32_t len = build_readme_text(NULL);
	vfs_create_file("README  TXT", read_readme, NULL,len);
#endif
	if(create_files_fn)
	{
		create_files_fn();
	}
	return RT_EOK;
}

/***************************************************************************
* Function     : register_ummfp_partition
* Description  : 把fal partition表中，有效的分区名称，注册到UMMFP
* Author       : songwensheng@hc
* Date         : 2020/11/6
* Return       : 
* Parameters   : 分区名称
* Notes        : 
***************************************************************************/

rt_err_t register_ummfp_partition(const char* partition_name)
{
	rt_err_t result = RT_EOK;

	rt_slist_t * l = RT_NULL;
	struct partition_name_t * pname = RT_NULL;
	
		
	rt_slist_for_each(l,&partition_names)
	{
		pname = rt_slist_entry(l,struct partition_name_t,object_list);
		
		if(strncmp(pname->name,partition_name,RT_NAME_MAX) == 0)
		{
			return -RT_EINVAL;
		}
	
	}
	if(fal_partition_find(partition_name) == RT_NULL)
	{
		return -RT_EINVAL;
	}
	
	pname = (struct partition_name_t*)rt_malloc(sizeof(struct partition_name_t));
	if(pname == RT_NULL)return -RT_ENOMEM;
	rt_slist_append(&partition_names, &(pname->object_list));
	rt_strncpy(pname->name, partition_name, RT_NAME_MAX);

	
	return result;
}

static rt_err_t rt_ummfp_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t rt_ummfp_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_ummfp_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t rt_ummfp_control(rt_device_t dev, int cmd, void *args)
{
    struct ummfp_t * ummfp = (struct ummfp_t *)dev->user_data;
    switch (cmd)
    {
    case RT_DEVICE_CTRL_BLK_GETGEOME:
        rt_memcpy(args, &ummfp->geometry, sizeof(struct rt_device_blk_geometry));
        break;
    default:
        break;
    }
    return RT_EOK;
}

static rt_size_t rt_ummfp_read(rt_device_t dev,
                               rt_off_t    pos,
                               void       *buffer,
                               rt_size_t   size)
{
	vfs_read(pos, (uint8_t *)buffer, size);
	return size;
}
static rt_size_t rt_ummfp_write(rt_device_t dev,
                                rt_off_t    pos,
                                const void *buffer,
                                rt_size_t   size)
{
	
	vfs_write(pos, buffer, size);
	return 0;
}
#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops ummfp_blk_ops = 
{
    rt_ummfp_init,
    rt_ummfp_open,
    rt_ummfp_close,
    rt_ummfp_read,
    rt_ummfp_write,
    rt_ummfp_control
};
#endif



rt_err_t ummfp_init(create_init_files_cb create_files_fn, partition_writted_done_cb written_done_fn)
{
	rt_err_t result = RT_EOK;
	struct ummfp_t * pUMMFP = &UMMFP;
	
	//init dummy fat16 filesystem
	vfs_init("UMMFP",VFS_DISK_SIZE,check_allowed_file,written_done_fn);
	
	create_init_files(create_files_fn);

	
	pUMMFP->geometry.sector_count = VFS_DISK_SIZE / VFS_SECTOR_SIZE;
	pUMMFP->geometry.bytes_per_sector = VFS_SECTOR_SIZE;
	pUMMFP->geometry.block_size = VFS_SECTOR_SIZE; //没有用途
	
	
	pUMMFP->dev.type = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    pUMMFP->dev.ops  = &ummfp_blk_ops;
#else
	pUMMFP->dev.init = rt_ummfp_init;
	pUMMFP->dev.open = rt_ummfp_open;
	pUMMFP->dev.close = rt_ummfp_close;
	pUMMFP->dev.read = rt_ummfp_read;
	pUMMFP->dev.write = rt_ummfp_write;
	pUMMFP->dev.control = rt_ummfp_control;
#endif
	pUMMFP->dev.user_data = pUMMFP;
	
	result = rt_device_register(&pUMMFP->dev, "ummfp0",
							RT_DEVICE_FLAG_RDWR |  RT_DEVICE_FLAG_STANDALONE);
	
	LOG_I("ummfp block device registered = %d",result);
	return result;
	
}
/* 使用方法
	1. 在USB设备选项中打开 MSC  #define RT_USB_DEVICE_MSTORAGE
	2. 确保  #define RT_USB_MSTORAGE_DISK_NAME "ummfp0"
	3. 确保正确的fal 分区，并在ummfp_init 之前，注册 分区名称
	4. 在U盘设备枚举前完成 ummfp_init
	
	2020/11/6 songwensheng@hc */

static void partition_written_done( const char*name, uint32_t size)
{
	LOG_I("Partition [%s] written %d", name, size);
}
void ummfp_test()
{
	register_ummfp_partition("hzk12");
	register_ummfp_partition("msc");
	register_ummfp_partition("part1");

	ummfp_init(RT_NULL,partition_written_done);
}