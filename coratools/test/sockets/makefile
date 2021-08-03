all: debug release

debug: coratools_debug
	msbuild /p:Configuration=Debug sockets.vcxproj

release: coratools_release
	msbuild /p:Configuration=Release sockets.vcxproj

coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
