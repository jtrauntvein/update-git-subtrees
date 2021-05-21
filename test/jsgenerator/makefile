all: debug

debug: coratools_debug
	devenv jsgenerator.vcxproj /build debug

release: coratools_release
	devenv jsgenerator.vcxproj /build release

coratools_debug: nothing
	cd ../..
	nmake debug_dll
	cd $(MAKEDIR)

coratools_release: nothing
	cd ../../
	nmake release_dll
	cd $(MAKEDIR)

nothing:
	@echo doing nothing
