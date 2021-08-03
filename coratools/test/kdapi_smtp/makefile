all: debug

debug: coratools_debug
	msbuild  SmtpTest.vcxproj /p:Configuration=Debug

release: coratools_release
	msbuild SmtpTest.vcxproj /p:Configuration=Release


coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)


coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
