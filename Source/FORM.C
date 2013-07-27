/****************************************************************************/
/*    FILE:  FORM.C                                                         */
/****************************************************************************/
#include "COLOR.H"
#include "DBLLINK.H"
#include "FILE.H"
#include "FORM.H"
#include "KEYMAP.H"
#include "KEYSCAN.H"
#include "MEMORY.H"
#include "MESSAGE.H"
#include "PICS.H"
#include "SOUND.H"
#include "TICKER.H"

static T_formCallBackRoutine formcallback = NULL;
static T_formObjectID G_formObjectArray[MAX_FORM_OBJECTS];
static E_Boolean G_formHasTextBoxes = FALSE;
static E_Boolean G_formHasButtons = FALSE;

static T_doubleLinkList G_formStack = DOUBLE_LINK_LIST_BAD;

/* prototype internal routines */
static T_formObjectID FormCreateObject(
        E_formObjectType type,
        T_formObjectID IDobj,
        T_word32 IDnum);

static T_formObjectID FormCreateObject(
        E_formObjectType type,
        T_formObjectID IDobj,
        T_word32 IDnum)
{
    T_word32 size;
    T_formObjectStruct *myID;

    DebugRoutine("FormCreateObject");

    size = sizeof(T_formObjectStruct);
    /* allocate memory for a new object structure */
    myID = (T_formObjectID)MemAlloc(size);
    DebugCheck(myID!=NULL);
    if (myID != NULL ) {
        /* set the object type */
        myID->objtype = type;
        /* set the pointer to the object */
        myID->objID = IDobj;
        /* set the status to default */
        myID->status = 0;
        /* set the numerical ID for this object */
        myID->numID = IDnum;
    }

    DebugEnd();
    /* return the object pointer */

    return (myID);
}

T_formObjectID FormAddButton(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *picturename,
        E_Boolean toggletype,
        T_word16 hotkey,
        T_word32 idnum)
{
    T_word16 i;
    T_buttonID buttonID;

    DebugRoutine("FormAddButton");
    DebugCheck(picturename!=NULL);

    G_formHasButtons = TRUE;
    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new button */
            buttonID = ButtonCreate(x1, y1, picturename, toggletype, hotkey,
                    FormReportButton, FormReportButton);
            /* now that a button has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_BUTTON,
                    (T_formObjectID)buttonID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugCheck(buttonID != NULL);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_formObjectID FormAddTextButton(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *data,
        T_byte8 *picturename,
        T_byte8 *fontname,
        T_byte8 fcolor,
        T_byte8 bcolor,
        E_Boolean toggletype,
        T_word16 hotkey,
        T_word32 idnum)
{
    T_word16 i;
    T_buttonID buttonID;

    DebugRoutine("FormAddTextButton");
    DebugCheck(picturename!=NULL);

    G_formHasButtons = TRUE;
    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new button */
            buttonID = ButtonCreate(x1, y1, picturename, toggletype, hotkey,
                    FormReportButton, FormReportButton );
            ButtonSetFont(buttonID, fontname);
            ButtonSetText(buttonID, data, fcolor);
            /* now that a button has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_BUTTON,
                    (T_formObjectID)buttonID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_formObjectID FormAddText(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *data,
        T_byte8 *fontname,
        T_byte8 fcolor,
        T_byte8 bcolor,
        T_word32 idnum)
{
    T_word16 i;
    T_textID textID;

    DebugRoutine("FormAddText");
    DebugCheck(fontname!=NULL);
    DebugCheck(data!=NULL);

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new button */
            textID = TextCreate(x1, y1, data);
            TextSetFont(textID, fontname);
            TextSetColor(textID, fcolor, bcolor);
            /* now that a button has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_TEXT,
                    (T_formObjectID)textID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_formObjectID FormAddGraphic(
        T_word16 x1,
        T_word16 y1,
        T_byte8 *picturename,
        T_word32 idnum)
{
    T_word16 i;
    T_graphicID graphicID;

    DebugRoutine("FormAddGraphic");
    DebugCheck(picturename!=NULL);

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new graphic */
            graphicID = GraphicCreate(x1, y1, picturename);
            /* now that a graphic has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_GRAPHIC,
                    (T_formObjectID)graphicID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_formObjectID FormAddSlider(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word32 idnum)
{
    T_word16 i;
    T_sliderID sliderID;

    DebugRoutine("FormAddSlider");

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new slider */
            sliderID = SliderCreate(x1, y1, x2);

            /* now that a slider has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_SLIDER,
                    (T_formObjectID)sliderID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_formObjectID FormAddTextField(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word16 y2,
        T_byte8 *fontname,
        T_byte8 maxfieldlength,
        E_TxtfldDataType datatype,
        T_word16 hotkey,
        T_word32 idnum)
{
    T_word16 i;
    T_TxtfldID TxtfldID;

    DebugRoutine("FormAddTextField");

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new textfield */
            TxtfldID = TxtfldCreate(x1, y1, x2, y2, fontname, maxfieldlength,
                    hotkey, datatype, FormReportField, FormReportField);
            /* now that a textfield has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_TEXTFIELD,
                    (T_formObjectID)TxtfldID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_formObjectID FormAddTextBox(
        T_word16 x1,
        T_word16 y1,
        T_word16 x2,
        T_word16 y2,
        T_byte8 *fontname,
        T_word32 maxlength,
        T_byte8 hotkey,
        E_Boolean numericonly,
        E_TxtboxJustify justify,
        E_TxtboxMode boxmode,
        T_word32 idnum)
{
    T_word16 i;
    T_TxtboxID TxtboxID;

    DebugRoutine("FormAddTextBox");
    DebugCheck(fontname != NULL);
    DebugCheck(maxlength > 0);
    DebugCheck(justify < Txtbox_JUSTIFY_UNKNOWN);
    DebugCheck(boxmode < Txtbox_MODE_UNKNOWN);

    G_formHasTextBoxes = TRUE;

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        /* find an empty slot */
        if (G_formObjectArray[i] == NULL ) {
            /* found one, create a new textform */
            TxtboxID = TxtboxCreate(x1, y1, x2, y2, fontname, maxlength, hotkey,
                    numericonly, justify, boxmode, FormReportTextBox);

            TxtboxSetCallback(TxtboxID, FormReportTextBox);
            /* now that a textform has been created, make an objstruct for it */
            G_formObjectArray[i] = FormCreateObject(FORM_OBJECT_TEXTBOX,
                    (T_formObjectID)TxtboxID, idnum);
            /* we made a new object struct, break from the loop */
            break;
        }
    }

    /* make sure we haven't exceeded any limits */
    DebugCheck(i!=MAX_FORM_OBJECTS);
    DebugEnd();
    /* return the ID for the object created */
    return (G_formObjectArray[i]);
}

