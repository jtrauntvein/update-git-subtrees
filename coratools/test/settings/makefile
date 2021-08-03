all: debug release

debug: coratools_debug
	vcbuild settings.vcproj debug

release: coratools_release
	vcbuild settings.vcproj release

coratools_debug: nothing
	cd ..\..
	nmake debug
	cd $(MAKEDIR)

coratools_release: nothing
	cd ..\..
	nmake release
	cd $(MAKEDIR)

nothing:
	echo doing nothing
