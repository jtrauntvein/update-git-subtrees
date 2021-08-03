all: debug release

debug: debug_uni debug_uni64

release: release_uni release_uni64

debug_uni: javascript
	msbuild coratools.vcxproj /p:Configuration=debug_uni

debug_uni64: javascript
	msbuild coratools.vcxproj /p:Configuration=debug_uni64 /p:Platform=x64

release_uni: javascript
	msbuild coratools.vcxproj /p:Configuration=release_uni

release_uni64: javascript
	msbuild coratools.vcxproj /p:Configuration=release_uni64 /p:Platform=x64


publish: release

clean:
	rm -rf debug_uni release_uni debug_uni64 release_uni64

javascript: nothing
	cd javascript
	make WIN32=true DO_JSHINT=$(DO_JSHINT)
	cd $(MAKEDIR)


boost: nothing
	cd ..\..\boost-1.55.0
	bootstrap
	.\b2
	cd $(MAKEDIR)

nothing:
	@echo doing nothing
