all: debug

debug: coratools_debug
	msbuild /p:Configuration=Debug

release: coratools_release
	msbuild /p:Configuration=Release

coratools_debug:
	cd ..\..
	nmake debug
	cd $(MAKEDIR)


coratools_release:
	cd ..\..\
	nmake release
	cd $(MAKEDIR)
