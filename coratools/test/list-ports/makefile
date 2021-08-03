all: debug release

debug: coratools_debug
	msbuild /p:Configuration=Debug list-ports.vcxproj


release: coratools_release
	msbuild /p:Configuration=Release list-ports.vcxproj


coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)


coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
