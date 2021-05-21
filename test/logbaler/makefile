all: debug release


debug: coratools_debug
	vcbuild logbaler.vcproj debug

release: coratools_release
	vcbuild logbaler.vcproj release


coratools_debug: nothing
	cd ..\..
	nmake debug_dll
	cd $(MAKEDIR)


coratools_release: nothing
	cd ..\..
	nmake release_dll
	cd $(MAKEDIR)


nothing:
	@echo doing nothing