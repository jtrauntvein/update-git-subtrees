hhc=/tools/htmlhelp/hhc.exe

all: coratools.chm

coratools.chm: coratools.hhp
	$(hhc) coratools.hhp


coratools.hhp: coratools.dbk driver.xsl
	xsltproc driver.xsl coratools.dbk
	mv htmlhelp.hhp coratools.hhp