T_void FormReportButton(T_buttonID buttonID)
{
    T_buttonStruct *p_button;
    T_formObjectStruct *p_object;
    T_word16 i;
    E_buttonAction buttonAction;

    DebugRoutine("FormReportButton");
    DebugCheck(buttonID != NULL);

    p_button = (T_buttonStruct *)buttonID;
    buttonAction = ButtonGetAction();

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        p_object = (T_formObjectStruct*)G_formObjectArray[i];
        if (p_object->objtype == FORM_OBJECT_BUTTON) {
            if ((T_buttonID)p_object->objID == buttonID) {
                /* found the button, call the callback routine */
                if (formcallback != NULL && buttonAction != 0) {
                    formcallback(FORM_OBJECT_BUTTON, buttonAction,
                            p_object->numID);
                    /* leave the loop */
                    break;
                }
            }
        }
    }

    DebugEnd();
}

T_void FormReportSlider(T_sliderID sliderID)
{
    T_sliderStruct *p_slider;
    T_formObjectStruct *p_object;
    T_word16 i;

    DebugRoutine("FormReportSlider");
    DebugCheck(sliderID != NULL);

    p_slider = (T_sliderStruct *)sliderID;

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        p_object = (T_formObjectStruct*)G_formObjectArray[i];
        if (p_object->objtype == FORM_OBJECT_SLIDER) {
            if ((T_sliderID)p_object->objID == sliderID) {
                /* found the button, call the callback routine */
                if (formcallback != NULL ) {
                    formcallback(FORM_OBJECT_SLIDER, 1, p_object->numID);
                    /* leave the loop */
                    break;
                }
            }
        }
    }

    DebugEnd();
}

T_void FormReportField(T_TxtfldID TxtfldID)
{
    T_TxtfldStruct *p_txtfld;

    DebugRoutine("FormReportField");

    p_txtfld = (T_TxtfldStruct*)TxtfldID;

    TxtfldNextField();

    DebugEnd();
}

