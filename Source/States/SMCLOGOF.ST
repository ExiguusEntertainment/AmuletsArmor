#---------------------------------------------------------------------------
# FILE: SMCLOGOF.ST
#---------------------------------------------------------------------------
# State Machine: Client Logoff State Machine
#
# Who Date     Notes:
# --- -------- ---------------
# LES 03/04/95 Created
#
#---------------------------------------------------------------------------


Name SMCLogoff
Initials LES

State SaveCharacter
  Enter
  Idle
  Cond Flag SAVE_COMPLETE Exit
  Cond Flag TIMEOUT Timeout
  Cond Flag SAVE_ERROR ReportError

State Timeout
  Enter

State ReportError
  Enter
  Cond Flag REPORT_ERROR_COMPLETE Exit

State Exit

#---------------------------------------------------------------------------
# END OF FILE: SMCLOGOF.ST
#---------------------------------------------------------------------------
