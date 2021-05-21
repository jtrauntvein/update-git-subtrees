all: debug release

debug: nothing
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)
	msbuild alert2.vcxproj /p:Configuration=Debug

release: nothing
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
	msbuild alert2.vcxproj /p:Configuration=Release

nothing:
	@echo "doing nothing"
