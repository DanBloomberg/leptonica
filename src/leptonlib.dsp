# Microsoft Developer Studio Project File - Name="leptonlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=leptonlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "leptonlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "leptonlib.mak" CFG="leptonlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "leptonlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "leptonlib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "leptonlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "dependencies-lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LEPTONLIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "dependencies-include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LEPTONLIB_EXPORTS" /D "L_LITTLE_ENDIAN" /D "USE_PSTDINT" /D snprintf=_snprintf /D COMPILER_MSVC=1 /D "XMD_H" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dependencies-lib\libtiff.lib dependencies-lib\libpng.lib dependencies-lib\jpeg.lib dependencies-lib\zlib.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "leptonlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LEPTONLIB_EXPORTS" /YX /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "dependencies-include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LEPTONLIB_EXPORTS" /D "L_LITTLE_ENDIAN" /D "USE_PSTDINT" /D snprintf=_snprintf /D COMPILER_MSVC=1 /D "XMD_H" /D "_CRT_SECURE_NO_WARNINGS" /YX /FD /GZ  /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dependencies-lib\libtiff.lib dependencies-lib\libpng.lib dependencies-lib\jpeg.lib dependencies-lib\zlib.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "leptonlib - Win32 Release"
# Name "leptonlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\adaptmap.c
# End Source File
# Begin Source File

SOURCE=.\src\affine.c
# End Source File
# Begin Source File

SOURCE=.\src\affinecompose.c
# End Source File
# Begin Source File

SOURCE=.\src\arithlow.c
# End Source File
# Begin Source File

SOURCE=.\src\arrayaccess.c
# End Source File
# Begin Source File

SOURCE=.\src\bardecode.c
# End Source File
# Begin Source File

SOURCE=.\src\baseline.c
# End Source File
# Begin Source File

SOURCE=.\src\bbuffer.c
# End Source File
# Begin Source File

SOURCE=.\src\bilinear.c
# End Source File
# Begin Source File

SOURCE=.\src\binarize.c
# End Source File
# Begin Source File

SOURCE=.\src\binexpand.c
# End Source File
# Begin Source File

SOURCE=.\src\binexpandlow.c
# End Source File
# Begin Source File

SOURCE=.\src\binreduce.c
# End Source File
# Begin Source File

SOURCE=.\src\binreducelow.c
# End Source File
# Begin Source File

SOURCE=.\src\blend.c
# End Source File
# Begin Source File

SOURCE=.\src\bmf.c
# End Source File
# Begin Source File

SOURCE=.\src\bmpio.c
# End Source File
# Begin Source File

SOURCE=.\src\boxbasic.c
# End Source File
# Begin Source File

SOURCE=.\src\boxfunc1.c
# End Source File
# Begin Source File

SOURCE=.\src\boxfunc2.c
# End Source File
# Begin Source File

SOURCE=.\src\boxfunc3.c
# End Source File
# Begin Source File

SOURCE=.\src\ccbord.c
# End Source File
# Begin Source File

SOURCE=.\src\ccthin.c
# End Source File
# Begin Source File

SOURCE=.\src\classapp.c
# End Source File
# Begin Source File

SOURCE=.\src\colorcontent.c
# End Source File
# Begin Source File

SOURCE=.\src\colormap.c
# End Source File
# Begin Source File

SOURCE=.\src\colormorph.c
# End Source File
# Begin Source File

SOURCE=.\src\colorquant1.c
# End Source File
# Begin Source File

SOURCE=.\src\colorquant2.c
# End Source File
# Begin Source File

SOURCE=.\src\colorseg.c
# End Source File
# Begin Source File

SOURCE=.\src\compare.c
# End Source File
# Begin Source File

SOURCE=.\src\conncomp.c
# End Source File
# Begin Source File

SOURCE=.\src\convolve.c
# End Source File
# Begin Source File

SOURCE=.\src\convolvelow.c
# End Source File
# Begin Source File

SOURCE=.\src\correlscore.c
# End Source File
# Begin Source File

SOURCE=.\src\dwacomb.2.c
# End Source File
# Begin Source File

SOURCE=.\src\dwacomblow.2.c
# End Source File
# Begin Source File

SOURCE=.\src\edge.c
# End Source File
# Begin Source File

