all: debug release

debug: coratools_debug
	msbuild event_xml.vcxproj /p:Configuration=Debug

release: coratools_release
	msbuild event_xml.vcxproj /p:Configuration=Release

coratools_debug:
	cd ..\..
	nmake debug
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release
	cd $(MAKEDIR)

