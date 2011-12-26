PREFIX	=	$(DEVKITARM)/bin/arm-eabi-
CC		=	$(PREFIX)gcc
AS		=	$(PREFIX)as
LD		=	$(PREFIX)gcc
SSTRIP	=	$(DEVKITARM)/bin/arm-eabi-strip

CFLAGS	=	-mbig-endian -fomit-frame-pointer -Os -Wall -I. -mcpu=arm926ej-s -marm
CFLAGS +=	-fno-builtin-memcpy -fno-builtin-memset -fno-builtin-toupper -fno-builtin-memcmp -fno-builtin-malloc -fno-builtin-free

ASFLAGS	=	-mbig-endian -mcpu=arm926ej-s

LDFLAGS	=	-nostartfiles -nodefaultlibs -mbig-endian -Wl,-T,iosmodule.ld,-Map,iosmodule.map -n

LIBS	=	-lgcc

TARGET	=	iosmodule.elf
OBJECTS	=	start.o utils_asm.o HW.o memory.o memory_asm.o common.o ff.o diskio.o alloc.o dip.o DSP.o Patches.o main.o vsprintf.o string.o tiny_ehci_glue.o usb_os.o

.PHONY: FORCE

all: $(TARGET)

$(TARGET) : iosmodule.ld $(OBJECTS)
	@echo  "LD	$@"
	@$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
	@$(SSTRIP) -s $@
	@../FixELF/FixELF.exe ../MIOSv10/0000000c.app iosmodule.elf dimios.elf
	@/c/Users/crediar/Documents/Visual\ Studio\ Codename\ Orcas/Projects/Boot2Me/Release/Boot2Me.exe dimios.elf ../NAND/title/00000001/00000100/content/00000008.app

%.o : %.s
	@echo  "AS	$@"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

%.o : %.S
	@echo  "AS	$@"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

%.o : %.c
	@echo  "CC	$@"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	@echo  "DEP	$@"
	@set -e; $(CC) -M $(CFLAGS) $< \
		| sed 's?\($*\)\.o[ :]*?\1.o $@ : ?g' > $@; \
		[ -s $@ ] || rm -f $@

%.d: %.S
	@echo	"DEP	$@"
	@touch $@

-include $(OBJECTS:.o=.d)

clean:
	-rm -f *.elf *.o *.bin *.d *.map ../NAND/title/00000001/00000100/content/00000008.app
