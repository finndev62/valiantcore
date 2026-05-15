#![no_std]
#![allow(dead_code)]
use::core::panic::PanicInfo;

#[cfg(target_arch = "x86_64")]
type AddrT = u64;

#[cfg(target_arch = "x86")]
type AddrT = u32;

#[repr(C, packed)]
pub struct EthPacket {
   pub dest_mac:     [u8; 6],
   pub source_mac:   [u8; 6],
   pub ethertype:    u16,
   pub data:         [u8; 1500],
   pub length:       u32,
}

#[no_mangle]
pub extern "C" fn rust_validate_packet(packet: *mut EthPacket) -> i8 {
     let p = unsafe { &*packet };


     if p.length == 0 || p.length > 1500 {
         return -1;
}

let broadcast = [0xFF_u8; 6];
let all_zero =  [0x00_u8; 6];

if p.dest_mac == all_zero {
     return -1;
}

if p.source_mac == broadcast {
    return -1;
 }

 0
}

#[no_mangle]
pub extern "C" fn rust_encrypt_payload(packet: *mut EthPacket) {
    let p = unsafe { &mut *packet };
    let key: u8 = 0xAB;

    for i in 0..p.length as usize {
      p.data [i] ^=  key;
    }
}

#[no_mangle]
pub extern "C" fn rust_mac_match(a: *const u8, b: *const u8) -> i8 {
   for i in 0..6 {
      if unsafe { *a.add(i) != *b.add(i) } {
         return 0;
      }
    }
    1
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
   loop{}
}
