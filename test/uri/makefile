all: debug


debug: coratools_debug makefile
	msbuild uri.vcxproj /p:Configuration=debug

nothing:
	@echo doing nothing


coratools_debug: nothing
	cd ../..
	nmake debug
	cd $(MAKEDIR)
