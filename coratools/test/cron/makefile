all: debug

debug: coratools_debug
	msbuild cron.vcxproj /p:Configuration=debug


coratools_debug: 
	cd ..\..
	nmake debug_uni
	cd $(MAKEDIR)
