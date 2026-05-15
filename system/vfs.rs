#![no_std]
#![allow(dead_code)]
#![allow(unused_variables)]
#![allow(static_mut_refs)]

use core::panic::PanicInfo;

#[cfg(target_arch = "x86_64")]
type AddrT = u64;
#[cfg(not(target_arch = "x86_64"))]
type AddrT = u32;

pub const MAX_OPEN_FILES:   usize = 512;
pub const VNODE_TABLE_SIZE: usize = 128;
pub const MOUNT_MAX:        usize = 32;
pub const PATH_MAX:         usize = 4096;
pub const MAX_INODES:       usize = 128;
pub const MAX_FILENAME:     usize = 64;

pub const FSTYPE_NONE:   u32 = 0;
pub const FSTYPE_SZFS:   u32 = 1;
pub const FSTYPE_FAT32:  u32 = 2;
pub const FSTYPE_PROCFS: u32 = 3;

pub const O_RDONLY: u32 = 0o000;
pub const O_WRONLY: u32 = 0o001;
pub const O_RDWR:   u32 = 0o002;
pub const O_CREAT:  u32 = 0o100;
pub const O_EXCL:   u32 = 0o200;
pub const O_TRUNC:  u32 = 0o1000;
pub const O_APPEND: u32 = 0o2000;

pub const S_IRUSR: u16 = 0o0400;
pub const S_IWUSR: u16 = 0o0200;
pub const S_IXUSR: u16 = 0o0100;
pub const S_IRWXU: u16 = 0o0700;

pub const VTYPE_REG: u16 = 1;
pub const VTYPE_DIR: u16 = 2;
pub const VTYPE_LNK: u16 = 3;
pub const VTYPE_DEV: u16 = 4;

pub const ENOENT:  i32 = -2;
pub const EIO:     i32 = -5;
pub const EBADF:   i32 = -9;
pub const EACCES:  i32 = -13;
pub const EEXIST:  i32 = -17;
pub const ENOTDIR: i32 = -20;
pub const EINVAL:  i32 = -22;
pub const EMFILE:  i32 = -24;

extern "C" {
    fn ata_read_250gb(lba: u64, count: u16, buf: *mut u16) -> i32;
    fn ata_write_250gb(lba: u64, count: u16, buf: *const u16) -> i32;
    fn security_scan_buffer(buf: *const u8, len: usize) -> i32;
    fn kprint(msg: *const u8);
}

