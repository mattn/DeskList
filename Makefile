EXEDIR=DeskList
DLLDIR=DeskHook
OUTDIR=Obj
dev=vc

#---- MSVC6 ----
!if "$(dev)" == "vc"
!message Compileing by MSVC
cc=cl
link=link
rc=rc
execflags=-c -Fo$@ -Gz
dllcflags=-c -Fo$@
lflags=-out:DeskList.exe
exelflags=-out:$@
exelibs=shell32.lib user32.lib ws2_32.lib gdi32.lib advapi32.lib comctl32.lib
dlllflags=-out:$@ -dll
dlllibs=shell32.lib advapi32.lib user32.lib gdi32.lib
!endif

#---- BCC32 ----
!if "$(dev)" == "bc"
!message Compileing by Borland
cc=bcc32
link=ilink32
rc=brc32
execflags=-c -tW -nObj -DWIN32 -D_WIN32_WINNT=0x0400
dllcflags=-c -tWD -nObj -DWIN32 -zS -D_BCC_
rflags=-r -w32
exelflags=-Tpe -n -aa -Gn -x
dlllflags=-Tpd -n -ap -Gn -x
exelibs=,$@, , c0w32.obj shell32.lib user32.lib gdi32.lib advapi32.lib import32.lib cw32mt.lib, ,
dlllibs=,$@, , c0d32.obj shell32.lib user32.lib gdi32.lib advapi32.lib import32.lib cw32mt.lib, $(DLLDIR)\DeskHook.def,
!endif

all : DeskList.exe DeskHook.dll

DeskList.exe : $(OUTDIR)\DeskList.obj $(OUTDIR)\DeskLibs.obj $(OUTDIR)\DeskList.res
	$(link) $(exelflags) $(OUTDIR)\DeskList.obj $(OUTDIR)\DeskLibs.obj $(exelibs) $(OUTDIR)\DeskList.res

$(OUTDIR)\DeskList.obj : $(EXEDIR)\DeskList.c $(EXEDIR)\DeskLibs.h $(EXEDIR)\resource.h
	$(cc) $(execflags) $(EXEDIR)\DeskList.c

$(OUTDIR)\DeskLibs.obj : $(EXEDIR)\DeskLibs.c $(EXEDIR)\DeskLibs.h
	$(cc) $(execflags) $(EXEDIR)\DeskLibs.c

$(OUTDIR)\DeskList.res : $(EXEDIR)\DeskList.rc $(EXEDIR)\resource.h
	$(rc) -r -Fo$(OUTDIR)\DeskList.res $(EXEDIR)\DeskList.rc

DeskHook.dll : $(OUTDIR)\DeskHook.obj $(OUTDIR)\DeskHook.res
	$(link) $(dlllflags) $(OUTDIR)\DeskHook.obj $(dlllibs) $(OUTDIR)\DeskHook.res

$(OUTDIR)\DeskHook.obj : $(DLLDIR)\DeskHook.c $(DLLDIR)\DeskHook.h $(DLLDIR)\resource.h
	$(cc) $(dllcflags) $(DLLDIR)\DeskHook.c

$(OUTDIR)\DeskHook.res : $(DLLDIR)\DeskHook.rc $(DLLDIR)\resource.h
	rc -r -Fo$(OUTDIR)\DeskHook.res $(DLLDIR)\DeskHook.rc

clean :
	del /Q Obj\*.*