T_void FormReportTextBox(T_TxtboxID TxtboxID)
{
    T_TxtboxStruct *p_Txtbox;
    T_formObjectStruct *p_object;
    E_TxtboxAction action;
    T_word16 i;

    DebugRoutine("FormReportBox");
    DebugCheck(TxtboxID != NULL);

    p_Txtbox = (T_TxtboxStruct*)TxtboxID;
    action = TxtboxGetAction();

    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        p_object = (T_formObjectStruct*)G_formObjectArray[i];
        if (!p_object)
            continue;
        if (p_object->objtype == FORM_OBJECT_TEXTBOX) {
            if ((T_TxtboxID)p_object->objID == TxtboxID) {
                /* found the button, call the callback routine */
                if (formcallback != NULL && action != 0) {
                    formcallback(FORM_OBJECT_TEXTBOX, action, p_object->numID);
                    /* leave the loop */
                    break;
                }
            }
        }
    }
    DebugEnd();
}

T_void FormLoadFromFile(T_byte8 *filename)
{
    FILE *fp;
    T_word16 i;
    T_word16 objtype = 0, objid, x1, y1, x2, y2;
    T_word16 hotkey, toggletype, fieldtype, fcolor, bcolor, justify;
    T_word16 sbupID, sbdnID, sbgrID;
    T_word16 numericonly;
    T_word32 maxlength;
    T_byte8 picname[32];
    T_byte8 fontname[32];
    T_byte8 tempstr[256];
    T_byte8 tempstr2[256];
    E_Boolean isincludedfile = FALSE;
    E_Boolean appendtext = FALSE;
    E_Boolean cursorset = FALSE;
    T_formObjectID objID;
    T_formObjectStruct *p_obj;
    T_buttonID SBUbuttonID, SBDbuttonID;
    T_graphicID SBGgraphicID;
    T_byte8 *p_includedtext;
    T_word32 size;

    DebugRoutine("FormLoadFromFile");
    DebugCheck(filename!=NULL);

    /* first, clean up the form structure and delete any previous forms */
    FormCleanUp();

    /* open up the file */
    fp = fopen(filename, "r");
    DebugCheck(fp!=NULL);
    while (feof(fp) == FALSE) {
        objtype = 0;
        /* get a line from the main file */
        fgets(tempstr, 128, fp);
        /* strip last (newline) character */
        if (tempstr[strlen(tempstr) - 1] == '\n')
            tempstr[strlen(tempstr) - 1] = '\0';

        /* append text to current object if flag is set */
        if (appendtext == TRUE) {
            if (strcmp(tempstr, "ENDOFTEXT") == 0) {
                /* turn off appendstring mode */
                TxtboxBackSpace(p_obj->objID);
                TxtboxCursTop(p_obj->objID);
                TxtboxRepaginate(p_obj->objID);
                TxtboxFirstBox();
                appendtext = FALSE;
                sprintf(tempstr, "#");
            } else if (tempstr[0] != '$' && tempstr[0] != '#') {
                /* strip last character if newline */
                if (tempstr[strlen(tempstr) - 1] == '\n')
                    tempstr[strlen(tempstr) - 1] = '\0';
                TxtboxAppendString(p_obj->objID, tempstr);
                TxtboxAppendKey(p_obj->objID, 13);
                sprintf(tempstr, "#");
            }
        }

        /* check to see if we should open an included file */
        if (tempstr[0] == '$') {
            /* strip the '$' from the string */
            for (i = 1; i < strlen(tempstr); i++)
                tempstr2[i - 1] = tempstr[i];
            tempstr2[i - 1] = '\0';
            /* open an included file */
            p_includedtext = FileLoad(tempstr2, &size);
            TxtboxSetData(p_obj->objID, p_includedtext);
            TxtboxRepaginateAll(p_obj->objID);
            TxtboxCursTop(p_obj->objID);
            MemFree(p_includedtext);

//        	fp2 = fopen (tempstr2,"r");
//	        DebugCheck (fp2!=NULL);
//           isincludedfile=TRUE;

            sprintf(tempstr, "#");
        }

        /* ignore comments and blank lines */
        if (tempstr[0] != '#' && tempstr[0] != ' ') {
            sscanf(tempstr, "%d", &objtype);
            if (objtype == 1) /* add a graphic */
            {
                sscanf(tempstr, "%d,%d,%d,%d,%s", &objtype, &objid, &x1, &y1,
                        picname);
                FormAddGraphic(x1, y1, picname, objid);
            } else if (objtype == 2) /* add a text */
            {
                sscanf(tempstr, "%d,%d,%d,%d,%d,%d", &objtype, &objid, &x1, &y1,
                        &fcolor, &bcolor);
                /* get font name */
                fgets(tempstr, 128, fp);
                sscanf(tempstr, "%s", fontname);
                /* get text */
                fgets(tempstr, 128, fp);
                /* strip last (newline) character */
                if (tempstr[strlen(tempstr) - 1] == '\n')
                    tempstr[strlen(tempstr) - 1] = '\0';
                /* add a text object */
                FormAddText(x1, y1, tempstr, fontname, (T_byte8)fcolor, (T_byte8)bcolor, objid);
            } else if (objtype == 3) /* add a button */
            {
                sscanf(tempstr, "%d,%d,%d,%d,%d,%d,%s", &objtype, &objid, &x1,
                        &y1, &toggletype, &hotkey, picname);
                FormAddButton(x1, y1, picname, (E_Boolean)toggletype, hotkey,
                        objid);
            } else if (objtype == 4) /* add a text button */
            {
                sscanf(tempstr, "%d,%d,%d,%d,%d,%d,%d", &objtype, &objid, &x1, &y1,
                        &fcolor, &toggletype, &hotkey);
                /* get picture name */
                fgets(tempstr, 128, fp);
                sscanf(tempstr, "%s", picname);
                /* get font name */
                fgets(tempstr, 128, fp);
                sscanf(tempstr, "%s", fontname);
                /* get buttontext */
                fgets(tempstr, 128, fp);
                /* strip last (newline) character */
                if (tempstr[strlen(tempstr) - 1] == '\n')
                    tempstr[strlen(tempstr) - 1] = '\0';
                /* make a text button */
                FormAddTextButton(x1, y1, tempstr, picname, fontname, (T_byte8)fcolor, 0,
                        (E_Boolean)toggletype, hotkey, objid);
            } else if (objtype == 5) /* add a text box */
            {
                sscanf(tempstr, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s",
                        &objtype, &objid, &x1, &y1, &x2, &y2, &maxlength,
                        &numericonly, &justify, &fieldtype, &hotkey, &sbupID,
                        &sbdnID, &sbgrID, fontname);

                /* read in default text */
//                fgets (tempstr,128,fp);
                /* create a text box */
                /* set maximum length to highest possible value if 0 */
                if (maxlength == 0)
                    maxlength--;

                objID = FormAddTextBox(
							x1, 
							y1, 
							x2, 
							y2, 
							fontname, 
							maxlength,
							(T_byte8)hotkey, 
							(T_byte8)numericonly, 
							justify, 
							(E_TxtboxMode)fieldtype,
							objid);

                DebugCheck(objID != NULL);
                /* set form scroll bar stuff */
                if (sbupID != 0) {
                    SBUbuttonID = FormGetObjID(sbupID);
                    ButtonSetData(SBUbuttonID, objid);
                    ButtonSetCallbacks(SBUbuttonID, NULL, TxtboxHandleSBUp);
                    SBDbuttonID = FormGetObjID(sbdnID);
                    ButtonSetData(SBDbuttonID, objid);
                    ButtonSetCallbacks(SBDbuttonID, NULL, TxtboxHandleSBDn);
                    SBGgraphicID = FormGetObjID(sbgrID);
                    DebugCheck(SBUbuttonID != NULL);
                    DebugCheck(SBDbuttonID != NULL);
                    DebugCheck(SBGgraphicID != NULL);
                    p_obj = (T_formObjectStruct*)objID;
                    TxtboxSetScrollBarObjIDs(p_obj->objID, SBUbuttonID,
                            SBDbuttonID, SBGgraphicID);
                }

                /* set default text */
                p_obj = (T_formObjectStruct *)objID;
                appendtext = TRUE;

            } else if (objtype == 6) /* add a slider */
            {
                sscanf(tempstr, "%d,%d,%d,%d,%d", &objtype, &objid, &x1, &y1,
                        &x2);
                objID = FormAddSlider(x1, y1, x2, objid);
                DebugCheck(objID != NULL);
                p_obj = (T_formObjectStruct *)objID;
                SliderSetCallBack(p_obj->objID, FormReportSlider);
            }
        }
    }
    fclose(fp);
    DebugEnd();
}

