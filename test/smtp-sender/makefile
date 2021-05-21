all: debug release


debug: coratools_debug
	msbuild smtp-sender.vcxproj /p:Configuration=Debug


release: coratools_release
	msbuild smtp-sender.vcxproj /p:Configuration=Release


coratools_debug: nothing
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)


coratools_release: nothing
	cd ..\..\
	nmake release_uni
	cd $(MAKEDIR)


nothing:
	@echo doing nothing
