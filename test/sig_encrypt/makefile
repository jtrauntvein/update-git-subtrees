all: debug release

debug: coratools_debug
	msbuild sig_encrypt.vcxproj /p:Configuration=Debug 

release: coratools_release
	msbuild sig_encrypt.vcxproj /p:Configuration=Release

coratools_debug: 
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)