SOURCE=.\src\enhance.c
# End Source File
# Begin Source File

SOURCE=.\src\fhmtauto.c
# End Source File
# Begin Source File

SOURCE=.\src\fhmtgen.1.c
# End Source File
# Begin Source File

SOURCE=.\src\fhmtgenlow.1.c
# End Source File
# Begin Source File

SOURCE=.\src\flipdetect.c
# End Source File
# Begin Source File

SOURCE=.\src\fliphmtgen.c
# End Source File
# Begin Source File

SOURCE=.\src\fmorphauto.c
# End Source File
# Begin Source File

SOURCE=.\src\fmorphgen.1.c
# End Source File
# Begin Source File

SOURCE=.\src\fmorphgenlow.1.c
# End Source File
# Begin Source File

SOURCE=.\src\fpix1.c
# End Source File
# Begin Source File

SOURCE=.\src\fpix2.c
# End Source File
# Begin Source File

SOURCE=.\src\gifiostub.c
# End Source File
# Begin Source File

SOURCE=.\src\gplot.c
# End Source File
# Begin Source File

SOURCE=.\src\graphics.c
# End Source File
# Begin Source File

SOURCE=.\src\graymorph.c
# End Source File
# Begin Source File

SOURCE=.\src\graymorphlow.c
# End Source File
# Begin Source File

SOURCE=.\src\grayquant.c
# End Source File
# Begin Source File

SOURCE=.\src\grayquantlow.c
# End Source File
# Begin Source File

SOURCE=.\src\heap.c
# End Source File
# Begin Source File

SOURCE=.\src\jbclass.c
# End Source File
# Begin Source File

SOURCE=.\src\jpegio.c
# End Source File
# Begin Source File

SOURCE=.\src\kernel.c
# End Source File
# Begin Source File

SOURCE=.\src\list.c
# End Source File
# Begin Source File

SOURCE=.\src\maze.c
# End Source File
# Begin Source File

SOURCE=.\src\morph.c
# End Source File
# Begin Source File

SOURCE=.\src\morphapp.c
# End Source File
# Begin Source File

SOURCE=.\src\morphdwa.c
# End Source File
# Begin Source File

SOURCE=.\src\morphseq.c
# End Source File
# Begin Source File

SOURCE=.\src\numabasic.c
# End Source File
# Begin Source File

SOURCE=.\src\numafunc1.c
# End Source File
# Begin Source File

SOURCE=.\src\numafunc2.c
# End Source File
# Begin Source File

SOURCE=.\src\pageseg.c
# End Source File
# Begin Source File

SOURCE=.\src\paintcmap.c
# End Source File
# Begin Source File

SOURCE=.\src\parseprotos.c
# End Source File
# Begin Source File

SOURCE=.\src\partition.c
# End Source File
# Begin Source File

SOURCE=.\src\pix1.c
# End Source File
# Begin Source File

SOURCE=.\src\pix2.c
# End Source File
# Begin Source File

SOURCE=.\src\pix3.c
# End Source File
# Begin Source File

SOURCE=.\src\pix4.c
# End Source File
# Begin Source File

SOURCE=.\src\pixabasic.c
# End Source File
# Begin Source File

SOURCE=.\src\pixacc.c
# End Source File
# Begin Source File

SOURCE=.\src\pixafunc1.c
# End Source File
# Begin Source File

SOURCE=.\src\pixafunc2.c
# End Source File
# Begin Source File

SOURCE=.\src\pixalloc.c
# End Source File
# Begin Source File

SOURCE=.\src\pixarith.c
# End Source File
# Begin Source File

SOURCE=.\src\pixconv.c
# End Source File
# Begin Source File

SOURCE=.\src\pixtiling.c
# End Source File
# Begin Source File

SOURCE=.\src\pngio.c
# End Source File
# Begin Source File

SOURCE=.\src\pnmio.c
# End Source File
# Begin Source File

SOURCE=.\src\projective.c
# End Source File
# Begin Source File

SOURCE=.\src\psio.c
# End Source File
# Begin Source File

SOURCE=.\src\ptra.c
# End Source File
# Begin Source File

SOURCE=.\src\pts.c
# End Source File
# Begin Source File

SOURCE=.\src\queue.c
# End Source File
# Begin Source File

