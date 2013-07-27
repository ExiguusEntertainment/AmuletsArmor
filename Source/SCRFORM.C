/****************************************************************************/
/*    FILE:  SCRFORM.C                                                      */
/****************************************************************************/
#include "COLOR.H"
#include "SCRFORM.H"
#include "SCRIPT.H"
#include "SCRIPTEV.H"

static E_Boolean G_scriptCommExit = FALSE;
static T_word32 G_currentFormNumber = 0 ;
static E_Boolean G_scriptCommActive = FALSE ;
static T_script G_formScript = SCRIPT_BAD ;

static T_keyboardEventHandler G_oldKeyboardHandler = NULL ;
static T_mouseEventHandler G_oldMouseHandler = NULL ;

/* Internal prototypes: */
static T_void IScriptFormEventOneNumber(T_word16 eventNumber, T_sword32 num) ;
static T_void IScriptFormEventTwoNumbers(
                  T_word16 eventNumber,
                  T_sword32 num1,
                  T_sword32 num2) ;

/****************************************************************************/
/*  Routine:  ScriptFormStart                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScriptForm is called to present a form to the player and allow         */
/*  choices to be made and sent to the server.  The server can then make    */
/*  decisions and send them to this form.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine is not be recursive, so any call to this routine while   */
/*  it already being executed will cause an error.                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 uiFormNumber       -- Number of form to load                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FormLoadFromFile                                                      */
/*    FormSetCallbackRoutine                                                */
/*    GraphicUpdateAllGraphicsBuffered                                      */
/*    FormGenericControl                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/13/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScriptFormStart(T_word32 uiFormNumber)
{
    T_byte8 formName[80] ;
    static E_Boolean inThisRoutine = FALSE ;

    DebugRoutine ("ScriptFormStart");
    DebugCheck(inThisRoutine == FALSE) ;
    DebugCheck(G_scriptCommActive == FALSE) ;

    /* This routine cannot be recursed. */
    inThisRoutine = TRUE ;

    G_formScript = ScriptLock(uiFormNumber) ;
    DebugCheck(G_formScript != SCRIPT_BAD) ;

    if (G_formScript != SCRIPT_BAD)  {
        /* This module is now active. */
        G_scriptCommActive = TRUE ;

        sprintf(formName, "UI%06ld.FRM", uiFormNumber) ;

        /* load the form for this page */
        FormLoadFromFile (formName);

        /* set the form callback routine to MainUIControl */
        FormSetCallbackRoutine (ScriptFormCallback);

        /* double buffer drawing */
        GraphicUpdateAllGraphicsBuffered ();

        /* Set up the current form. */
        G_currentFormNumber = uiFormNumber ;

        /* go into generic control loop */
     //  FormGenericControl(&G_mainExit);

        inThisRoutine = FALSE ;

        G_oldKeyboardHandler = KeyboardGetEventHandler ();
        G_oldMouseHandler = MouseGetEventHandler ();

        MouseSetEventHandler    (FormHandleMouse);
        KeyboardSetEventHandler (FormHandleKey);

        /* Initialize the script for the first time.*/
        ScriptEvent(
            G_formScript,
            SCRIPT_EVENT_INITIALIZE,
            SCRIPT_DATA_TYPE_NONE,
            NULL,
            SCRIPT_DATA_TYPE_NONE,
            NULL,
            SCRIPT_DATA_TYPE_NONE,
            NULL) ;

        KeyboardBufferOn() ;
    }
ColorUpdate(0) ;

    DebugEnd();
}


