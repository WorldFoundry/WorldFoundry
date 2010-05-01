set WF_TARGET=psx

set STREAMING=off
set BUILDMODE=debug
cd pigs2
make
cd ..\source
make
cd ..

set BUILDMODE=release
cd pigs2
make
cd ..\source
make
cd ..

set STREAMING=hd
set BUILDMODE=debug
cd pigs2
make
cd ..\source
make
cd ..

set BUILDMODE=release
cd pigs2
make
cd ..\source
make
cd ..

set STREAMING=cd
set BUILDMODE=debug
cd pigs2
make
cd ..\source
make
cd ..

set BUILDMODE=release
cd pigs2
make
cd ..\source
make
cd ..
