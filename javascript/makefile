ifeq ($(DO_JSHINT), 1)
%.h: %.js mkcdecl.exe makefile
	jshint --reporter=jshint-report.js --config=jshint-config.js $<
	./mkcdecl.exe $@ $< $* Csi Expression Javascript
else
%.h: %.js mkcdecl.exe makefile
	./mkcdecl.exe $@ $< $* Csi Expression Javascript
endif

scripts = CsiAbsoluteValue.h CsiAddition.h CsiAndOperator.h CsiArcCosine.h CsiArcSine.h \
	CsiArcTangent.h CsiArcTangent2.h CsiAvgRun.h CsiAvgRunOverTime.h CsiAvgRunOverTimeWithReset.h \
	CsiCeiling.h CsiConstant.h CsiCosine.h CsiGreater.h CsiGreaterEqual.h CsiLessEqual.h \
	CsiModulo.h CsiNot.h CsiNotEqual.h CsiDivision.h CsiEquivalence.h CsiEtoPower.h \
	CsiExponentiation.h CsiFix.h CsiFloor.h CsiFormatFloat.h CsiFormatTime.h CsiFractionalPart.h \
	CsiHex.h CsiHexToDec.h CsiHyperbolicCosine.h CsiIIF.h CsiImplication.h CsiInStr.h CsiInStrRev.h \
	CsiInt.h CsiIsFinite.h CsiLTrim.h CsiLast.h CsiLeft.h CsiLen.h CsiLgrDate.h CsiLogBase10.h \
	CsiMaxRun.h CsiMaxRunOverTime.h CsiMaxRunOverTimeWithReset.h CsiMedianRun.h CsiMedianRunOverTime.h \
	CsiMid.h CsiMinRun.h CsiMinRunOverTime.h CsiMinRunOverTimeWithReset.h CsiModulo.h \
	CsiMultiplication.h CsiNaturalLog.h CsiNegation.h CsiRTrim.h CsiReplace.h CsiRight.h CsiRound.h \
	CsiSpace.h CsiStdDev.h CsiStdDevOverTime.h CsiStdDevOverTimeWithReset.h CsiStrComp.h \
	CsiStrReverse.h CsiSubtraction.h CsiSynchVariable.h CsiTimestamp.h CsiToDate.h CsiToFloat.h \
	CsiToInt.h CsiTotal.h CsiTotalOverTime.h CsiTotalOverTimeWithReset.h CsiTrim.h CsiValueAtTime.h \
	CsiExpression.h CsiHyperbolicSine.h CsiHyperbolicTangent.h CsiIsEqual.h CsiLess.h CsiOrOperator.h CsiRandom.h CsiSign.h \
	CsiSine.h CsiSquareRoot.h CsiTangent.h CsiXorOperator.h calc_std_dev.h maintain_historic_values.h \
	maintain_resettable_values.h CsiVariable.h CsiWebQuery.h CsiDataManager.h jquery.h sprintf.h \
	CsiComponent.h TestComponent.h CsiUtility.h CsiOneShotTimer.h CsiClockChecker.h CsiSystemTime.h \
	CsiSystemTimeGmt.h CsiServerTime.h CsiExprToken.h CsiSetTimestamp.h CsiAvgSpa.h CsiMinSpa.h CsiMaxSpa.h \
	CsiAlarmsManager.h CsiSwitchFunction.h CsiLocalToGmt.h CsiGmtToLocal.h CsiCompareNumbers.h \
	CsiUnits.h CsiColour.h DcpGraph.h CsiSourceTimeVariable.h


all: $(scripts)

clean:
	rm *.h


CsiAbsoluteValue.h: CsiAbsoluteValue.js

CsiAddition.h: CsiAddition.js

CsiAndOperator.h: CsiAndOperator.js

CsiArcCosine.h: CsiArcCosine.js

CsiArcSine.h: CsiArcSine.js 


CsiArcTangent.h: CsiArcTangent.js

CsiArcTangent2.h: CsiArcTangent2.js

CsiAvgRun.h: CsiAvgRun.js

CsiAvgRunOverTime.h: CsiAvgRunOverTime.js

CsiAvgRunOverTimeWithReset.h: CsiAvgRunOverTimeWithReset.js 


CsiCeiling.h: CsiCeiling.js

CsiConstant.h: CsiConstant.js

CsiCosine.h: CsiCosine.js

CsiGreater.h: CsiGreater.js

CsiGreaterEqual.h: CsiGreaterEqual.js

CsiLessEqual.h: CsiLessEqual.js 

CsiLess.h: CsiLess.js

CsiOrOperator.h: CsiOrOperator.js

CsiXorOperator.h: CsiXorOperator.js

CsiModulo.h: CsiModulo.js

CsiNot.h: CsiNot.js

CsiNotEqual.h: CsiNotEqual.js

CsiDivision.h: CsiDivision.js

CsiEquivalence.h: CsiEquivalence.js

CsiEtoPower.h: CsiEtoPower.js 


CsiExponentiation.h: CsiExponentiation.js

CsiFix.h: CsiFix.js

CsiFloor.h: CsiFloor.js

CsiFormatFloat.h: CsiFormatFloat.js

