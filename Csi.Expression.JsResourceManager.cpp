/* Csi.Expression.JsResourceManager.cpp

   Copyright (C) 2010, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 July 2010
   Last Change: Thursday 16 May 2019
   Last Commit: $Date: 2020-07-14 15:47:08 -0600 (Tue, 14 Jul 2020) $
   Last Changed by: $Author: jbritt $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Expression.JsResourceManager.h"
#include "javascript/CsiAbsoluteValue.h"
#include "javascript/CsiAddition.h"
#include "javascript/CsiAlarmsManager.h"
#include "javascript/CsiAndOperator.h"
#include "javascript/CsiArcCosine.h"
#include "javascript/CsiArcSine.h"
#include "javascript/CsiArcTangent.h"
#include "javascript/CsiArcTangent2.h"
#include "javascript/CsiAvgRun.h"
#include "javascript/CsiAvgRunOverTime.h"
#include "javascript/CsiAvgRunOverTimeWithReset.h"
#include "javascript/CsiCeiling.h"
#include "javascript/CsiConstant.h"
#include "javascript/CsiCosine.h"
#include "javascript/CsiGreater.h"
#include "javascript/CsiGreaterEqual.h"
#include "javascript/CsiLessEqual.h"
#include "javascript/CsiModulo.h"
#include "javascript/CsiNot.h"
#include "javascript/CsiNotEqual.h"
#include "javascript/CsiDivision.h"
#include "javascript/CsiEquivalence.h"
#include "javascript/CsiEtoPower.h"
#include "javascript/CsiExponentiation.h"
#include "javascript/CsiFix.h"
#include "javascript/CsiFloor.h"
#include "javascript/CsiFormatFloat.h"
#include "javascript/CsiFormatTime.h"
#include "javascript/CsiFractionalPart.h"
#include "javascript/CsiHex.h"
#include "javascript/CsiHexToDec.h"
#include "javascript/CsiHyperbolicCosine.h"
#include "javascript/CsiIIF.h"
#include "javascript/CsiImplication.h"
#include "javascript/CsiInStr.h"
#include "javascript/CsiInStrRev.h"
#include "javascript/CsiInt.h"
#include "javascript/CsiIsFinite.h"
#include "javascript/CsiLTrim.h"
#include "javascript/CsiLast.h"
#include "javascript/CsiLeft.h"
#include "javascript/CsiLen.h"
#include "javascript/CsiLgrDate.h"
#include "javascript/CsiLogBase10.h"
#include "javascript/CsiMaxRun.h"
#include "javascript/CsiMaxRunOverTime.h"
#include "javascript/CsiMaxRunOverTimeWithReset.h"
#include "javascript/CsiMedianRun.h"
#include "javascript/CsiMedianRunOverTime.h"
#include "javascript/CsiMid.h"
#include "javascript/CsiMinRun.h"
#include "javascript/CsiMinRunOverTime.h"
#include "javascript/CsiMinRunOverTimeWithReset.h"
#include "javascript/CsiMultiplication.h"
#include "javascript/CsiNaturalLog.h"
#include "javascript/CsiNegation.h"
#include "javascript/CsiRTrim.h"
#include "javascript/CsiReplace.h"
#include "javascript/CsiRight.h"
#include "javascript/CsiRound.h"
#include "javascript/CsiSourceTimeVariable.h"
#include "javascript/CsiSpace.h"
#include "javascript/CsiStdDev.h"
#include "javascript/CsiStdDevOverTime.h"
#include "javascript/CsiStdDevOverTimeWithReset.h"
#include "javascript/CsiStrComp.h"
#include "javascript/CsiStrReverse.h"
#include "javascript/CsiSubtraction.h"
#include "javascript/CsiSynchVariable.h"
#include "javascript/CsiTimestamp.h"
#include "javascript/CsiToDate.h"
#include "javascript/CsiToFloat.h"
#include "javascript/CsiToInt.h"
#include "javascript/CsiTotal.h"
#include "javascript/CsiTotalOverTime.h"
#include "javascript/CsiTotalOverTimeWithReset.h"
#include "javascript/CsiTrim.h"
#include "javascript/CsiValueAtTime.h"
#include "javascript/CsiHyperbolicSine.h"
#include "javascript/CsiHyperbolicTangent.h"
#include "javascript/CsiIsEqual.h"
#include "javascript/CsiLess.h"
#include "javascript/CsiOrOperator.h"
#include "javascript/CsiRandom.h"
#include "javascript/CsiSign.h"
#include "javascript/CsiSine.h"
#include "javascript/CsiSquareRoot.h"
#include "javascript/CsiTangent.h"
#include "javascript/CsiXorOperator.h"
#include "javascript/CsiVariable.h"
#include "javascript/CsiExpression.h"
#include "javascript/calc_std_dev.h"
#include "javascript/maintain_historic_values.h"
#include "javascript/maintain_resettable_values.h"
#include "javascript/CsiWebQuery.h"
#include "javascript/CsiDataManager.h"
#include "javascript/sprintf.h"
#include "javascript/jquery.h"
#include "javascript/CsiComponent.h"
#include "javascript/TestComponent.h"
#include "javascript/CsiUtility.h"
#include "javascript/CsiOneShotTimer.h"
#include "javascript/CsiClockChecker.h"
#include "javascript/CsiSystemTime.h"
#include "javascript/CsiSystemTimeGmt.h"
#include "javascript/CsiServerTime.h"
#include "javascript/CsiExprToken.h"
#include "javascript/CsiSetTimestamp.h"
#include "javascript/CsiAvgSpa.h"
#include "javascript/CsiMinSpa.h"
#include "javascript/CsiMaxSpa.h"
#include "javascript/CsiSwitchFunction.h"
#include "javascript/CsiGmtToLocal.h"
#include "javascript/CsiLocalToGmt.h"
#include "javascript/CsiCompareNumbers.h"
#include "javascript/CsiUnits.h"
#include "javascript/CsiColour.h"
#include "javascript/DcpGraph.h"


namespace Csi
{
   namespace Expression
   {
      namespace Javascript
      {
         ////////////////////////////////////////////////////////////
         // JsResourceInit
         ////////////////////////////////////////////////////////////
         ResourceInit JsResourceInit[] =
         {
            { "CsiAbsoluteValue.js", CsiAbsoluteValue, sizeof(CsiAbsoluteValue), CsiAbsoluteValue_date,
              { "CsiExpression.js" } },
            { "CsiAddition.js", CsiAddition, sizeof(CsiAddition), CsiAddition_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiAndOperator.js", CsiAndOperator, sizeof(CsiAndOperator), CsiAndOperator_date,
              { "CsiExpression.js" } },
            { "CsiArcCosine.js", CsiArcCosine, sizeof(CsiArcCosine), CsiArcCosine_date,
              { "CsiExpression.js" } },
            { "CsiArcSine.js", CsiArcSine, sizeof(CsiArcSine), CsiArcSine_date,
              { "CsiExpression.js" } },
            { "CsiArcTangent.js", CsiArcTangent, sizeof(CsiArcTangent), CsiArcTangent_date,
              { "CsiExpression.js" } },
            { "CsiArcTangent2.js", CsiArcTangent2, sizeof(CsiArcTangent2), CsiArcTangent2_date,
              { "CsiExpression.js" } },
            { "CsiAvgRun.js", CsiAvgRun, sizeof(CsiAvgRun), CsiAvgRun_date,
              { "CsiExpression.js" } },
            { "CsiAvgRunOverTime.js", CsiAvgRunOverTime, sizeof(CsiAvgRunOverTime), CsiAvgRunOverTime_date,
              { "CsiExpression.js", "maintain_historic_values.js" } },
            { "CsiAvgRunOverTimeWithReset.js", CsiAvgRunOverTimeWithReset, sizeof(CsiAvgRunOverTimeWithReset), CsiAvgRunOverTimeWithReset_date,
              { "CsiExpression.js", "maintain_resettable_values.js" } },
            { "CsiCeiling.js", CsiCeiling, sizeof(CsiCeiling), CsiCeiling_date,
              { "CsiExpression.js" } },
            { "CsiConstant.js", CsiConstant, sizeof(CsiConstant), CsiConstant_date,
              { "CsiExpression.js" } },
            { "CsiCosine.js", CsiCosine, sizeof(CsiCosine), CsiCosine_date,
              { "CsiExpression.js" } },
            { "CsiGreater.js", CsiGreater, sizeof(CsiGreater), CsiGreater_date,
              { "CsiExpression.js", "CsiCompareNumbers.js" } },
            { "CsiGreaterEqual.js", CsiGreaterEqual, sizeof(CsiGreaterEqual), CsiGreaterEqual_date,
              { "CsiExpression.js", "CsiCompareNumbers.js" } },
            { "CsiLessEqual.js", CsiLessEqual, sizeof(CsiLessEqual), CsiLessEqual_date,
               { "CsiExpression.js", "CsiCompareNumbers.js" } },
            { "CsiIsEqual.js", CsiIsEqual, sizeof(CsiIsEqual), CsiIsEqual_date,
               { "CsiExpression.js", "CsiCompareNumbers.js" } },
            { "CsiLess.js", CsiLess, sizeof(CsiLess), CsiLess_date,
               { "CsiExpression.js", "CsiCompareNumbers.js" } },
            { "CsiModulo.js", CsiModulo, sizeof(CsiModulo), CsiModulo_date,
               { "CsiExpression.js" } },
            { "CsiNot.js", CsiNot, sizeof(CsiNot), CsiNot_date,
              { "CsiExpression.js" } },
            { "CsiNotEqual.js", CsiNotEqual, sizeof(CsiNotEqual), CsiNotEqual_date,
              { "CsiExpression.js", "CsiCompareNumbers.js" } },
            { "CsiDivision.js", CsiDivision, sizeof(CsiDivision), CsiDivision_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiEquivalence.js", CsiEquivalence, sizeof(CsiEquivalence), CsiEquivalence_date,
              { "CsiExpression.js" } },
            { "CsiEtoPower.js", CsiEtoPower, sizeof(CsiEtoPower), CsiEtoPower_date,
              { "CsiExpression.js" } },
            { "CsiExponentiation.js", CsiExponentiation, sizeof(CsiExponentiation), CsiExponentiation_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiFix.js", CsiFix, sizeof(CsiFix), CsiFix_date,
              { "CsiExpression.js" } },
            { "CsiFloor.js", CsiFloor, sizeof(CsiFloor), CsiFloor_date,
              { "CsiExpression.js" } },
            { "CsiFormatFloat.js", CsiFormatFloat, sizeof(CsiFormatFloat), CsiFormatFloat_date,
              { "CsiExpression.js", "sprintf.js" } },
            { "CsiFormatTime.js", CsiFormatTime, sizeof(CsiFormatTime), CsiFormatTime_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiFractionalPart.js", CsiFractionalPart, sizeof(CsiFractionalPart), CsiFractionalPart_date,
              { "CsiExpression.js" } },
            { "CsiHex.js", CsiHex, sizeof(CsiHex), CsiHex_date,
              { "CsiExpression.js", "sprintf.js" } },
            { "CsiHexToDec.js", CsiHexToDec, sizeof(CsiHexToDec), CsiHexToDec_date,
              { "CsiExpression.js" } },
            { "CsiHyperbolicCosine.js", CsiHyperbolicCosine, sizeof(CsiHyperbolicCosine), CsiHyperbolicCosine_date,
              { "CsiExpression.js" } },
            { "CsiIIF.js", CsiIIF, sizeof(CsiIIF), CsiIIF_date,
              { "CsiExpression.js" } },
            { "CsiSwitchFunction.js", CsiSwitchFunction, sizeof(CsiSwitchFunction), CsiSwitchFunction_date,
              { "CsiExpression.js" } },
            { "CsiImplication.js", CsiImplication, sizeof(CsiImplication), CsiImplication_date,
              { "CsiExpression.js" } },
            { "CsiInStr.js", CsiInStr, sizeof(CsiInStr), CsiInStr_date,
              { "CsiExpression.js" } },
            { "CsiInStrRev.js", CsiInStrRev, sizeof(CsiInStrRev), CsiInStrRev_date,
              { "CsiExpression.js" } },
            { "CsiInt.js", CsiInt, sizeof(CsiInt), CsiInt_date,
              { "CsiExpression.js" } },
            { "CsiIsFinite.js", CsiIsFinite, sizeof(CsiIsFinite), CsiIsFinite_date,
              { "CsiExpression.js" } },
            { "CsiLTrim.js", CsiLTrim, sizeof(CsiLTrim), CsiLTrim_date,
              { "CsiExpression.js" } },
            { "CsiLast.js", CsiLast, sizeof(CsiLast), CsiLast_date,
              { "CsiExpression.js" } },
            { "CsiLeft.js", CsiLeft, sizeof(CsiLeft), CsiLeft_date,
              { "CsiExpression.js" } },
            { "CsiLen.js", CsiLen, sizeof(CsiLen), CsiLen_date,
              { "CsiExpression.js" } },
            { "CsiLgrDate.js", CsiLgrDate, sizeof(CsiLgrDate), CsiLgrDate_date,
              { "sprintf.js" } },
            { "CsiLogBase10.js", CsiLogBase10, sizeof(CsiLogBase10), CsiLogBase10_date,
              { "CsiExpression.js" } },
            { "CsiMaxRun.js", CsiMaxRun, sizeof(CsiMaxRun), CsiMaxRun_date,
              { "CsiExpression.js" } },
            { "CsiMaxRunOverTime.js", CsiMaxRunOverTime, sizeof(CsiMaxRunOverTime), CsiMaxRunOverTime_date,
              { "CsiExpression.js", "maintain_historic_values.js" } },
            { "CsiMaxRunOverTimeWithReset.js", CsiMaxRunOverTimeWithReset, sizeof(CsiMaxRunOverTimeWithReset), CsiMaxRunOverTimeWithReset_date,
              { "CsiExpression.js", "maintain_resettable_values.js" } },
            { "CsiMedianRun.js", CsiMedianRun, sizeof(CsiMedianRun), CsiMedianRun_date,
              { "CsiExpression.js" } },
            { "CsiMedianRunOverTime.js", CsiMedianRunOverTime, sizeof(CsiMedianRunOverTime), CsiMedianRunOverTime_date,
              { "CsiMedianRun.js", "maintain_historic_values.js" } },
            { "CsiMid.js", CsiMid, sizeof(CsiMid), CsiMid_date,
              { "CsiExpression.js" } },
            { "CsiMinRun.js", CsiMinRun, sizeof(CsiMinRun), CsiMinRun_date,
              { "CsiExpression.js" } },
            { "CsiMinRunOverTime.js", CsiMinRunOverTime, sizeof(CsiMinRunOverTime), CsiMinRunOverTime_date,
              { "CsiExpression.js", "maintain_historic_values.js" } },
            { "CsiMinRunOverTimeWithReset.js", CsiMinRunOverTimeWithReset, sizeof(CsiMinRunOverTimeWithReset), CsiMinRunOverTimeWithReset_date,
              { "CsiExpression.js", "CsiLgrDate.js", "maintain_resettable_values.js" } },
            { "CsiModulo.js", CsiModulo, sizeof(CsiModulo), CsiModulo_date,
              { "CsiExpression.js" } },
            { "CsiMultiplication.js", CsiMultiplication, sizeof(CsiMultiplication), CsiMultiplication_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiNaturalLog.js", CsiNaturalLog, sizeof(CsiNaturalLog), CsiNaturalLog_date,
              { "CsiExpression.js" } },
            { "CsiNegation.js", CsiNegation, sizeof(CsiNegation), CsiNegation_date,
              { "CsiExpression.js" } },
            { "CsiRTrim.js", CsiRTrim, sizeof(CsiRTrim), CsiRTrim_date,
              { "CsiExpression.js" } },
            { "CsiReplace.js", CsiReplace, sizeof(CsiReplace), CsiReplace_date,
              { "CsiExpression.js" } },
            { "CsiRight.js", CsiRight, sizeof(CsiRight), CsiRight_date,
              { "CsiExpression.js" } },
            { "CsiRound.js", CsiRound, sizeof(CsiRound), CsiRound_date,
              { "CsiExpression.js" } },
            { "CsiSpace.js", CsiSpace, sizeof(CsiSpace), CsiSpace_date,
              { "CsiExpression.js" } },
            { "CsiStdDev.js", CsiStdDev, sizeof(CsiStdDev), CsiStdDev_date,
              { "CsiExpression.js", "calc_std_dev.js" } },
            { "CsiStdDevOverTime.js", CsiStdDevOverTime, sizeof(CsiStdDevOverTime), CsiStdDevOverTime_date,
              { "CsiExpression.js", "maintain_historic_values.js", "calc_std_dev.js" } },
            { "CsiStdDevOverTimeWithReset.js", CsiStdDevOverTimeWithReset, sizeof(CsiStdDevOverTimeWithReset), CsiStdDevOverTimeWithReset_date,
              { "CsiExpression.js", "maintain_resettable_values.js", "calc_std_dev.js" } },
            { "CsiStrComp.js", CsiStrComp, sizeof(CsiStrComp), CsiStrComp_date,
              { "CsiExpression.js" } },
            { "CsiStrReverse.js", CsiStrReverse, sizeof(CsiStrReverse), CsiStrReverse_date,
              { "CsiExpression.js" } },
            { "CsiSubtraction.js", CsiSubtraction, sizeof(CsiSubtraction), CsiSubtraction_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiSynchVariable.js", CsiSynchVariable, sizeof(CsiSynchVariable), CsiSynchVariable_date,
              { "CsiExpression.js", "CsiVariable.js" } },
            { "CsiTimestamp.js", CsiTimestamp, sizeof(CsiTimestamp), CsiTimestamp_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiSetTimestamp.js", CsiSetTimestamp, sizeof(CsiSetTimestamp), CsiSetTimestamp_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiToDate.js", CsiToDate, sizeof(CsiToDate), CsiToDate_date,
              { "CsiExpression.js", "CsiLgrDate.js" } },
            { "CsiToFloat.js", CsiToFloat, sizeof(CsiToFloat), CsiToFloat_date,
              { "CsiExpression.js" } },
            { "CsiToInt.js", CsiToInt, sizeof(CsiToInt), CsiToInt_date,
              { "CsiExpression.js" } },
            { "CsiTotal.js", CsiTotal, sizeof(CsiTotal), CsiTotal_date,
              { "CsiExpression.js" } },
            { "CsiTotalOverTime.js", CsiTotalOverTime, sizeof(CsiTotalOverTime), CsiTotalOverTime_date,
              { "CsiExpression.js", "maintain_historic_values.js" } },
            { "CsiTotalOverTimeWithReset.js", CsiTotalOverTimeWithReset, sizeof(CsiTotalOverTimeWithReset), CsiTotalOverTimeWithReset_date,
              { "CsiExpression.js", "maintain_resettable_values.js" } },
            { "CsiTrim.js", CsiTrim, sizeof(CsiTrim), CsiTrim_date,
              { "CsiExpression.js" } },
            { "CsiValueAtTime.js", CsiValueAtTime, sizeof(CsiValueAtTime), CsiValueAtTime_date,
              { "CsiExpression.js", "maintain_historic_values.js" } },
            { "CsiHyperbolicSine.js", CsiHyperbolicSine, sizeof(CsiHyperbolicSine), CsiHyperbolicSine_date,
              { "CsiExpression.js" } },
            { "CsiHyperbolicTangent.js", CsiHyperbolicTangent, sizeof(CsiHyperbolicTangent), CsiHyperbolicTangent_date,
              { "CsiExpression.js" } },
            { "CsiOrOperator.js", CsiOrOperator, sizeof(CsiOrOperator), CsiOrOperator_date,
              { "CsiExpression.js" } },
            { "CsiRandom.js", CsiRandom, sizeof(CsiRandom), CsiRandom_date,
              { "CsiExpression.js" } },
            { "CsiSign.js", CsiSign, sizeof(CsiSign), CsiSign_date,
              { "CsiExpression.js" } },
            { "CsiSine.js", CsiSine, sizeof(CsiSine), CsiSine_date,
              { "CsiExpression.js" } },
            { "CsiSquareRoot.js", CsiSquareRoot, sizeof(CsiSquareRoot), CsiSquareRoot_date,
              { "CsiExpression.js" } },
            { "CsiTangent.js", CsiTangent, sizeof(CsiTangent), CsiTangent_date,
              { "CsiExpression.js" } },
            { "CsiXorOperator.js", CsiXorOperator, sizeof(CsiXorOperator), CsiXorOperator_date,
              { "CsiExpression.js" } },
            { "CsiSourceTimeVariable.js", CsiSourceTimeVariable, sizeof(CsiSourceTimeVariable), CsiSourceTimeVariable_date,
              { "CsiLgrDate.js", "CsiClockChecker.js" }},
            { "CsiAvgSpa.js", CsiAvgSpa, sizeof(CsiAvgSpa), CsiAvgSpa_date,
              { "CsiExpression.js" } },
            { "CsiMinSpa.js", CsiMinSpa, sizeof(CsiMinSpa), CsiMinSpa_date,
              { "CsiExpression.js" } },
            { "CsiMaxSpa.js", CsiMaxSpa, sizeof(CsiMaxSpa), CsiMaxSpa_date,
              { "CsiExpression.js" } },
            { "CsiExpression.js", CsiExpression, sizeof(CsiExpression), CsiExpression_date,
              { "CsiExprToken.js", "CsiVariable.js", "CsiSourceTimeVariable.js" } },
            { "CsiVariable.js", CsiVariable, sizeof(CsiVariable), CsiVariable_date,
              { "CsiLgrDate.js" } },
            { "calc_std_dev.js", calc_std_dev, sizeof(calc_std_dev), calc_std_dev_date,
              { } },
            { "maintain_historic_values.js", maintain_historic_values, sizeof(maintain_historic_values), maintain_historic_values_date,
              { } },
            { "maintain_resettable_values.js", maintain_resettable_values, sizeof(maintain_resettable_values), maintain_resettable_values_date,
              { } },
            { "CsiWebQuery.js", CsiWebQuery, sizeof(CsiWebQuery), CsiWebQuery_date,
              { "CsiLgrDate.js", "CsiExpression.js" } },
            { "CsiDataManager.js", CsiDataManager, sizeof(CsiDataManager), CsiDataManager_date,
              { "CsiWebQuery.js", "CsiOneShotTimer.js", "CsiUtility.js", "CsiAlarmsManager.js" } },
            { "CsiAlarmsManager.js", CsiAlarmsManager, sizeof(CsiAlarmsManager), CsiAlarmsManager_date,
              { "CsiOneShotTimer.js" } },
            { "jquery.js", jquery, sizeof(jquery), jquery_date,
              { } },
            { "sprintf.js", sprintf, sizeof(sprintf), sprintf_date,
              { } },
            { "CsiComponent.js", CsiComponent, sizeof(CsiComponent), CsiComponent_date,
              { "CsiUtility.js", "CsiLgrDate.js", "CsiColour.js", "CsiOneShotTimer.js" } },
            { "TestComponent.js", TestComponent, sizeof(TestComponent), TestComponent_date,
              { "CsiComponent.js" } },
            { "CsiUtility.js", CsiUtility, sizeof(CsiUtility), CsiUtility_date,
              { } },
            { "CsiColour.js", CsiColour, sizeof(CsiColour), CsiColour_date,
              { } },
            { "CsiOneShotTimer.js", CsiOneShotTimer, sizeof(CsiOneShotTimer), CsiOneShotTimer_date,
              { } },
            { "CsiClockChecker.js", CsiClockChecker, sizeof(CsiClockChecker), CsiClockChecker_date,
              {"CsiUtility.js", "CsiOneShotTimer.js" } },
            { "CsiSystemTime.js", CsiSystemTime, sizeof(CsiSystemTime), CsiSystemTime_date,
              { "CsiLgrDate.js", "CsiExpression.js" } },
            { "CsiSystemTimeGmt.js", CsiSystemTimeGmt, sizeof(CsiSystemTimeGmt), CsiSystemTimeGmt_date,
              { "CsiLgrDate.js", "CsiExpression.js" } },
            { "CsiServerTime.js", CsiServerTime, sizeof(CsiServerTime), CsiServerTime_date,
              { "CsiLgrDate.js", "CsiExpression.js" } },
            { "CsiExprToken.js", CsiExprToken, sizeof(CsiExprToken), CsiExprToken_date,
              { "CsiLgrDate.js", "sprintf.js" } },
            { "CsiGmtToLocal.js", CsiGmtToLocal, sizeof(CsiGmtToLocal), CsiGmtToLocal_date,
              { "CsiExpression.js" } },
            { "CsiLocalToGmt.js", CsiLocalToGmt, sizeof(CsiLocalToGmt), CsiLocalToGmt_date,
              { "CsiExpression.js" } },
            { "CsiCompareNumbers.js", CsiCompareNumbers, sizeof(CsiCompareNumbers), CsiCompareNumbers_date,
              { } },
            { "CsiUnits.js", CsiUnits, sizeof(CsiUnits), CsiUnits_date,
              { } },
            { "DcpGraph.js", DcpGraph, sizeof(DcpGraph), DcpGraph_date,
              { "CsiLgrDate.js", "CsiColour.js", "CsiUtility.js", "CsiExpression.js" } },
            { "", 0, 0, "", { } }
         };
      };


      ////////////////////////////////////////////////////////////
      // class JsResourceManager definitions
      ////////////////////////////////////////////////////////////
      JsResourceManager::JsResourceManager()
      {
         add_resources(Javascript::JsResourceInit);
      } // constructor


      JsResourceManager::~JsResourceManager()
      {}
   };
};

