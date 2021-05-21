all: debug release

debug: coratools_debug wxtools_debug
	msbuild /p:Configuration=debug graph.vcxproj


release: coratools_debug wxtools_debug
	msbuild /p:Configuration=release graph.vcxproj


coratools_debug:
	cd ..\..
	nmake debug
	cd $(MAKEDIR)


coratools_release:
	cd ..\..
	nmake release
	cd $(MAKEDIR)


wxtools_debug:
	cd ..\..\..\wxtools
	nmake debug
	cd $(MAKEDIR)


wxtool_release:
	cd ..\..\..\wxtools
	nmake release
	cd $(MAKEDIR)
