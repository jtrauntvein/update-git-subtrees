all: debug release

debug: coratools_debug
	msbuild expressions.vcxproj /p:Configuration=debug


release: coratools_release
	msbuild expressions.vcxproj /p:Configuration=release


coratools_debug: nothing
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)


coratools_release: nothing
	cd ..\..
	nmake release_uni
	cd $(MAKEDIR)


nothing:
	@echo doing nothing

clean: nothing
	rm -rf Debug Release *.ncb *.suo