/****************************************************************************/
/*  Routine:  ScriptFormEnd                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScriptFormEnd ends a session between UI and comm and closes out the    */
/*  form and all related functionality.                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/13/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScriptFormEnd(T_void)
{
    DebugRoutine("ScriptFormEnd") ;
    DebugCheck(G_scriptCommActive == TRUE) ;

    /* Get rid of the current form. */
	FormCleanUp();

    /* Release the script. */
    ScriptUnlock(G_formScript) ;
    G_formScript = SCRIPT_BAD ;

    /* Note that we don't have a form to work with any more. */
    G_scriptCommActive = FALSE ;

    MouseSetEventHandler(G_oldMouseHandler);
    KeyboardSetEventHandler(G_oldKeyboardHandler);

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ScriptFormCallback                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScriptFormCallback is called each time an action occurs with the       */
/*  current form.  This routine filters the data and passes it on up to the */
/*  the server for evaluation.                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.   NOTE:  Not all ui messages are sent to the server.            */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_formObjectType objtype    -- Type of object causing event           */
/*                                                                          */
/*    T_word16 objstatus          -- State of the object (state depends on  */
/*                                   objtype).                              */
/*                                                                          */
/*    T_word32 objID              -- ID of particular object                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/13/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScriptFormCallback(
           E_formObjectType objtype,
		   T_word16 objstatus,
		   T_word32 objID)
{
    T_sword32 id ;
    T_sword32 selection ;

    DebugRoutine ("ScriptFormCallback");

    id = objID ;
    switch(objtype)  {
        case FORM_OBJECT_BUTTON:
        case FORM_OBJECT_TEXTBUTTON:
            switch(objstatus)  {
                case BUTTON_ACTION_RELEASED:
                    IScriptFormEventOneNumber(
                        SCRIPT_EVENT_BUTTON_RELEASED,
                        id) ;
                    break ;
                case BUTTON_ACTION_PUSHED:
                    IScriptFormEventOneNumber(
                        SCRIPT_EVENT_BUTTON_PUSHED,
                        id) ;
                    break ;
            }
            break ;
        case FORM_OBJECT_TEXTBOX:
            switch(objstatus)  {
                case Txtbox_ACTION_GAINED_FOCUS:
                    IScriptFormEventOneNumber(
                        SCRIPT_EVENT_TEXT_BOX_GAINED_FOCUS,
                        id) ;
                    break ;
                case Txtbox_ACTION_LOST_FOCUS:
                    IScriptFormEventOneNumber(
                        SCRIPT_EVENT_TEXT_BOX_LOST_FOCUS,
                        id) ;
                    break ;
                case Txtbox_ACTION_ACCEPTED:
                    IScriptFormEventOneNumber(
                        SCRIPT_EVENT_TEXT_BOX_ACCEPTED,
                        id) ;
                    break ;
                case Txtbox_ACTION_DATA_CHANGED:
                    IScriptFormEventOneNumber(
                        SCRIPT_EVENT_TEXT_BOX_DATA_CHANGED,
                        id) ;
                    break ;
                case Txtbox_ACTION_SELECTION_CHANGED:
                    /* Get what is the new selection. */
                    selection = TxtboxGetSelectionNumber(FormGetObjID(id)) ;

                    /* Send this event to the script. */
                    IScriptFormEventTwoNumbers(
                        SCRIPT_EVENT_TEXT_BOX_SELECTION_CHANGED,
                        id,
                        selection) ;
                    break ;
            }
            break ;
    }
//printf(">%d %d %d\n", objstatus, objtype, objID) ;

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  ScriptFormUpdate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScriptFormUpdate is called as much as possible to update all the       */
/*  events necessary for doing UI via the comm port.                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ScriptFormUpdate                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/13/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScriptFormUpdate(T_void)
{
    DebugRoutine("ScriptFormUpdate") ;
    DebugCheck(G_scriptCommActive == TRUE) ;

    GraphicUpdateAllGraphics() ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  IScriptFormEventOneNumber          * INTERNAL *                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IScriptFormEventOneNumber is a "macro" routine to send the currnt     */
/*  script form an event with one number being passed.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 eventNumber        -- Event to send                          */
/*                                                                          */
/*    T_sword32 num               -- Number to send with event              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ScriptEvent                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/16/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IScriptFormEventOneNumber(T_word16 eventNumber, T_sword32 num)
{
     DebugRoutine("ScriptFormEventOneNumberParm") ;
     DebugCheck(eventNumber < SCRIPT_EVENT_UNKNOWN) ;

     ScriptEvent(
         G_formScript,
         eventNumber,
         SCRIPT_DATA_TYPE_32_BIT_NUMBER,
         (T_void *)num,
         SCRIPT_DATA_TYPE_NONE,
         NULL,
         SCRIPT_DATA_TYPE_NONE,
         NULL) ;

     DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IScriptFormEventTwoNumbers         * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IScriptFormEventTwoNumbers is a "macro" routine to send the current   */
/*  script form an event with two numbers being passed.                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 eventNumber        -- Event to send                          */
/*                                                                          */
/*    T_sword32 num               -- Number to send with event              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ScriptEvent                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/16/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IScriptFormEventTwoNumbers(
                  T_word16 eventNumber,
                  T_sword32 num1,
                  T_sword32 num2)
{
     DebugRoutine("IScriptFormEventTwoNumbers") ;
     DebugCheck(eventNumber < SCRIPT_EVENT_UNKNOWN) ;

     ScriptEvent(
         G_formScript,
         eventNumber,
         SCRIPT_DATA_TYPE_32_BIT_NUMBER,
         (T_void *)num1,
         SCRIPT_DATA_TYPE_32_BIT_NUMBER,
         (T_void *)num2,
         SCRIPT_DATA_TYPE_NONE,
         NULL) ;

     DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ScriptFormTextBoxSetSelection      * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ScriptFormTextBoxSetSelection moves the cursor on a ui text box       */
/*  to the given row -- for selection boxes, this causes the nth item to    */
/*  be selected.                                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 id                 -- text box id number                     */
/*                                                                          */
/*    T_word16 selection          -- Selection number/row                   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FormGetObjID                                                          */
/*    TxtboxCursSetRow                                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/16/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ScriptFormTextBoxSetSelection(
           T_word16 id,
           T_word16 selection)
{
    T_TxtboxID textBoxID ;

    DebugRoutine("ScriptFormTextBoxSetSelection") ;
    DebugCheck(G_formScript != SCRIPT_BAD) ;

    textBoxID = FormGetObjID(id) ;
    TxtboxCursSetRow(textBoxID, selection) ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE: SCRFORM.C                                                */
/****************************************************************************/
