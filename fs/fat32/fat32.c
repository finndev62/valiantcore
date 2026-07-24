

#include "../../include/kernel.h"
#include <stdint.h>


#define FAT32_EOC        0x0FFFFFFF
#define FAT32_FREE       0x00000000
#define FAT32_BAD        0x0FFFFFF7
#define FAT32_BOOT_SIG   0x29

#define ATTR_READ_ONLY   0x01
#define ATTR_HIDDEN      0x02
#define ATTR_SYSTEM      0x04
#define ATTR_VOLUME_ID   0x08
#define ATTR_DIRECTORY   0x10
#define ATTR_ARCHIVE     0x20
#define ATTR_LFN         0x0F

#define FAT32_MAX_FILES  512
#define FAT32_SECTOR_SZ  512
#define FAT32_NAME_LEN   64


typedef struct __attribute__((packed)) {
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sec;
    uint8_t  sec_per_clus;
    uint16_t rsvd_sec_cnt;
    uint8_t  num_fats;
    uint16_t root_ent_cnt;
    uint16_t tot_sec16;
    uint8_t  media;
    uint16_t fat_sz16;
    uint16_t sec_per_trk;
    uint16_t num_heads;
    uint32_t hidd_sec;
    uint32_t tot_sec32;
    uint32_t fat_sz32;
    uint16_t ext_flags;
    uint16_t fs_ver;
    uint32_t root_clus;
    uint16_t fs_info;
    uint16_t bk_boot_sec;
    uint8_t  reserved[12];
    uint8_t  drv_num;
    uint8_t  reserved1;
    uint8_t  boot_sig;
    uint32_t vol_id;
    uint8_t  vol_lab[11];
    uint8_t  fil_sys_type[8];
} fat32_bpb_t;


typedef struct __attribute__((packed)) {
    uint8_t  name[8];
    uint8_t  ext[3];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;
} fat32_dir_entry_t;


typedef struct {
    uint32_t first_cluster;
    uint32_t cur_cluster;
    uint32_t file_size;
    uint32_t pos;
    uint8_t  is_dir;
    uint8_t  active;
    uint8_t  name[FAT32_NAME_LEN];
} fat32_file_t;


typedef struct {
    fat32_bpb_t bpb;
    uint64_t    fat_start;
    uint64_t    data_start;
    uint32_t    root_cluster;
    uint8_t     sec_per_clus;
    uint8_t     initialized;
    fat32_file_t files[FAT32_MAX_FILES];
} fat32_state_t;

static fat32_state_t fat32;


extern int ata_read_250gb(uint64_t lba, uint16_t count, uint16_t *buf);
extern int ata_write_250gb(uint64_t lba, uint16_t count, const uint16_t *buf);


static uint8_t fat32_sector_buf[FAT32_SECTOR_SZ];


static uint64_t fat32_cluster_to_lba(uint32_t cluster) {
    if (cluster < 2) return 0;   
    return fat32.data_start +
           ((uint64_t)(cluster - 2) * fat32.sec_per_clus);
}


static int fat32_read_sector(uint64_t lba, uint8_t *buf) {
    if (!buf) return -1;
    int r = ata_read_250gb(lba, 1, (uint16_t *)buf);
    return r;
}


static int fat32_write_sector(uint64_t lba, const uint8_t *buf) {
    if (!buf) return -1;
    return ata_write_250gb(lba, 1, (const uint16_t *)buf);
}


static uint32_t fat32_read_fat_entry(uint32_t cluster) {
    uint64_t fat_offset  = (uint64_t)cluster * 4;
    uint64_t fat_sector  = fat32.fat_start + (fat_offset / FAT32_SECTOR_SZ);
    uint32_t entry_off   = (uint32_t)(fat_offset % FAT32_SECTOR_SZ);

    if (fat32_read_sector(fat_sector, fat32_sector_buf) != 0)
        return FAT32_BAD;

    uint32_t value;
    __builtin_memcpy(&value, fat32_sector_buf + entry_off, sizeof(uint32_t));
    return value & 0x0FFFFFFF;
}


