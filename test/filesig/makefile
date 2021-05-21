all: debug release


debug: coratools_debug
	msbuild filesig.vcxproj /p:Congiguration=debug 


release: coratools_release
	msbuild filesig.vcxproj /p:Configuration=release

coratools_debug:
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)


coratools_release:
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)

