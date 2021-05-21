all: debug release

debug: coratools_debug test1.h
	msbuild /p:config=Debug nmea.vcxproj

release: coratools_release test1.h
	msbuild /p:config=Release nmea.vcxproj

coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)

test1.h: mkcdecl.exe test1.txt makefile
	mkcdecl test1.h test1.txt test1_txt Tests

mkcdecl.exe: ..\..\..\mkcdecl.c
	cl ..\..\..\mkcdecl.c
