# cannon.mak
# Created by IBM WorkFrame/2 MakeMake at 19:58:40 on 17 Sep 2004
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .C .RC .obj .res

.all: \
    cannon.exe

.C.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Gh /Ti /Gm /Gd /Ss /Fo"%|dpfF.obj" /C %s

.RC.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s %|dpfF.RES

cannon.exe: \
    cannon.obj \
    resources.res \
    {$(LIB)}cppopa3.obj
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /de /pmtype:pm /noe"
     /Fecannon.exe
     cannon.obj
     cppom30.lib
<<
    rc.exe resources.res cannon.exe

cannon.obj: \
    cannon.C \
    {$(INCLUDE)}cannon.h

resources.res: \
    resources.RC \
    {$(INCLUDE)}resource.h


