all: debug

debug: base_32_encoder.exe

base_32_encoder.exe: main.cpp
	cd ../..
	nmake debug_uni
	cd $(MAKEDIR)
	cl /MTd /EHsc /Zi main.cpp ../../debug_uni/coratools.lib advapi32.lib

