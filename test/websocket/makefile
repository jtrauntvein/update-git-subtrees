all: debug release

debug: coratools_debug
	msbuild websocket.vcxproj /p:Configuration=Debug

release: coratools_release
	msbuild websocket.vcxproj /p:Configuration=Release


coratools_debug:
	cd ..\..
	nmake debug
	cd $(MAKEDIR)


coratools_release:
	cd ..\..
	nmake release
	cd $(MAKEDIR)
