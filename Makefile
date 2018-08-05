# Visual C++ Makefile for PuTTY.
#
# Use `nmake' to build.
#

##-- help
#
# Extra options you can set:
#
#  - FWHACK=/DFWHACK
#      Enables a hack that tunnels through some firewall proxies.
#
#  - VER=/DSNAPSHOT=1999-01-25
#      Generates executables whose About box report them as being a
#      development snapshot.
#
#  - VER=/DRELEASE=0.43
#      Generates executables whose About box report them as being a
#      release version.
#
#  - COMPAT=/DAUTO_WINSOCK
#      Causes PuTTY to assume that <windows.h> includes its own WinSock
#      header file, so that it won't try to include <winsock.h>.
#
#  - COMPAT=/DWINSOCK_TWO
#      Causes the PuTTY utilities to include <winsock2.h> instead of
#      <winsock.h>, except Plink which _needs_ WinSock 2 so it already
#      does this.
#
#  - RCFL=/DASCIICTLS
#      Uses ASCII rather than Unicode to specify the tab control in
#      the resource file. Probably most useful when compiling with
#      Cygnus/mingw32, whose resource compiler may have less of a
#      problem with it.
#
#  - XFLAGS=/DMALLOC_LOG
#      Causes PuTTY to emit a file called putty_mem.log, logging every
#      memory allocation and free, so you can track memory leaks.
#
#  - XFLAGS=/DMINEFIELD
#      Causes PuTTY to use a custom memory allocator, similar in
#      concept to Electric Fence, in place of regular malloc(). Wastes
#      huge amounts of RAM, but should cause heap-corruption bugs to
#      show up as GPFs at the point of failure rather than appearing
#      later on as second-level damage.
#
##--

CFLAGS = /nologo /W3 /YX /O2 /Yd /D_WINDOWS /DDEBUG /ML /Fd
# LFLAGS = /debug

# Use MSVC DLL
# CFLAGS = /nologo /W3 /YX /O2 /Yd /D_WINDOWS /DDEBUG /MD /Fd

# Disable debug and incremental linking
LFLAGS = /incremental:no

.c.obj:
	cl $(COMPAT) $(FWHACK) $(XFLAGS) $(CFLAGS) /c $*.c

OBJ=obj
RES=res

##-- objects putty puttytel
GOBJS1 = window.$(OBJ) windlg.$(OBJ) winctrls.$(OBJ) terminal.$(OBJ)
GOBJS2 = xlat.$(OBJ) sizetip.$(OBJ)
##-- objects puttytel
TOBJS = be_nossh.$(OBJ)
##-- resources putty puttytel
PRESRC = win_res.$(RES)
##-- resources pageant
PAGERC = pageant.$(RES)
##-- resources puttygen
GENRC = puttygen.$(RES)
##-- resources pscp
SRESRC = scp.$(RES)
##-- resources plink
LRESRC = plink.$(RES)
##-- objects putty puttytel pscp plink
MOBJS = misc.$(OBJ) version.$(OBJ) winstore.$(OBJ) settings.$(OBJ)
MOBJ2 = tree234.$(OBJ)
##-- gui-apps
# putty
# puttytel
# pageant
# puttygen
##-- console-apps
# pscp
# plink ws2_32
##--

LIBS1 = advapi32.lib user32.lib gdi32.lib
LIBS2 = comctl32.lib comdlg32.lib
LIBS3 = shell32.lib
SOCK1 = wsock32.lib
SOCK2 = ws2_32.lib

all: putty.exe puttytel.exe pscp.exe plink.exe pageant.exe puttygen.exe

puttytel.exe: $(GOBJS1) $(GOBJS2) $(LOBJS1) $(TOBJS) $(MOBJS) $(MOBJ2) $(PRESRC) puttytel.rsp
	link $(LFLAGS) -out:puttytel.exe -map:puttytel.map @puttytel.rsp

puttytel.rsp: makefile
	echo /nologo /subsystem:windows > puttytel.rsp
	echo $(GOBJS1) >> puttytel.rsp
	echo $(GOBJS2) >> puttytel.rsp
	echo $(LOBJS1) >> puttytel.rsp
	echo $(TOBJS) >> puttytel.rsp
	echo $(MOBJS) >> puttytel.rsp
	echo $(MOBJ2) >> puttytel.rsp
	echo $(PRESRC) >> puttytel.rsp
	echo $(LIBS1) >> puttytel.rsp
	echo $(LIBS2) >> puttytel.rsp
	echo $(SOCK2) >> puttytel.rsp


##-- dependencies
window.$(OBJ): window.c putty.h puttymem.h network.h win_res.h storage.h winstuff.h
windlg.$(OBJ): windlg.c putty.h puttymem.h network.h ssh.h win_res.h winstuff.h
winctrls.$(OBJ): winctrls.c winstuff.h winstuff.h
be_nossh.$(OBJ): be_nossh.c
version.$(OBJ): version.c
settings.$(OBJ): settings.c putty.h puttymem.h network.h storage.h
winstore.$(OBJ): winstore.c putty.h puttymem.h network.h storage.h
terminal.$(OBJ): terminal.c putty.h puttymem.h network.h
sizetip.$(OBJ): sizetip.c putty.h puttymem.h network.h winstuff.h
telnet.$(OBJ): telnet.c putty.h puttymem.h network.h
xlat.$(OBJ): xlat.c putty.h puttymem.h network.h
raw.$(OBJ): raw.c putty.h puttymem.h network.h
ldisc.$(OBJ): ldisc.c putty.h puttymem.h network.h
misc.$(OBJ): misc.c putty.h puttymem.h network.h
tree234.$(OBJ): tree234.c tree234.h puttymem.h

##--

# Hack to force version.obj to be rebuilt always
version.obj: versionpseudotarget
	@echo (built version.obj)
versionpseudotarget:
	cl $(FWHACK) $(VER) $(CFLAGS) /c version.c

##-- dependencies
win_res.$(RES): win_res.rc win_res.h putty.ico
##--
win_res.$(RES):
	rc $(FWHACK) $(RCFL) -r -DWIN32 -D_WIN32 -DWINVER=0x0400 win_res.rc

clean: tidy
	del *.exe

tidy:
	del *.obj
	del *.res
	del *.pch
	del *.aps
	del *.ilk
	del *.pdb
	del *.rsp
	del *.dsp
	del *.dsw
	del *.ncb
	del *.opt
	del *.plg
	del *.map
