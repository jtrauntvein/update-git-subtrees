all: debug release


debug: coratools_debug
	msbuild software_license.vcxproj /p:Configuration=debug

release: coratools_release
	msbuild software_license.vcxproj /p:Configuation=release


coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