T_void FormDeleteObject(T_formObjectID objectID)
{
    DebugRoutine("FormDeleteObject");

    DebugEnd();
}

T_void FormCleanUp(T_void)
{
    T_word16 i;
    T_formObjectStruct *p_object;
    E_Boolean UScrewedUpSomewhere = FALSE;

    DebugRoutine("FormCleanUp");

    /* update the graphics */
    GraphicUpdateAllGraphics();

    /* loop through all objects, deleting structures if available */
    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        if (G_formObjectArray[i] != NULL ) {
            p_object = (T_formObjectStruct *)G_formObjectArray[i];
            /* first delete the object (button, textfield, ect) */
            DebugCheck(p_object != NULL);
            switch (p_object->objtype) {
                case FORM_OBJECT_BUTTON:
                    ButtonDelete((T_buttonID)p_object->objID);
                    break;
                case FORM_OBJECT_TEXT:
                    TextDelete((T_textID)p_object->objID);
                    break;
                case FORM_OBJECT_TEXTBUTTON:
                    ButtonDelete((T_buttonID)p_object->objID);
                    break;
                case FORM_OBJECT_GRAPHIC:
                    GraphicDelete((T_graphicID)p_object->objID);
                    break;
                case FORM_OBJECT_TEXTFIELD:
                    TxtfldDelete((T_TxtfldID)p_object->objID);
                    break;
                case FORM_OBJECT_TEXTBOX:
                    TxtboxDelete((T_TxtboxID)p_object->objID);
                    break;
                case FORM_OBJECT_SLIDER:
                    SliderDelete((T_sliderID)p_object->objID);
                    break;
                default:
                    /* something is wrong! */
                    DebugCheck(UScrewedUpSomewhere==TRUE);
                    break;
            }

            /* now delete the object structure */
            MemFree(G_formObjectArray[i]);
            MemCheck(101);
            G_formObjectArray[i] = NULL;
        }
    }

    G_formHasTextBoxes = FALSE;
    G_formHasButtons = FALSE;
    DebugEnd();
}