static int fat32_write_fat_entry(uint32_t cluster, uint32_t value) {
    uint64_t fat_offset = (uint64_t)cluster * 4;
    uint64_t fat_sector = fat32.fat_start + (fat_offset / FAT32_SECTOR_SZ);
    uint32_t entry_off  = (uint32_t)(fat_offset % FAT32_SECTOR_SZ);

    if (fat32_read_sector(fat_sector, fat32_sector_buf) != 0)
        return -1;

    uint32_t val = (value & 0x0FFFFFFF);
    __builtin_memcpy(fat32_sector_buf + entry_off, &val, sizeof(uint32_t));

    return fat32_write_sector(fat_sector, fat32_sector_buf);
}


static uint32_t fat32_alloc_cluster(void) {
    uint32_t total_clusters =
        (fat32.bpb.tot_sec32 - (uint32_t)fat32.data_start)
        / fat32.sec_per_clus;

    for (uint32_t i = 2; i < total_clusters + 2; i++) {
        if (fat32_read_fat_entry(i) == FAT32_FREE) {
            fat32_write_fat_entry(i, FAT32_EOC);
            return i;
        }
    }
    return 0; 
}


int fat32_init(void) {
    if (fat32_read_sector(0, fat32_sector_buf) != 0) {
        kprint("[FAT32] Boot sector read failed\n");
        return -1;
    }

    __builtin_memcpy(&fat32.bpb, fat32_sector_buf, sizeof(fat32_bpb_t));

    
    if (fat32.bpb.boot_sig != FAT32_BOOT_SIG) {
        kprint("[FAT32] Invalid boot signature\n");
        return -1;
    }

    
    if (fat32.bpb.fat_sz32 == 0) {
        kprint("[FAT32] Not a FAT32 filesystem\n");
        return -1;
    }

    fat32.sec_per_clus  = fat32.bpb.sec_per_clus;
    fat32.fat_start     = fat32.bpb.rsvd_sec_cnt;
    fat32.root_cluster  = fat32.bpb.root_clus;
    fat32.data_start    = fat32.fat_start +
                         ((uint64_t)fat32.bpb.num_fats * fat32.bpb.fat_sz32);

    for (int i = 0; i < FAT32_MAX_FILES; i++)
        fat32.files[i].active = 0;

    fat32.initialized = 1;
    kprint("[FAT32] Initialized successfully\n");
    return 0;
}


static void fat32_8dot3_to_name(const uint8_t *name83, const uint8_t *ext,
                                 uint8_t *out, uint32_t out_len) {
    if (!name83 || !ext || !out || out_len == 0) return;

    uint32_t i = 0, j = 0;

    
    while (i < 8 && j < out_len - 1 && name83[i] != ' ')
        out[j++] = name83[i++];

    
    if (ext[0] != ' ' && j < out_len - 2) {
        out[j++] = '.';
        for (i = 0; i < 3 && j < out_len - 1 && ext[i] != ' '; i++)
            out[j++] = ext[i];
    }

    out[j] = '\0';
}


int fat32_ls(const char *path) {
    if (!fat32.initialized) {
        kprint("[FAT32] Not initialized\n");
        return -1;
    }

    
    uint32_t cluster = fat32.root_cluster;

    while (cluster < FAT32_EOC) {
        uint64_t lba = fat32_cluster_to_lba(cluster);

        uint32_t entries_per_clus =
            (fat32.sec_per_clus * FAT32_SECTOR_SZ) / sizeof(fat32_dir_entry_t);

        for (uint32_t sec = 0; sec < fat32.sec_per_clus; sec++) {
            if (fat32_read_sector(lba + sec, fat32_sector_buf) != 0)
                return -1;

            fat32_dir_entry_t *entries = (fat32_dir_entry_t *)fat32_sector_buf;
            uint32_t count = FAT32_SECTOR_SZ / sizeof(fat32_dir_entry_t);

            for (uint32_t i = 0; i < count; i++) {
                if (entries[i].name[0] == 0x00) return 0;   
                if (entries[i].name[0] == 0xE5) continue;   
                if (entries[i].attr == ATTR_LFN) continue;  
                if (entries[i].attr & ATTR_VOLUME_ID) continue;

                uint8_t name_buf[FAT32_NAME_LEN];
                fat32_8dot3_to_name(entries[i].name, entries[i].ext,
                                    name_buf, FAT32_NAME_LEN);

                if (entries[i].attr & ATTR_DIRECTORY)
                    kprint("[DIR]  ");
                else
                    kprint("       ");

                kprint((const char *)name_buf);
                kprint("\n");
            }
        }
        cluster = fat32_read_fat_entry(cluster);
    }
    return 0;
}


