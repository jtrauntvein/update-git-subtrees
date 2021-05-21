all: debug release


debug: coratools_debug
	msbuild /p:Configuration=debug md9emu.vcxproj


release: coratools_release
	msbuild /p:Configuration=release md9emu.vcxproj


coratools_debug:
	cd ..\..
	nmake debug
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release
	cd $(MAKEDIR)
