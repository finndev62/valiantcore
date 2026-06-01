#include "kernel.h"
#include "network.h"


// ============================================≠=========
//   Developer >  : Bıgpower
//   Maintiner >  : Bıgpower
//       ____  _________  ____  ____ _       _______ ____  _______ _  _______ __ 
//     / __ )/  _/ ____// __ \/ __ \ | / / ____/ __ \/ / / / ____/ __ \/ |/ / ____/ / 
//    / __  |/ // / __ / /_/ / / / / |/ / __/ / /_/ / / / / __/ / /_/ /  / / __/ / /  
//   / /_/ // // /_/ // ____/ /_/ /|  / /___/ _, _/ /_/ / /___/ _, _/ /|  / /___/ /___
//  /_____/___/\____//_/    \____/ | /_____/_/ |_|\____/_____/_/ |_/_/ |_/_____/_____/
//                                 |/
// =======================================================

#define RTL_VENDOR_ID     0x10EC
#define RTL_DEVICE_8111   0x8168
#define RTL_DEVICE_8111B  0x8136
#define RTL_DEVICE_8111C  0x8169

/* ----------------------------------------------
  *       Developer >  : Bıgpower                 *
  *       Maintainer >  : Bıgpower                *
  * ----------------------------------------------*
  *     Producer & Director > : Bıgpower          *
  * ----------------------------------------------*/


#define RTL_MACO        0x00
#define RTL_MARO        0x08
#define RTL_TXSTATUSO   0x10
#define RTL_TXADDR0     0x20
#define RTL_RXBUF       0x30
#define RTL_CMD         0x37
#define RTL_RXBUFPTR    0x38
#define RTL_RXBUFADDR   0x3A
#undef RTL_IMR
#define RTL_IMR         0x3C
#define RTL_ISR         0x3E
#define RTL_TCR         0x40
#define RTL_RCR         0x44
#define RTL_MPC         0x4C
#define RTL_CONFIG1     0x52
#define RTL_MSR         0x58
#define RTL_BMCR        0x62


#define RTL_CMD_RESET    0x10
#define RTL_CMD_RX_EN    0x08
#define RTL_CMD_TX_EN    0x04

#define RTL_ISR_ROK     0x0001
#define RTL_ISR_TOK     0x0004
#define RTL_ISR_CABLE   0x2000


#define RTL_MSR_LINK      0x04
#define RTL_MSR_SPEED100  0x08


#define RTL_RCR_AAP       0x01
#define RTL_RCR_APM       0x02
#define RTL_RCR_AB        0x08
#define RTL_RCR_WRAP      0x80
#define RTL_RCR_RBLEN     0x1800
#define RTL_RCR_MXDMA     0x0700


#define PCI_CONFIG_ADDR  0xCF8
#define PCI_CONFIG_DATA  0xCFC
#define PCI_BARO         0x10
#define PCI_CMD          0x04

#define RX_BUF_SIZE      65536
#define TX_BUF_SIZE      1536

static addr_t    rtl_base      = 0;
static uint8_t   rtl_mac[6]    = { 0 };
static uint8_t   cable_status  = 0;
static uint32_t  rx_packets    = 0;
static uint32_t  rx_errors     = 0;
static uint32_t  tx_packets    = 0;
static uint32_t  tx_error      = 0;
static uint8_t   rx_buf[RX_BUF_SIZE];
static uint8_t   tx_buf[TX_BUF_SIZE];

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = (uint32_t)(
       ((uint32_t)bus << 16)  |
       ((uint32_t)slot << 11) |
       ((uint32_t)func << 8)  |
       (offset & 0xFC)        |
       0x80000000
);
outl(PCI_CONFIG_ADDR, addr);
return inl(PCI_CONFIG_DATA);
}

   static void pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
      uint32_t addr = (uint32_t)(
       ((uint32_t)bus << 16)  |
       ((uint32_t)slot << 11) |
       ((uint32_t)func << 8)  |
       (offset & 0xFC)        |
       0x80000000
);
outl(PCI_CONFIG_ADDR, addr);
outl(PCI_CONFIG_DATA, val);
}

static uint16_t pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint16_t)(pci_read32(bus, slot, func, offset) >> ((offset & 2) * 8));
}

