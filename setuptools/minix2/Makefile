include ../Makefile.inc

TARGET = Intel-2.0.4
M2USR = $(M2ROOT)/usr

setup: $(TARGET).tar.bz2
	tar -xjvf $^
	mkdir -p $(M2USR)
	tar -xzvf $(TARGET)/i86/USR.TAZ -C $(M2USR)
	tar -xzvf $(TARGET)/src/CMD.TAZ -C $(M2USR)
	tar -xzvf $(TARGET)/src/SYS.TAZ -C $(M2USR)
	tar -xzvf $(TARGET)/src/FIX.TAZ -C $(M2USR)

unsetup:
	rm -rf $(M2ROOT)