macro_rules! vfs_print {
    ($msg:expr) => {
        unsafe { kprint(concat!("[VFS] ", $msg, "\0").as_ptr()); }
    };
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct Inode {
    pub id:          u32,
    pub is_active:   bool,
    pub is_dir:      bool,
    pub size_bytes:  u64,
    pub start_block: u64,
    pub permissions: u16,
    pub uid:         u16,
    pub name:        [u8; MAX_FILENAME],
}

impl Inode {
    pub const fn new() -> Self {
        Self {
            id:          0,
            is_active:   false,
            is_dir:      false,
            size_bytes:  0,
            start_block: 0,
            permissions: 0o644,
            uid:         0,
            name:        [0u8; MAX_FILENAME],
        }
    }
}
#[derive(Copy, Clone)]
#[repr(C)]
pub struct VNode {
    pub v_id:    u64,
    pub v_type:  u16,
    pub v_count: i32,
    pub v_data:  usize,
    pub is_locked: bool,
}

impl VNode {
    pub const fn new() -> Self {
        Self {
            v_id:      0,
            v_type:    0,
            v_count:   0,
            v_data:    0,
            is_locked: false,
        }
    }
}

pub struct Mount {
    pub dev_id:  u32,
    pub fs_type: u32,
    pub active:  bool,
}

#[derive(Copy, Clone)]
pub struct File {
    pub f_inode_idx: usize,
    pub f_pos:       u64,
    pub f_flags:     u32,
    pub f_count:     i32,
    pub f_uid:       u16,
    pub f_active:    bool,
}

impl File {
    pub const fn new() -> Self {
        Self {
            f_inode_idx: 0,
            f_pos:       0,
            f_flags:     0,
            f_count:     0,
            f_uid:       0,
            f_active:    false,
        }
    }
}

pub struct FileSystem {
    pub inodes:      [Inode; MAX_INODES],
    pub inode_count: usize,
    pub fs_type:     u32,
}

impl FileSystem {
    pub const fn new() -> Self {
        Self {
            inodes:      [Inode::new(); MAX_INODES],
            inode_count: 0,
            fs_type:     FSTYPE_SZFS,
        }
    }

    pub fn find_file(&self, path: &str) -> Option<usize> {
        let path_bytes = path.as_bytes();
        for i in 0..self.inode_count {
            if !self.inodes[i].is_active { continue; }
            let name = &self.inodes[i].name;
            let mut matched = true;
            for j in 0..path_bytes.len().min(MAX_FILENAME) {
                if name[j] != path_bytes[j] { matched = false; break; }
            }
            if matched { return Some(i); }
        }
        None
    }

    pub fn create_entry(&mut self, path: &str, is_dir: bool) -> i32 {
        if self.inode_count >= MAX_INODES { return -1; }
        let idx = self.inode_count;
        self.inodes[idx].id        = idx as u32;
        self.inodes[idx].is_active = true;
        self.inodes[idx].is_dir    = is_dir;
        self.inodes[idx].size_bytes  = 0;
        self.inodes[idx].start_block = 100 + idx as u64;
        self.inodes[idx].permissions = 0o644;

        let path_bytes = path.as_bytes();
        for i in 0..path_bytes.len().min(MAX_FILENAME) {
            self.inodes[idx].name[i] = path_bytes[i];
        }

        self.inode_count += 1;
        idx as i32
    }
}

pub struct VfsManager {
    pub files:       [File; MAX_OPEN_FILES],
    pub vnodes:      [VNode; VNODE_TABLE_SIZE],
    pub fs:          FileSystem,
    pub total_opens: u64,
}

impl VfsManager {
    pub const fn new() -> Self {
        Self {
            files:       [File::new(); MAX_OPEN_FILES],
            vnodes:      [VNode::new(); VNODE_TABLE_SIZE],
            fs:          FileSystem::new(),
            total_opens: 0,
        }
    }

    pub fn init(&mut self) {
        for i in 0..MAX_OPEN_FILES {
            self.files[i].f_active = false;
        }
        /* Create root directory */
        self.fs.create_entry("/", true);
        self.fs.create_entry("/boot", true);
        self.fs.create_entry("/drivers", true);
        self.fs.create_entry("/system", true);
        self.fs.create_entry("/security", true);
        self.fs.create_entry("/logs", true);
        vfs_print!("VFS initialized. CyberArmor filesystem ready.");
    }

    /* ── sys_open ───────────────────────────────────────────── */
    pub fn sys_open(&mut self, path: &str, flags: u32) -> i32 {
        let inode_idx = match self.fs.find_file(path) {
            Some(idx) => {
                if (flags & O_EXCL) != 0 { return EEXIST; }
                idx
            }
            None => {
                if (flags & O_CREAT) != 0 {
                    let r = self.fs.create_entry(path, false);
                    if r < 0 { return EIO; }
                    r as usize
                } else {
                    return ENOENT;
                }
            }
        };

        for fd in 3..MAX_OPEN_FILES {
            if !self.files[fd].f_active {
                self.files[fd].f_active    = true;
                self.files[fd].f_inode_idx = inode_idx;
                self.files[fd].f_flags     = flags;
                self.files[fd].f_count     = 1;
                self.files[fd].f_uid       = 0;
                self.files[fd].f_pos       = if (flags & O_APPEND) != 0 {
                    self.fs.inodes[inode_idx].size_bytes
                } else { 0 };

                self.total_opens += 1;
                return fd as i32;
            }
        }
        EMFILE
    }

    pub fn sys_close(&mut self, fd: usize) -> i32 {
        if fd >= MAX_OPEN_FILES || !self.files[fd].f_active {
            return EBADF;
        }
        self.files[fd].f_active = false;
        self.files[fd].f_count  = 0;
        0
    }

    pub fn sys_read(&mut self, fd: usize, buf: *mut u16, count: u32) -> i32 {
        if fd >= MAX_OPEN_FILES || !self.files[fd].f_active {
            return EBADF;
        }
        let inode = &self.fs.inodes[self.files[fd].f_inode_idx];
        if self.files[fd].f_pos >= inode.size_bytes { return 0; }

        let lba = inode.start_block + (self.files[fd].f_pos / 512);
        let result = unsafe { ata_read_250gb(lba, 1, buf) };
        if result == 0 {
            self.files[fd].f_pos += 512;
            return 512;
        }
        EIO
    }

    pub fn sys_write(&mut self, fd: usize, buf: *const u16, count: u32) -> i32 {
        if fd >= MAX_OPEN_FILES || !self.files[fd].f_active {
            return EBADF;
        }
        if (self.files[fd].f_flags & O_WRONLY) == 0 &&
           (self.files[fd].f_flags & O_RDWR)   == 0 {
            return EACCES;
        }

        /* Security scan before write */
        let scan = unsafe {
            security_scan_buffer(buf as *const u8, count as usize)
        };
        if scan != 0 {
            vfs_print!("BLOCKED: Virus detected in write buffer!");
            return EACCES;
        }

        let inode = &self.fs.inodes[self.files[fd].f_inode_idx];
        let lba = inode.start_block + (self.files[fd].f_pos / 512);
        let result = unsafe { ata_write_250gb(lba, 1, buf) };
        if result == 0 {
            self.files[fd].f_pos += 512;
            return 512;
        }
        EIO
    }

    pub fn sys_lseek(&mut self, fd: usize, offset: i64, whence: i32) -> i64 {
        if fd >= MAX_OPEN_FILES || !self.files[fd].f_active {
            return EBADF as i64;
        }
        match whence {
            0 => self.files[fd].f_pos = offset as u64,
            1 => self.files[fd].f_pos = (self.files[fd].f_pos as i64 + offset) as u64,
            _ => return EINVAL as i64,
        }
        self.files[fd].f_pos as i64
    }

    pub fn sys_mkdir(&mut self, path: &str) -> i32 {
        if self.fs.find_file(path).is_some() { return EEXIST; }
        self.fs.create_entry(path, true)
    }

    pub fn sys_sync(&self) {
        for i in 0..self.fs.inode_count {
            if self.fs.inodes[i].is_active {
                let lba = 20 + i as u64;
                unsafe {
                    ata_write_250gb(
                        lba, 1,
                        &self.fs.inodes[i] as *const _ as *const u16
                    );
                }
            }
        }
        vfs_print!("INFO: Filesystem synced to disk.");
    }
}

static mut VFS: VfsManager = VfsManager::new();

#[no_mangle]
pub extern "C" fn vfs_init() {
    unsafe { VFS.init(); }
}

#[no_mangle]
pub extern "C" fn vfs_open(path: *const u8, flags: u32) -> i32 {
    if path.is_null() { return EINVAL; }
    let s = unsafe {
        let mut len = 0;
        while *path.add(len) != 0 { len += 1; }
        core::str::from_utf8_unchecked(core::slice::from_raw_parts(path, len))
    };
    unsafe { VFS.sys_open(s, flags) }
}

#[no_mangle]
pub extern "C" fn vfs_close(fd: u32) -> i32 {
    unsafe { VFS.sys_close(fd as usize) }
}

#[no_mangle]
pub extern "C" fn vfs_read(fd: u32, buf: *mut u16, count: u32) -> i32 {
    unsafe { VFS.sys_read(fd as usize, buf, count) }
}

#[no_mangle]
pub extern "C" fn vfs_write(fd: u32, buf: *const u16, count: u32) -> i32 {
    unsafe { VFS.sys_write(fd as usize, buf, count) }
}

#[no_mangle]
pub extern "C" fn vfs_mkdir(path: *const u8) -> i32 {
    if path.is_null() { return EINVAL; }
    let s = unsafe {
        let mut len = 0;
        while *path.add(len) != 0 { len += 1; }
        core::str::from_utf8_unchecked(core::slice::from_raw_parts(path, len))
    };
    unsafe { VFS.sys_mkdir(s) }
}

#[no_mangle]
pub extern "C" fn vfs_sync() {
    unsafe { VFS.sys_sync(); }
}

#[panic_handler]
fn panic_handler(_info: &PanicInfo) -> ! {
    unsafe {
        core::arch::asm!("cli");
        loop { core::arch::asm!("hlt"); }
    }
}