/****************************************************
* ================================================================================
PROJECT: VALIANTCORE
AUTHOR: bigpower
COPYRIGHT (C) 2026 bigpower

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

--------------------------------------------------------------------------------
ADDITIONAL TERMS & OBLIGATIONS:

As the original author, bigpower, I explicitly mandate that any derivative work, 
modification, or distribution of VALIANTCORE must strictly adhere to the 
Copyleft provisions of the GNU General Public License v3. 

1. OPEN SOURCE MANDATE: You are encouraged to study, share, and improve 
   VALIANTCORE. However, any modification or derivative project created 
   using this source code MUST be released under the same or a compatible 
   open-source license.

2. ATTRIBUTION: Any public distribution or derivative work must retain 
   the original copyright notice (Copyright (C) 2026 bigpower) and acknowledge 
   the original authorship of this project.

3. FREEDOM OF SOFTWARE: This project is built on the philosophy of software 
   freedom. By using or modifying VALIANTCORE, you agree to respect these 
   principles, ensuring that the code remains accessible and transparent 
   to the entire developer community.
================================================================================

                      GNU GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.

                            Preamble

  The GNU General Public License is a free, copyleft license for
software and other kinds of works.

  The licenses for most software and other practical works are designed
to take away your freedom to share and change the works.  By contrast,
the GNU General Public License is intended to guarantee your freedom to
share and change all versions of a program--to make sure it remains free
software for all its users.  We, the Free Software Foundation, use the
GNU General Public License for most of our software; it applies also to
any other work released this way by its authors.  You can apply it to
your programs, too.

  When we speak of free software, we are referring to freedom, not
price.  Our General Public Licenses are designed to make sure that you
have the freedom to distribute copies of free software (and charge for
them if you wish), that you receive source code or can get it if you
want it, that you can change the software or use pieces of it in new
free programs, and that you know you can do these things.

  To protect your rights, we need to prevent others from denying you
these rights or asking you to surrender the rights.  Therefore, you have
certain responsibilities if you distribute copies of the software, or if
you modify it: responsibilities to respect the freedom of others.

  For example, if you distribute copies of such a program, whether
gratis or for a fee, you must pass on to the recipients the same
freedoms that you received.  You must make sure that they, too, receive
or can get the source code.  And you must show them these terms so they
know their rights.

  Developers that use the GNU GPL protect your rights with two steps:
(1) assert copyright on the software, and (2) offer you this License
giving you legal permission to copy, distribute and/or modify it.

  For the developers' and authors' protection, the GPL clearly explains
that there is no warranty for this free software.  For both users' and
authors' sake, the GPL requires that modified versions be marked as
changed, so that their problems will not be attributed erroneously to
authors of previous versions.

  Some devices are designed to deny users access to install or run
modified versions of the software inside them, although the manufacturer
can do so.  This is fundamentally incompatible with the aim of
protecting users' freedom to change the software.  The systematic
pattern of such abuse occurs in the area of products for individuals to
use, which is precisely where it is most unacceptable.  Therefore, we
have designed this version of the GPL to prohibit the practice for those
products.  If such problems arise substantially in other domains, we
stand ready to extend this provision to those domains in future versions
of the GPL, as needed to protect the freedom of users.

  Finally, every program is threatened constantly by software patents.
States should not allow patents to restrict development and use of
software on general-purpose computers, but in those that do, we wish to
avoid the special danger that patents applied to a free program could
make it effectively proprietary.  To prevent this, the GPL assures that
patents cannot be used to render the program non-free.

  The precise terms and conditions for copying, distribution and
modification follow.

                       TERMS AND CONDITIONS

  0. Definitions.

  "This License" refers to version 3 of the GNU General Public License.

  "Copyright" also means copyright-like laws that apply to other kinds of
works, such as semiconductor masks.

  "The Program" refers to any copyrightable work licensed under this
License.  Each licensee is addressed as "you".  "Licensees" and
"recipients" may be individuals or organizations.

  To "modify" a work means to copy from or adapt all or part of the work
in a fashion requiring copyright permission, other than the making of an
exact copy.  The resulting work is called a "modified version" of the
earlier work or a work "based on" the earlier work.

  A "covered work" means either the unmodified Program or a work based
on the Program.

  To "propagate" a work means to do anything with it that, without
permission, would make you directly or secondarily liable for
infringement under applicable copyright law, except executing it on a
computer or modifying a private copy.  Propagation includes copying,
distribution (with or without modification), making available to the
public, and in some countries other activities as well.

  To "convey" a work means any kind of propagation that enables other
parties to make or receive copies.  Mere interaction with a user through
a computer network, with no transfer of a copy, is not conveying.

  An interactive user interface displays "Appropriate Legal Notices"
to the extent that it includes a convenient and prominently visible
feature that (1) displays an appropriate copyright notice, and (2)
tells the user that there is no warranty for the work (except to the
extent that warranties are provided), that licensees may convey the
work under this License, and how to view a copy of this License.  If
the interface presents a list of user commands or options, such as a
menu, a prominent item in the list meets this criterion.

  1. Source Code.

  The "source code" for a work means the preferred form of the work
for making modifications to it.  "Object code" means any non-source
form of a work.

  A "Standard Interface" means an interface that either is an official
standard defined by a recognized standards body, or, in the case of
interfaces specified for a particular programming language, one that
is widely used among developers working in that language.

  The "System Libraries" of an executable work include anything, other
than the work as a whole, that (a) is included in the normal form of
packaging a Major Component, but which is not part of that Major
Component, and (b) serves only to enable use of the work with that
Major Component, or to implement a Standard Interface for which an
implementation is available to the public in source code form.  A
"Major Component", in this context, means a major essential component
(kernel, window system, and so on) of the specific operating system
(if any) on which the executable work runs, or a compiler used to
produce the work, or an object code interpreter used to run it.

  The "Corresponding Source" for a work in object code form means all
the source code needed to generate, install, and (for an executable
work) run the object code and to modify the work, including scripts to
control those activities.  However, it does not include the work's
System Libraries, or general-purpose tools or generally available free
programs which are used unmodified in performing those activities but
which are not part of the work.  For example, Corresponding Source
includes interface definition files associated with source files for
the work, and the source code for shared libraries and dynamically
linked subprograms that the work is specifically designed to require,
such as by intimate data communication or control flow between those
subprograms and other parts of the work.

  The Corresponding Source need not include anything that users
can regenerate automatically from other parts of the Corresponding
Source.

  The Corresponding Source for a work in source code form is that
same work.

  2. Basic Permissions.

  All rights granted under this License are granted for the term of
copyright on the Program, and are irrevocable provided the stated
conditions are met.  This License explicitly affirms your unlimited
permission to run the unmodified Program.  The output from running a
covered work is covered by this License only if the output, given its
content, constitutes a covered work.  This License acknowledges your
rights of fair use or other equivalent, as provided by copyright law.

  You may make, run and propagate covered works that you do not
convey, without conditions so long as your license otherwise remains
in force.  You may convey covered works to others for the sole purpose
of having them make modifications exclusively for you, or provide you
with facilities for running those works, provided that you comply with
the terms of this License in conveying all material for which you do
not control copyright.  Those thus making or running the covered works
for you must do so exclusively on your behalf, under your direction
and control, on terms that prohibit them from making any copies of
your copyrighted material outside their relationship with you.

  Conveying under any other circumstances is permitted solely under
the conditions stated below.  Sublicensing is not allowed; section 10
makes it unnecessary.

  3. Protecting Users' Legal Rights From Anti-Circumvention Law.

  No covered work shall be deemed part of an effective technological
measure under any applicable law fulfilling obligations under article
11 of the WIPO copyright treaty adopted on 20 December 1996, or
similar laws prohibiting or restricting circumvention of such
measures.

  When you convey a covered work, you waive any legal power to forbid
circumvention of technological measures to the extent such circumvention
is effected by exercising rights under this License with respect to
the covered work, and you disclaim any intention to limit operation or
modification of the work as a means of enforcing, against the work's
users, your or third parties' legal rights to forbid circumvention of
technological measures.

  4. Conveying Verbatim Copies.

  You may convey verbatim copies of the Program's source code as you
receive it, in any medium, provided that you conspicuously and
appropriately publish on each copy an appropriate copyright notice;
keep intact all notices stating that this License and any
non-permissive terms added in accord with section 7 apply to the code;
keep intact all notices of the absence of any warranty; and give all
recipients a copy of this License along with the Program.

  You may charge any price or no price for each copy that you convey,
and you may offer support or warranty protection for a fee.

  5. Conveying Modified Source Versions.

  You may convey a work based on the Program, or the modifications to
produce it from the Program, in the form of source code under the
terms of section 4, provided that you also meet all of these conditions:

    a) The work must carry prominent notices stating that you modified
    it, and giving a relevant date.

    b) The work must carry prominent notices stating that it is
    released under this License and any conditions added under section
    7.  This requirement modifies the requirement in section 4 to
    "keep intact all notices".

    c) You must license the entire work, as a whole, under this
    License to anyone who comes into possession of a copy.  This
    License will therefore apply, along with any applicable section 7
    additional terms, to the whole of the work, and all its parts,
    regardless of how they are packaged.  This License gives no
    permission to license the work in any other way, but it does not
    invalidate such permission if you have separately received it.

    d) If the work has interactive user interfaces, each must display
    Appropriate Legal Notices; however, if the Program has interactive
    interfaces that do not display Appropriate Legal Notices, your
    work need not make them do so.

  A compilation of a covered work with other separate and independent
works, which are not by their nature extensions of the covered work,
and which are not combined with it such as to form a larger program,
in or on a volume of a storage or distribution medium, is called an
"aggregate" if the compilation and its resulting copyright are not
used to limit the access or legal rights of the compilation's users
beyond what the individual works permit.  Inclusion of a covered work
in an aggregate does not cause this License to apply to the other
parts of the aggregate.

  6. Conveying Non-Source Forms.

  You may convey a covered work in object code form under the terms
of sections 4 and 5, provided that you also convey the
machine-readable Corresponding Source under the terms of this License,
in one of these ways:

    a) Convey the object code in, or embodied in, a physical product
    (including a physical distribution medium), accompanied by the
    Corresponding Source fixed on a durable physical medium
    customarily used for software interchange.

    b) Convey the object code in, or embodied in, a physical product
    (including a physical distribution medium), accompanied by a
    written offer, valid for at least three years and valid for as
    long as you offer spare parts or customer support for that product
    model, to give anyone who possesses the object code either (1) a
    copy of the Corresponding Source for all the software in the
    product that is covered by this License, on a durable physical
    medium customarily used for software interchange, for a price no
    more than your reasonable cost of physically performing this
    conveying of source, or (2) access to copy the
    Corresponding Source from a network server at no charge.

    c) Convey individual copies of the object code with a copy of the
    written offer to provide the Corresponding Source.  This
    alternative is allowed only occasionally and noncommercially, and
    only if you received the object code with such an offer, in accord
    with subsection 6b.

    d) Convey the object code by offering access from a designated
    place (gratis or for a charge), and offer equivalent access to the
    Corresponding Source in the same way through the same place at no
    further charge.  You need not require recipients to copy the
    Corresponding Source along with the object code.  If the place to
    copy the object code is a network server, the Corresponding Source
    may be on a different server (operated by you or a third party)
    that supports equivalent copying facilities, provided you maintain
    clear directions next to the object code saying where to find the
    Corresponding Source.  Regardless of what server hosts the
    Corresponding Source, you remain obligated to ensure that it is
    available for as long as needed to satisfy these requirements.

    e) Convey the object code using peer-to-peer transmission, provided
    you inform other peers where the object code and Corresponding
    Source of the work are being offered to the general public at no
    charge under subsection 6d.

  A separable portion of the object code, whose source code is excluded
from the Corresponding Source as a System Library, need not be
included in conveying the object code work.

  A "User Product" is either (1) a "consumer product", which means any
tangible personal property which is normally used for personal, family,
or household purposes, or (2) anything designed or sold for incorporation
into a dwelling.  In determining whether a product is a consumer product,
doubtful cases shall be resolved in favor of coverage.  For a particular
product received by a particular user, "normally used" refers to a
typical or common use of that class of product, regardless of the status
of the particular user or of the way in which the particular user
actually uses, or expects or is expected to use, the product.  A product
is a consumer product regardless of whether the product has substantial
commercial, industrial or non-consumer uses, unless such uses represent
the only significant mode of use of the product.

  "Installation Information" for a User Product means any methods,
procedures, authorization keys, or other information required to install
and execute modified versions of a covered work in that User Product from
a modified version of its Corresponding Source.  The information must
suffice to ensure that the continued functioning of the modified object
code is in no case prevented or interfered with solely because
modification has been made.

  If you convey an object code work under this section in, or with, or
specifically for use in, a User Product, and the conveying occurs as
part of a transaction in which the right of possession and use of the
User Product is transferred to the recipient in perpetuity or for a
fixed term (regardless of how the transaction is characterized), the
Corresponding Source conveyed under this section must be accompanied
by the Installation Information.  But this requirement does not apply
if neither you nor any third party retains the ability to install
modified object code on the User Product (for example, the work has
been installed in ROM).

  The requirement to provide Installation Information does not include a
requirement to continue to provide support service, warranty, or updates
for a work that has been modified or installed by the recipient, or for
the User Product in which it has been modified or installed.  Access to a
network may be denied when the modification itself materially and
adversely affects the operation of the network or violates the rules and
protocols for communication across the network.

  Corresponding Source conveyed, and Installation Information provided,
in accord with this section must be in a format that is publicly
documented (and with an implementation available to the public in
source code form), and must require no special password or key for
unpacking, reading or copying.

  7. Additional Terms.

  "Additional permissions" are terms that supplement the terms of this
License by making exceptions from one or more of its conditions.
Additional permissions that are applicable to the entire Program shall
be treated as though they were included in this License, to the extent
that they are valid under applicable law.  If additional permissions
apply only to part of the Program, that part may be used separately
under those permissions, but the entire Program remains governed by
this License without regard to the additional permissions.

  When you convey a copy of a covered work, you may at your option
remove any additional permissions from that copy, or from any part of
it.  (Additional permissions may be written to require their own
removal in certain cases when you modify the work.)  You may place
additional permissions on material, added by you to a covered work,
for which you have or can give appropriate copyright permission.

  Notwithstanding any other provision of this License, for material you
add to a covered work, you may (if authorized by the copyright holders of
that material) supplement the terms of this License with terms:

    a) Disclaiming warranty or limiting liability differently from the
    terms of sections 15 and 16 of this License; or

    b) Requiring preservation of specified reasonable legal notices or
    author attributions in that material or in the Appropriate Legal
    Notices displayed by works containing it; or
    c) Prohibiting misrepresentation of the origin of that material, or
    requiring that modified versions of such material be marked in
    reasonable ways as different from the original version; or

    d) Limiting the use for publicity purposes of names of licensors or
    authors of the material; or

    e) Declining to grant rights under trademark law for use of some
    trade names, trademarks, or service marks; or

    f) Requiring indemnification of licensors and authors of that
    material by anyone who conveys the material (or modified versions of
    it) with contractual assumptions of liability to the recipient, for
    any liability that these contractual assumptions directly impose on
    */


