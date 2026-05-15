#![no_std]
#![allow(dead_code)]
#![allow(static_mut_refs)]

use core::panic::PanicInfo;

/* ── Hibrit tip ─────────────────────────────────────────────── */
#[cfg(target_arch = "x86_64")]
type AddrT = u64;
#[cfg(not(target_arch = "x86_64"))]
type AddrT = u32;

extern "C" {
    fn ata_write_250gb(lba: u64, count: u16, buffer: *const u16) -> i32;
    fn ata_read_250gb(lba: u64, count: u16, buffer: *mut u16) -> i32;
}

/* ── Güvenlik sabitleri ─────────────────────────────────────── */
const DISK_LIMIT_LBA:    u64 = 488_397_168;
const SECTOR_SIZE:       usize = 512;
const MAX_WRITE_SECTORS: u16 = 128;       /* tek seferde max 64KB */
const MAGIC_CANARY:      u32 = 0xDEADC0DE; /* bellek bütünlük testi */
const XOR_KEY:           u8  = 0xA5;       /* basit veri doğrulama */

/* ── Hata tipleri ───────────────────────────────────────────── */
#[derive(Debug)]
pub enum DiskError {
    LbaOutOfBounds,
    EmptyData,
    TooManySectors,
    HardwareError,
    BufferOverflow,
    IntegrityCheckFailed,
    NullPointer,
    UnalignedAccess,
}

/* ── Güvenli Disk Yöneticisi ────────────────────────────────── */
pub struct SafeDiskManager {
    sector_size:    usize,
    disk_limit_lba: u64,
    canary:         u32,
    write_count:    u64,
    error_count:    u32,
}

impl SafeDiskManager {
    pub const fn new() -> Self {
        Self {
            sector_size:    SECTOR_SIZE,
            disk_limit_lba: DISK_LIMIT_LBA,
            canary:         MAGIC_CANARY,
            write_count:    0,
            error_count:    0,
        }
    }

    /* ── Canary kontrolü (bellek bozulma tespiti) ───────────── */
    fn check_integrity(&self) -> bool {
        self.canary == MAGIC_CANARY
    }

    /* ── LBA sınır kontrolü ─────────────────────────────────── */
    fn validate_lba(&self, lba: u64, count: u16) -> Result<(), DiskError> {
        if lba > self.disk_limit_lba {
            return Err(DiskError::LbaOutOfBounds);
        }
        /* taşma kontrolü */
        if lba.saturating_add(count as u64) > self.disk_limit_lba {
            return Err(DiskError::LbaOutOfBounds);
        }
        Ok(())
    }

    /* ── Veri doğrulama (checksum) ──────────────────────────── */
    fn checksum(data: &[u16]) -> u32 {
        let mut sum: u32 = 0;
        for &word in data {
            sum = sum.wrapping_add(word as u32);
            sum ^= XOR_KEY as u32;
        }
        sum
    }

    /* ── Güvenli yazma ──────────────────────────────────────── */
    pub fn secure_write(&mut self, lba: u64, data: &[u16]) -> Result<u32, DiskError> {
        /* Bellek bütünlük kontrolü */
        if !self.check_integrity() {
            return Err(DiskError::IntegrityCheckFailed);
        }

        /* Boş veri kontrolü */
        if data.is_empty() {
            return Err(DiskError::EmptyData);
        }

        /* Sektör sayısı kontrolü */
        let sector_count = (data.len() / 256) as u16;
        if sector_count > MAX_WRITE_SECTORS {
            return Err(DiskError::TooManySectors);
        }
        if sector_count == 0 {
            return Err(DiskError::EmptyData);
        }

        /* LBA sınır kontrolü */
        self.validate_lba(lba, sector_count)?;

        /* Buffer overflow kontrolü */
        if data.len() > (MAX_WRITE_SECTORS as usize * 256) {
            return Err(DiskError::BufferOverflow);
        }

        /* Checksum hesapla */
        let checksum = Self::checksum(data);

        /* Donanıma yaz */
        let result = unsafe {
            ata_write_250gb(lba, sector_count, data.as_ptr())
        };

        if result == 0 {
            self.write_count = self.write_count.wrapping_add(1);
            Ok(checksum)
        } else {
            self.error_count = self.error_count.wrapping_add(1);
            Err(DiskError::HardwareError)
        }
    }

    /* ── Güvenli okuma ──────────────────────────────────────── */
    pub fn secure_read(&mut self, lba: u64, buffer: &mut [u16]) -> Result<u32, DiskError> {
        if !self.check_integrity() {
            return Err(DiskError::IntegrityCheckFailed);
        }

        if buffer.is_empty() {
            return Err(DiskError::EmptyData);
        }

        let sector_count = (buffer.len() / 256) as u16;
        if sector_count == 0 {
            return Err(DiskError::EmptyData);
        }

        self.validate_lba(lba, sector_count)?;

        let result = unsafe {
            ata_read_250gb(lba, sector_count, buffer.as_mut_ptr())
        };

        if result == 0 {
            let checksum = Self::checksum(buffer);
            Ok(checksum)
        } else {
            self.error_count = self.error_count.wrapping_add(1);
            Err(DiskError::HardwareError)
        }
    }

    /* ── İstatistik ─────────────────────────────────────────── */
    pub fn write_count(&self) -> u64 { self.write_count }
    pub fn error_count(&self) -> u32 { self.error_count }
}

/* ── C'ye açık fonksiyonlar ─────────────────────────────────── */
static mut DISK_MANAGER: SafeDiskManager = SafeDiskManager::new();

#[no_mangle]
pub extern "C" fn safe_disk_write(lba: u64, data: *const u16, len: usize) -> i32 {
    if data.is_null() { return -1; }
    let slice = unsafe { core::slice::from_raw_parts(data, len) };
    let mgr = unsafe { &mut DISK_MANAGER };
    match mgr.secure_write(lba, slice) {
        Ok(_)  => 0,
        Err(_) => -1,
    }
}

#[no_mangle]
pub extern "C" fn safe_disk_read(lba: u64, buffer: *mut u16, len: usize) -> i32 {
    if buffer.is_null() { return -1; }
    let slice = unsafe { core::slice::from_raw_parts_mut(buffer, len) };
    let mgr = unsafe { &mut DISK_MANAGER };
    match mgr.secure_read(lba, slice) {
        Ok(_)  => 0,
        Err(_) => -1,
    }
}

#[panic_handler]
fn panic_handler(_info: &PanicInfo) -> ! {
    loop {}
}
