all: debug release

debug: coratools_debug
	msbuild test_units.vcxproj /p:Configuration=Debug

release: coratools_release
	msbuild test_units.vcxproj /p:Configuration=Release


coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)


coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)