static addr_t rtl_find_device() {
   for (uint16_t bus = 0; bus < 256; bus++) {
       for (uint8_t slot = 0; slot < 32; slot++) {
           uint32_t id = pci_read32(bus, slot, 0, 0);
           uint16_t vendor = id & 0xFFFF;
           uint16_t device = (id >> 16) & 0xFFFF;

           if (vendor == RTL_VENDOR_ID) {
              if (device == RTL_DEVICE_8111  ||
                  device == RTL_DEVICE_8111B ||
                  device == RTL_DEVICE_8111C) {


                  uint32_t cmd = pci_read32(bus, slot, 0, PCI_CMD);
                  pci_write32(bus, slot, 0, PCI_CMD, cmd | 0x07);

                  addr_t base = (addr_t)(pci_read32(bus, slot, 0, PCI_BARO) & ~0xF);
                  return base;
               }
            }
         }
    }
   return 0;
}

static inline uint8_t rtl_read8(uint8_t reg) {
    return inb((uint16_t)rtl_base + reg);
}

static inline uint16_t rtl_read16(uint8_t reg) {
   return inw((uint16_t)(rtl_base + reg));
}

static inline uint32_t rtl_read32(uint8_t reg) {
   return inl((uint16_t)(rtl_base + reg));
}

static inline void rtl_write8(uint8_t reg, uint8_t val) {
    outb((uint16_t)(rtl_base + reg), val);
}