T_void FormHandleMouse(
        E_mouseEvent event,
        T_word16 x,
        T_word16 y,
        E_Boolean button)
{
    DebugRoutine("FormHandleMouse");
    /* send event to buttons */
    ButtonMouseControl(event, x, y, button);
    /* send event to sliders */
    SliderMouseControl(event, x, y, button);
    /* send event to text boxes (if there are any) */
    if (G_formHasTextBoxes == TRUE)
        TxtboxMouseControl(event, x, y, button);

    DebugEnd();
}

T_void FormHandleKey(E_keyboardEvent event, T_word16 scankey)
{
    static E_Boolean wasGamma = FALSE;

    DebugRoutine("FormHandleKey");
    if (G_formHasButtons == TRUE)
        ButtonKeyControl(event, scankey);
    if (G_formHasTextBoxes == TRUE)
        TxtboxKeyControl(event, scankey);

    if (KeyboardGetScanCode(KEY_SCAN_CODE_ALT) == TRUE) {
        if (KeyMapGetScan(KEYMAP_GAMMA_CORRECT) == TRUE)  {
            if (wasGamma == FALSE) {
                MessagePrintf("Gamma level %d",
                    ColorGammaAdjust()) ;
                ColorUpdate(1) ;
                wasGamma = TRUE;
            }
        } else {
            wasGamma = FALSE;
        }
    } else {
        wasGamma = FALSE;
    }

    DebugEnd();
}

T_void FormSetCallbackRoutine(T_formCallBackRoutine newcallback)
{
    DebugRoutine("FormSetCallbackRoutine");

//	DebugCheck (newcallback!=NULL);
    formcallback = newcallback;

    DebugEnd();
}

// Get a form object ID (or not, if doesn't exist)
T_formObjectID FormFindObjID(T_word32 numID)
{
    T_word16 i;
    T_formObjectID retvalue = NULL;
    T_formObjectStruct *p_object;

    DebugRoutine("FormFindObjID");
    for (i = 0; i < MAX_FORM_OBJECTS; i++) {
        if (G_formObjectArray[i] != NULL ) {
            /* get the pointer to the object structure */
            p_object = (T_formObjectStruct*)G_formObjectArray[i];
            /* check to see if the numerical ID matches */
            if (numID == p_object->numID) {
                /* found one, return the pointer to the object in question */
                retvalue = p_object->objID;
                break;
            }
        }
    }
    DebugEnd();
    return (retvalue);
}

