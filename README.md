# 使用方法

1. 在USB设备选项中打开 MSC  `#define RT_USB_DEVICE_MSTORAGE`
2. 确保  `#define RT_USB_MSTORAGE_DISK_NAME "ummfp0"`
3. 确保正确的fal 分区，并在ummfp_init 之前，注册 分区名称
4. 在U盘设备枚举前完成 ummfp_init

**功能依赖于fal 功能，需要提前配置好fal相关分区**

目前最大定义32个文件，但是由于win10会有隐藏和系统文件写入，会占用2~4个文件空间

---------------------------
主要结构体
---------------
```c
typedef void (*partition_writted_done_cb)(const char*partition_name, uint32_t len);
```
当某个分区写入完成，产生的回调。  
`partition_name` 分区名称  `len`写入的字节数

--------------------
```c
typedef void (*create_init_files_cb)();
```
用户用于创建初始文件的回调函数  
在回调函数中，可以使用
```c
vfs_file_t vfs_create_file(const vfs_filename_t filename, vfs_read_cb_t read_cb, vfs_write_cb_t write_cb, uint32_t len);
```
添加自己的文件。  
`filename`  文件名，注意为FAT16的格式。 前8字符为前缀，后3字符为后缀。前缀不足8个字符用空格代替填充。  
`read_cb`  读取文件时，调用的回调函数
`write_cb` 写入文件时， 调用的回调函数
`len`  文件长度。 需要在创建时确定 最大长度

------------------------------------
主要函数
-----------------------
```c
rt_err_t register_ummfp_partition(const char* partition_name);

```
添加允许写入的分区，对应为可以写入的文件前缀。如果要在readme.txt 中体现，需要在 `ummfp_init`之前调用。

```c
rt_err_t ummfp_init(create_init_files_cb create_files_fn, partition_writted_done_cb written_done_fn);
```
初始化整体功能

`create_files_fn`  创建用户文件回调
`written_done_fn` 分区写入结束回调