static inline void rtl_write16(uint8_t reg, uint16_t val) {
    outw((uint16_t)(rtl_base + reg), val);
}

static inline void rtl_write32(uint8_t reg, uint32_t val) {
    outl((uint16_t)(rtl_base + reg), val);
}

static void rtl_io_wait() {
    for (volatile int i = 0; i < 1000; i++);
}


static void rtl_read_mac() {
   for (int i = 0; i < 6; i++)
   rtl_mac[i] = rtl_read8(RTL_MACO + i);
}


static uint8_t rtl_check_cable() {
   uint8_t msr = rtl_read8(RTL_MSR);
   return (msr & RTL_MSR_LINK) ? 1 : 0;
}


int rtl8111_init() {
   kprint("RTL8111 Scanning PCI bus...\n");

   rtl_base = rtl_find_device();
   if (!rtl_base) {
      kprint("RTL8111 ERROR Device not found!\n");
      return -1;
    }

    kprint("RTL8111 Device found! Initializing...\n");


    rtl_write8(RTL_CMD, RTL_CMD_RESET);
    int timeout = 10000;
    while ((rtl_read8(RTL_CMD) & RTL_CMD_RESET) && timeout--);
    if (timeout <= 0) {
       kprint("ERROR Reset timeout!\n");
       return -1;
    }


    rtl_read_mac();


    rtl_write32(RTL_RXBUF, (uint32_t)(addr_t)rx_buf);


    rtl_write16(RTL_IMR, RTL_ISR_ROK | RTL_ISR_TOK | RTL_ISR_CABLE);


    rtl_write32(RTL_RCR,
      RTL_RCR_APM    |
      RTL_RCR_AB     |
      RTL_RCR_WRAP   |
      RTL_RCR_RBLEN  |
      RTL_RCR_MXDMA
    );


    rtl_write32(RTL_TCR, 0x03000700);


    rtl_write8(RTL_CMD, RTL_CMD_RX_EN | RTL_CMD_TX_EN);

    rtl_write8(RTL_CONFIG1, 0x00);


    cable_status = rtl_check_cable();
    if (cable_status) {
       kprint("RTL8111 Ethernet Cable connected!\n");
   } else {
       kprint("RTL8111 WARNING: Cable disconnected!\n");
    }


    kprint("RTL8111 Driver Ready.\n");
    return 0;
 }



 int rtl8111_send(const uint8_t *data, uint16_t len) {
     if (!rtl_base) {
        kprint("RTL8111 ERROR: Driver not initialized!\n");
        return -1;
     }
     if (!cable_status) {
        kprint("RTL8111 ERROR: Cable disconnected!\n");
        return -1;
     }
     if (len > TX_BUF_SIZE) {
        kprint("RTL8111 ERROR: Packet too large!\n");
        return -1;
     }



    for (uint16_t i = 0; i < len; i++)
        tx_buf[i] = data[i];

    rtl_write32(RTL_TXADDR0, (uint32_t)(addr_t)tx_buf);
    rtl_write32(RTL_TXSTATUSO, len);

            
    int timeout = 100000;
    while (!(rtl_read32(RTL_TXSTATUSO) & 0x8000) && timeout--);
    if (timeout <= 0) {
        kprint("[RTL8111] ERROR: Transmit timeout!\n");
        return -1;
    }

    tx_packets++;
    return 0;
}


