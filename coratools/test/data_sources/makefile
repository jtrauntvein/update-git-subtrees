all: 

debug:  nothing
	cd ../..
	nmake debug_uni
	cd $(MAKEDIR)
	msbuild data_sources.vcxproj /p:Configuration=debug 
	copy /y *.dll debug

release: nothing
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
	msbuild data_sources.vcxproj /p:Configuration=release
	copy /y *.dll release

csitypes: nothing
	cd ../../../csitypes
	nmake debug
	mkdir -p $(MAKEDIR)\debug
	copy /y debug\*.dll "$(MAKEDIR)\debug" 
	cd $(MAKEDIR)

nothing: 
	@echo doing nothing