CsiFormatTime.h: CsiFormatTime.js

CsiFractionalPart.h: CsiFractionalPart.js 


CsiHex.h: CsiHex.js

CsiHexToDec.h: CsiHexToDec.js

CsiHyperbolicCosine.h: CsiHyperbolicCosine.js

CsiIIF.h: CsiIIF.js

CsiImplication.h: CsiImplication.js

CsiInStr.h: CsiInStr.js

CsiInStrRev.h: CsiInStrRev.js 


CsiInt.h: CsiInt.js

CsiIsFinite.h: CsiIsFinite.js

CsiLTrim.h: CsiLTrim.js

CsiLast.h: CsiLast.js

CsiLeft.h: CsiLeft.js

CsiLen.h: CsiLen.js

CsiLgrDate.h: CsiLgrDate.js

CsiLogBase10.h: CsiLogBase10.js 


CsiMaxRun.h: CsiMaxRun.js

CsiMaxRunOverTime.h: CsiMaxRunOverTime.js

CsiMaxRunOverTimeWithReset.h: CsiMaxRunOverTimeWithReset.js

CsiMedianRun.h: CsiMedianRun.js

CsiMedianRunOverTime.h: CsiMedianRunOverTime.js

CsiMid.h: CsiMid.js

CsiMinRun.h: CsiMinRun.js

CsiMinRunOverTime.h: CsiMinRunOverTime.js

CsiMinRunOverTimeWithReset.h: CsiMinRunOverTimeWithReset.js

CsiModulo.h: CsiModulo.js 


CsiMultiplication.h: CsiMultiplication.js

CsiNaturalLog.h: CsiNaturalLog.js

CsiNegation.h: CsiNegation.js

CsiRTrim.h: CsiRTrim.js

CsiReplace.h: CsiReplace.js

CsiRight.h: CsiRight.js

CsiRound.h: CsiRound.js 


CsiSourceTimeVariable.h: CsiSourceTimeVariable.js

CsiSpace.h: CsiSpace.js

CsiStdDev.h: CsiStdDev.js

CsiStdDevOverTime.h: CsiStdDevOverTime.js

CsiStdDevOverTimeWithReset.h: CsiStdDevOverTimeWithReset.js

CsiStrComp.h: CsiStrComp.js 


CsiStrReverse.h: CsiStrReverse.js

CsiSubtraction.h: CsiSubtraction.js

CsiSynchVariable.h: CsiSynchVariable.js

CsiTimestamp.h: CsiTimestamp.js

CsiToDate.h: CsiToDate.js

CsiToFloat.h: CsiToFloat.js 


CsiToInt.h: CsiToInt.js

CsiTotal.h: CsiTotal.js

CsiTotalOverTime.h: CsiTotalOverTime.js

CsiTotalOverTimeWithReset.h: CsiTotalOverTimeWithReset.js

CsiTrim.h: CsiTrim.js

CsiValueAtTime.h: CsiValueAtTime.js 


CsiExpression.h: CsiExpression.js

HyperbolicSine.h: HyperbolicSine.js

HyperbolicTangent.h: HyperbolicTangent.js

IsEqual.h: IsEqual.js

Less.h: Less.js

OrOperator.h: OrOperator.js

Random.h: Random.js

Sign.h: Sign.js 


Sine.h: Sine.js

SquareRoot.h: SquareRoot.js

Tangent.h: Tangent.js

XorOperator.h: XorOperator.js

calc_std_dev.h: calc_std_dev.js

maintain_historic_values.h: maintain_historic_values.js

maintain_resettable_values.h: maintain_resettable_values.js

Variable.h: variable.js

CsiWebQuery.h: CsiWebQuery.js

CsiDataManager.h: CsiDataManager.js

jquery.h: jquery.js
	./mkcdecl.exe $@ $< $* Csi Expression Javascript

sprintf.h: sprintf.js

CsiComponent.h: CsiComponent.js

TestComponent.h: TestComponent.js

CsiUtility.h: CsiUtility.js

CsiOneShotTimer.h: CsiOneShotTimer.js

CsiClockChecker.h: CsiClockChecker.js

CsiSystemTime.h: CsiSystemTime.js

CsiSystemTimeGmt.h: CsiSystemTimeGmt.js

CsiExprToken.h: CsiExprToken.js

CsiSetTimestamp.h: CsiSetTimestamp.js

CsiAvgSpa.h: CsiAvgSpa.js

CsiMinSpa.h: CsiMinSpa.js

CsiMaxSpa.h: CsiMaxSpa.js

CsiAlarmsManager.h: CsiAlarmsManager.js

CsiSwitchFunction.h: CsiSwitchFunction.js

CsiGmtToLocal.h: CsiGmtToLocal.js

CsiLocalToGmt.h: CsiLocalToGmt.js

CsiCompareNumbers.h: CsiCompareNumbers.js

CsiUnits.h: CsiUnits.js

CsiColour.h: CsiColour.js

DcpGraph.h: DcpGraph.js

ifndef WIN32

mkcdecl.exe: ../mkcdecl.c
	gcc -o mkcdecl.exe ../mkcdecl.c

else

mkcdecl.exe: ../mkcdecl.c
	cl ../mkcdecl.c

endif