int fat32_open(const char *path, uint8_t write) {
    if (!fat32.initialized || !path) return -1;

    
    int fd = -1;
    for (int i = 0; i < FAT32_MAX_FILES; i++) {
        if (!fat32.files[i].active) { fd = i; break; }
    }
    if (fd < 0) { kprint("[FAT32] No free file slots\n"); return -1; }

    
    uint32_t cluster = fat32.root_cluster;

    while (cluster < FAT32_EOC) {
        uint64_t lba = fat32_cluster_to_lba(cluster);

        for (uint32_t sec = 0; sec < fat32.sec_per_clus; sec++) {
            if (fat32_read_sector(lba + sec, fat32_sector_buf) != 0)
                return -1;

            fat32_dir_entry_t *entries = (fat32_dir_entry_t *)fat32_sector_buf;
            uint32_t count = FAT32_SECTOR_SZ / sizeof(fat32_dir_entry_t);

            for (uint32_t i = 0; i < count; i++) {
                if (entries[i].name[0] == 0x00) goto not_found;
                if (entries[i].name[0] == 0xE5) continue;
                if (entries[i].attr == ATTR_LFN) continue;

                uint8_t name_buf[FAT32_NAME_LEN];
                fat32_8dot3_to_name(entries[i].name, entries[i].ext,
                                    name_buf, FAT32_NAME_LEN);

                
                int match = 1;
                for (int j = 0; j < FAT32_NAME_LEN; j++) {
                    if (path[j] != name_buf[j]) { match = 0; break; }
                    if (path[j] == '\0') break;
                }

                if (match) {
                    fat32.files[fd].active        = 1;
                    fat32.files[fd].first_cluster =
                        ((uint32_t)entries[i].fst_clus_hi << 16) |
                         entries[i].fst_clus_lo;
                    fat32.files[fd].cur_cluster   =
                        fat32.files[fd].first_cluster;
                    fat32.files[fd].file_size     = entries[i].file_size;
                    fat32.files[fd].pos           = 0;
                    fat32.files[fd].is_dir        =
                        (entries[i].attr & ATTR_DIRECTORY) ? 1 : 0;
                    return fd;
                }
            }
        }
        cluster = fat32_read_fat_entry(cluster);
    }

not_found:
    kprint("[FAT32] File not found\n");
    return -1;
}


int fat32_read(int fd, uint8_t *buf, uint32_t len) {
    if (fd < 0 || fd >= FAT32_MAX_FILES) return -1;
    if (!fat32.files[fd].active)         return -1;
    if (!buf || len == 0)                return -1;

    fat32_file_t *f   = &fat32.files[fd];
    uint32_t     read = 0;

    while (read < len && f->pos < f->file_size) {
        uint32_t clus_off  = f->pos % (fat32.sec_per_clus * FAT32_SECTOR_SZ);
        uint32_t sec_in_clus = clus_off / FAT32_SECTOR_SZ;
        uint32_t byte_off  = clus_off % FAT32_SECTOR_SZ;

        uint64_t lba = fat32_cluster_to_lba(f->cur_cluster) + sec_in_clus;
        if (fat32_read_sector(lba, fat32_sector_buf) != 0) return -1;

        uint32_t to_copy = FAT32_SECTOR_SZ - byte_off;
        if (to_copy > len - read)         to_copy = len - read;
        if (to_copy > f->file_size - f->pos) to_copy = f->file_size - f->pos;

        __builtin_memcpy(buf + read, fat32_sector_buf + byte_off, to_copy);
        read    += to_copy;
        f->pos  += to_copy;

        
        if ((f->pos % (fat32.sec_per_clus * FAT32_SECTOR_SZ)) == 0) {
            f->cur_cluster = fat32_read_fat_entry(f->cur_cluster);
            if (f->cur_cluster >= FAT32_EOC) break;
        }
    }
    return (int)read;
}


int fat32_close(int fd) {
    if (fd < 0 || fd >= FAT32_MAX_FILES) return -1;
    if (!fat32.files[fd].active)         return -1;
    fat32.files[fd].active = 0;
    return 0;
}