SOURCE=.\src\rank.c
# End Source File
# Begin Source File

SOURCE=.\src\readbarcode.c
# End Source File
# Begin Source File

SOURCE=.\src\readfile.c
# End Source File
# Begin Source File

SOURCE=.\src\rop.c
# End Source File
# Begin Source File

SOURCE=.\src\ropiplow.c
# End Source File
# Begin Source File

SOURCE=.\src\roplow.c
# End Source File
# Begin Source File

SOURCE=.\src\rotate.c
# End Source File
# Begin Source File

SOURCE=.\src\rotateam.c
# End Source File
# Begin Source File

SOURCE=.\src\rotateamlow.c
# End Source File
# Begin Source File

SOURCE=.\src\rotateorth.c
# End Source File
# Begin Source File

SOURCE=.\src\rotateorthlow.c
# End Source File
# Begin Source File

SOURCE=.\src\rotateshear.c
# End Source File
# Begin Source File

SOURCE=.\src\runlength.c
# End Source File
# Begin Source File

SOURCE=.\src\sarray.c
# End Source File
# Begin Source File

SOURCE=.\src\scale.c
# End Source File
# Begin Source File

SOURCE=.\src\scalelow.c
# End Source File
# Begin Source File

SOURCE=.\src\seedfill.c
# End Source File
# Begin Source File

SOURCE=.\src\seedfilllow.c
# End Source File
# Begin Source File

SOURCE=.\src\sel1.c
# End Source File
# Begin Source File

SOURCE=.\src\sel2.c
# End Source File
# Begin Source File

SOURCE=.\src\selgen.c
# End Source File
# Begin Source File

SOURCE=.\src\shear.c
# End Source File
# Begin Source File

SOURCE=.\src\skew.c
# End Source File
# Begin Source File

SOURCE=.\src\stack.c
# End Source File
# Begin Source File

SOURCE=.\src\textops.c
# End Source File
# Begin Source File

SOURCE=.\src\tiffio.c
# End Source File
# Begin Source File

SOURCE=.\src\tiffiostub.c
# End Source File
# Begin Source File

SOURCE=.\src\utils.c
# End Source File
# Begin Source File

SOURCE=.\src\viewfiles.c
# End Source File
# Begin Source File

SOURCE=.\src\warper.c
# End Source File
# Begin Source File

SOURCE=.\src\watershed.c
# End Source File
# Begin Source File

SOURCE=.\src\writefile.c
# End Source File
# Begin Source File

SOURCE=.\src\zlibmem.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\allheaders.h
# End Source File
# Begin Source File

SOURCE=.\src\alltypes.h
# End Source File
# Begin Source File

SOURCE=.\src\array.h
# End Source File
# Begin Source File

SOURCE=.\src\arrayaccess.h
# End Source File
# Begin Source File

SOURCE=.\src\bbuffer.h
# End Source File
# Begin Source File

SOURCE=.\src\bmf.h
# End Source File
# Begin Source File

SOURCE=.\src\bmp.h
# End Source File
# Begin Source File

SOURCE=.\src\ccbord.h
# End Source File
# Begin Source File

SOURCE=.\src\environ.h
# End Source File
# Begin Source File

SOURCE=.\src\freetype.h
# End Source File
# Begin Source File

SOURCE=.\src\gplot.h
# End Source File
# Begin Source File

SOURCE=.\src\heap.h
# End Source File
# Begin Source File

SOURCE=.\src\imageio.h
# End Source File
# Begin Source File

SOURCE=.\src\jbclass.h
# End Source File
# Begin Source File

SOURCE=.\src\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\src\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\src\leptprotos.h
# End Source File
# Begin Source File

SOURCE=.\src\list.h
# End Source File
# Begin Source File

SOURCE=.\src\morph.h
# End Source File
# Begin Source File

SOURCE=.\src\pix.h
# End Source File
# Begin Source File

SOURCE=.\src\ptra.h
# End Source File
# Begin Source File

SOURCE=.\src\queue.h
# End Source File
# Begin Source File

SOURCE=.\src\readbarcode.h
# End Source File
# Begin Source File

SOURCE=.\src\stack.h
# End Source File
# Begin Source File

SOURCE=.\src\watershed.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