int rtl8111_receive(uint16_t *buf, uint16_t *len) {
    if (!rtl_base) return -1;


    uint16_t isr = rtl_read16(RTL_ISR);
    if (!(isr & RTL_ISR_ROK)) return 0;

    rtl_write16(RTL_ISR, RTL_ISR_ROK);


    uint16_t rx_ptr = rtl_read16(RTL_RXBUFPTR);
    uint32_t pkt_len = *((uint16_t*)(rx_buf + rx_ptr + 2));

    if (pkt_len > RX_BUF_SIZE) {
       rx_errors++;
       kprint("RTL8111 ERROR: Oversized packet!\n");
       return -1;
    }

    for (uint16_t i = 0; i < pkt_len; i++)
        buf[i] = rx_buf[rx_ptr + 4 + i];
    *len = pkt_len;
    rx_packets++;
    return 1;
}

void rtl8111_irq_handler() {
    if (!rtl_base) return;


    uint16_t isr = rtl_read16(RTL_ISR);
    rtl_write16(RTL_ISR, isr);

    if (isr & RTL_ISR_CABLE) {
        cable_status = rtl_check_cable();
        if (cable_status)
          kprint("[RTL8111] Ethernet cable connected!\n");
        else
          kprint("RTL8111 WARNING: Cable disconnected!\n");
      }

      if (isr & RTL_ISR_ROK)
         kprint("RTL8111 Packet received!\n");
      if (isr & RTL_ISR_TOK)
         tx_packets++;
}

void rtl8111_stats() {
    kprint("[RTL8111] --- Driver Stats ---\n");
    kprint("[RTL8111] RX packets OK\n");
    kprint("[RTL8111] TX packets OK\n");
}

uint8_t rtl8111_link_status() { return cable_status; }
uint32_t rtl8111_rx_count()   { return rx_packets; }
uint32_t rtl8111_tx_count()   { return tx_packets; }