T_formObjectID FormGetObjID(T_word32 numID)
{
    T_formObjectID retvalue = NULL;

    DebugRoutine("FormGetObjID");
    retvalue = FormFindObjID(numID);
#ifndef NDEBUG
    if (retvalue == NULL ) {
        printf("bad form obj ID = %d\n", numID);
        fflush(stdout);
    }
#endif
    DebugCheck(retvalue != NULL);
    DebugEnd();
    return (retvalue);
}

T_void FormGenericControl(E_Boolean *exitflag)
{
    static T_word32 delta = 0, lastupdate = 0;
    T_keyboardEventHandler keyhandl;
    T_mouseEventHandler mousehandl;
    T_bitmap *p_bitmap;
    T_resource r_bitmap;
    T_bitmap *p_oldBitmap;
    T_word16 hotX, hotY;

    DebugRoutine("FormGenericControl");

    lastupdate = TickerGet();
    *exitflag = FALSE;

    /** Initialize the mouse. **/
    MouseGetBitmap(&hotX, &hotY, &p_oldBitmap);
    p_bitmap = (T_bitmap *)PictureLock("UI/MOUSE/DEFAULT", &r_bitmap);
    DebugCheck(p_bitmap != NULL);
    MouseSetDefaultBitmap(0, 0, p_bitmap);
    MouseUseDefaultBitmap();

    /* show the mouse and set the keyboard/mouse event handlers */
    keyhandl = KeyboardGetEventHandler();
    mousehandl = MouseGetEventHandler();
    /* flush the keyboard */
    KeyboardDebounce();
    MouseSetEventHandler(FormHandleMouse);
    KeyboardSetEventHandler(FormHandleKey);
//    MouseShow();

    do {
        delta = TickerGet();
        /* update color every 4 ticks */
        if ((delta - lastupdate) > 0) {
            lastupdate = delta;
            ColorUpdate(delta - lastupdate);
        }

        /* update events */
        GraphicUpdateAllGraphics();
//        MouseHide();
        MouseUpdateEvents();
        KeyboardUpdateEvents();
        SoundUpdate();
//        MouseShow();
//        delay (20);
    } while (*exitflag == FALSE
            && KeyboardGetScanCode(KEY_SCAN_CODE_ESC) == FALSE);

    /* clean up */
//    MouseHide();
    FormCleanUp();
    MouseSetEventHandler(mousehandl);
    KeyboardSetEventHandler(keyhandl);
    KeyboardDebounce();

    /** free up the mouse pointer resource. **/
    ResourceUnlock(r_bitmap);
    ResourceUnfind(r_bitmap);

    /* Turn off the mouse */
    MouseSetDefaultBitmap(hotX, hotY, p_oldBitmap);
    MouseUseDefaultBitmap();

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  FormGenericControlStart                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FormGenericControlStart sets up the form module for another form      */
/*  user interface by pushing the mouse and keyboard handlers.              */
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
/*    KeyboardDebounce                                                      */
/*    MousePushEventHandler                                                 */
/*    KeyboardPushEventHandler                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FormGenericControlStart(T_void)
{
    DebugRoutine("FormGenericControlStart");

    /* flush the keyboard */
    KeyboardDebounce();
    MousePushEventHandler(FormHandleMouse);
    KeyboardPushEventHandler(FormHandleKey);

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  FormGenericControlEnd                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FormGenericControlEnd cleans up the form module from the last         */
/*  user interface by popping the mouse and keyboard handlers.              */
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
/*    KeyboardDebounce                                                      */
/*    MousePushEventHandler                                                 */
/*    KeyboardPushEventHandler                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FormGenericControlEnd(T_void)
{
    DebugRoutine("FormGenericControlEnd");

    /* clean up */
    FormCleanUp();

    MousePopEventHandler();
    KeyboardPopEventHandler();

    KeyboardDebounce();

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  FormGenericControlUpdate                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FormGenericControlUpdates updates anything that is being processed    */
/*  with the current user interface.                                        */
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
/*    GraphicUpdateAllGraphics                                              */
/*    MouseUpdateEvents                                                     */
/*    KeyboardUpdateEvents                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FormGenericControlUpdate(T_void)
{
    DebugRoutine("FormGenericControlUpdate");

    GraphicUpdateAllGraphics();
    MouseUpdateEvents();
    KeyboardUpdateEvents();
    SoundUpdate();

    DebugEnd();
}

typedef struct {
    T_formCallBackRoutine callback;
    E_Boolean hasText;
    E_Boolean hasButtons;
} T_formVariousValues;

T_void FormPush(T_void)
{
    T_formObjectID *p_formObjs;
    T_void *p_state;
    T_formVariousValues *p_values;

    DebugRoutine("FormPush");

    /* Make sure we have a list. */
    if (G_formStack == DOUBLE_LINK_LIST_BAD)
        G_formStack = DoubleLinkListCreate();

    /* Create a list of form objects to put on the stack. */
    p_formObjs = MemAlloc(sizeof(G_formObjectArray));
    DebugCheck(p_formObjs != NULL);
    memcpy(p_formObjs, G_formObjectArray, sizeof(G_formObjectArray));
    DoubleLinkListAddElementAtFront(G_formStack, p_formObjs);

    /* Clear the list of form objects */
    memset(G_formObjectArray, 0, sizeof(G_formObjectArray));

    /* Put the buttons on the list. */
    p_state = ButtonGetStateBlock();
    DoubleLinkListAddElementAtFront(G_formStack, p_state);

    /* Put the graphics on the list. */
    p_state = GraphicGetStateBlock();
    DoubleLinkListAddElementAtFront(G_formStack, p_state);

    /* Put the sliders on the list. */
    p_state = SliderGetStateBlock();
    DoubleLinkListAddElementAtFront(G_formStack, p_state);

    /* Put the text boxes on the list. */
    p_state = TxtboxGetStateBlock();
    DoubleLinkListAddElementAtFront(G_formStack, p_state);

    /* Put the callback and other flags in the list */
    p_values = MemAlloc(sizeof(T_formVariousValues));
    p_values->callback = formcallback;
    p_values->hasText = G_formHasTextBoxes;
    p_values->hasButtons = G_formHasButtons;
    DoubleLinkListAddElementAtFront(G_formStack, p_values);

    DebugEnd();
}

