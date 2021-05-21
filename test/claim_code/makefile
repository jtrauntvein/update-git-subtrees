all: debug release

debug: coratools_debug
	msbuild claim_code.vcxproj /p:Configuration=debug

release: coratools_release
	msbuild claim_code.vcxproj /p:Configuration=release


coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)

coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)

