#ifndef __DAP_VFS_H__
#define __DAP_VFS_H__

#include <stdint.h>

#define VFS_CLUSTER_SIZE        0x1000
#define VFS_SECTOR_SIZE         512
#define VFS_INVALID_SECTOR      0xFFFFFFFF
#define VFS_FILE_INVALID        0
#define VFS_MAX_FILES           16

#define MB(size)                        ((size) * 1024 * 1024)
#define KB(size)                        ((size) * 1024)
#define VFS_DISK_SIZE (MB(64))
#if !defined(ARRAY_SIZE)
//! @brief Get number of elements in the array.
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif
#define util_assert RT_ASSERT
#define COMPILER_ASSERT(x) 
typedef char vfs_filename_t[11];
struct virtual_media;
// Callback for when data is written to a file on the virtual filesystem
typedef void (*vfs_write_cb_t)(struct virtual_media* vm, uint32_t sector_offset, const uint8_t *data, uint32_t num_sectors);
// Callback for when data is ready from the virtual filesystem
typedef uint32_t (*vfs_read_cb_t)(struct virtual_media* vm, uint32_t sector_offset, uint8_t *data, uint32_t num_sectors);

typedef const char* (*check_allowed_file_cb_t)(vfs_filename_t filename);

typedef struct virtual_media {
    vfs_read_cb_t read_cb;
    vfs_write_cb_t write_cb;
    uint32_t length;
	void *user_data;
	uint32_t start_offset;
} virtual_media_t;


#define FAT_CLUSTERS_MAX (65525 - 100)
#define FAT_CLUSTERS_MIN (4086 + 100)
typedef enum {
    VFS_FILE_ATTR_READ_ONLY     = (1 << 0),
    VFS_FILE_ATTR_HIDDEN        = (1 << 1),
    VFS_FILE_ATTR_SYSTEM        = (1 << 2),
    VFS_FILE_ATTR_VOLUME_LABEL  = (1 << 3),
    VFS_FILE_ATTR_SUB_DIR       = (1 << 4),
    VFS_FILE_ATTR_ARCHIVE       = (1 << 5),
} vfs_file_attr_bit_t;

typedef enum {
    VFS_FILE_CREATED = 0,   /*!< A new file was created */
    VFS_FILE_DELETED,       /*!< An existing file was deleted */
    VFS_FILE_CHANGED,       /*!< Some attribute of the file changed.
                                  Note: when a file is deleted or
                                  created a file changed
                                  notification will also occur*/
} vfs_file_change_t;
typedef struct {
    uint8_t boot_sector[11];
    /* DOS 2.0 BPB - Bios Parameter Block, 11 bytes */
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_logical_sectors;
    uint8_t  num_fats;
    uint16_t max_root_dir_entries;
    uint16_t total_logical_sectors;
    uint8_t  media_descriptor;
    uint16_t logical_sectors_per_fat;
    /* DOS 3.31 BPB - Bios Parameter Block, 12 bytes */
    uint16_t physical_sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t big_sectors_on_drive;
    /* Extended BIOS Parameter Block, 26 bytes */
    uint8_t  physical_drive_number;
    uint8_t  not_used;
    uint8_t  boot_record_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     file_system_type[8];
    /* bootstrap data in bytes 62-509 */
    uint8_t  bootstrap[448];
    /* These entries in place of bootstrap code are the *nix partitions */
    //uint8_t  partition_one[16];
    //uint8_t  partition_two[16];
    //uint8_t  partition_three[16];
    //uint8_t  partition_four[16];
    /* Mandatory value at bytes 510-511, must be 0xaa55 */
    uint16_t signature;
} __attribute__((packed)) mbr_t;

typedef struct file_allocation_table {
    uint8_t f[512];
} file_allocation_table_t;

typedef struct FatDirectoryEntry {
    vfs_filename_t filename;
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_ms;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high_16;
    uint16_t modification_time;
    uint16_t modification_date;
    uint16_t first_cluster_low_16;
    uint32_t filesize;
} __attribute__((packed)) FatDirectoryEntry_t;

// to save RAM all files must be in the first root dir entry (512 bytes)
//  but 2 actually exist on disc (32 entries) to accomodate hidden OS files,
//  folders and metadata
typedef struct root_dir {
    FatDirectoryEntry_t f[32];
} root_dir_t;

enum virtual_media_idx_t {
    MEDIA_IDX_MBR = 0,
    MEDIA_IDX_FAT1,
    MEDIA_IDX_FAT2,
    MEDIA_IDX_ROOT_DIR,

    MEDIA_IDX_COUNT
};

typedef void *vfs_file_t;
typedef uint32_t vfs_sector_t;

typedef void (*vfs_file_change_cb_t)(const vfs_filename_t filename, vfs_file_change_t change,
                                     vfs_file_t file, vfs_file_t new_file_data);

typedef void (*partition_writted_done_cb)(const char*partition_name, uint32_t len);

void vfs_init(const vfs_filename_t drive_name, uint32_t disk_size, check_allowed_file_cb_t cb, partition_writted_done_cb written_done_fn);
void vfs_write(uint32_t requested_sector, const uint8_t *buf, uint32_t num_sectors);
void vfs_read(uint32_t requested_sector, uint8_t *buf, uint32_t num_sectors);
vfs_file_t vfs_create_file(const vfs_filename_t filename, vfs_read_cb_t read_cb, vfs_write_cb_t write_cb, uint32_t len);
#endif