T_void FormPop(T_void)
{
    T_formObjectID *p_formObjs;
    T_doubleLinkListElement element;
    T_void *p_state;
    T_formVariousValues *p_values;

    DebugRoutine("FormPop");

    // Remove the current form fully
    FormCleanUp();

    /* Recover the form callback */
    element = DoubleLinkListGetFirst(G_formStack);
    p_values = (T_formVariousValues *)DoubleLinkListElementGetData(element);
    formcallback = p_values->callback;
    G_formHasTextBoxes = p_values->hasText;
    G_formHasButtons = p_values->hasButtons;
    MemFree(p_values);
    DoubleLinkListRemoveElement(element);

    /* Recover the list of text boxes */
    element = DoubleLinkListGetFirst(G_formStack);
    p_state = DoubleLinkListElementGetData(element);
    TxtboxSetStateBlock(p_state);
    MemFree(p_state);
    DoubleLinkListRemoveElement(element);

    /* Recover the list of sliders */
    element = DoubleLinkListGetFirst(G_formStack);
    p_state = DoubleLinkListElementGetData(element);
    SliderSetStateBlock(p_state);
    MemFree(p_state);
    DoubleLinkListRemoveElement(element);

    /* Recover the list of graphics */
    element = DoubleLinkListGetFirst(G_formStack);
    p_state = DoubleLinkListElementGetData(element);
    GraphicSetStateBlock(p_state);
    MemFree(p_state);
    DoubleLinkListRemoveElement(element);

    /* Recover the list of buttons */
    element = DoubleLinkListGetFirst(G_formStack);
    p_state = DoubleLinkListElementGetData(element);
    ButtonSetStateBlock(p_state);
    MemFree(p_state);
    DoubleLinkListRemoveElement(element);

    /* Recover the list of form objects */
    element = DoubleLinkListGetFirst(G_formStack);
    p_formObjs = DoubleLinkListElementGetData(element);
    memcpy(G_formObjectArray, p_formObjs, sizeof(G_formObjectArray));
    MemFree(p_formObjs);
    DoubleLinkListRemoveElement(element);

    /* If the list is empty, destroy it. */
    if (DoubleLinkListGetNumberElements(G_formStack) == 0) {
        DoubleLinkListDestroy(G_formStack);
        G_formStack = DOUBLE_LINK_LIST_BAD;
    }

    DebugEnd();
}
