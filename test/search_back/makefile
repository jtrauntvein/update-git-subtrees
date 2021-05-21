all: debug

debug: search_back.exe

search_back.exe: main.cpp
	cd ../..
	nmake debug
	cd $(MAKEDIR)
	cl /MTd /EHsc /Zi main.cpp ../../debug_static/coratools.lib advapi32.lib

