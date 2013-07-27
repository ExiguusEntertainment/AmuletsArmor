/****************************************************************************/
/*    FILE:  ConnUI.C                                                       */
/****************************************************************************/
#include "CONNUI.H"
#include "GENERAL.H"
#include "MEMORY.H"
#include "SMCONNEC.H"

#define CONNUI_CONNECT_BUTTON           302
#define CONNUI_EXIT_BUTTON              303
#define CONNUI_SELECTION_BOX            500
#define CONNUI_SERVER_NAME_TEXT         501
#define CONNUI_DIAL_TEXT                502
#define CONNUI_INIT_TEXT                503

static T_doubleLinkList G_connUIServerList;
static E_Boolean G_quitGame = FALSE;
static E_Boolean G_mainExit = FALSE;
static T_void ConnUIUpdate (T_void);
static T_byte8 G_currentModemInit[80]="ATZ";
static T_byte8 G_currentServerName[80]="";
static T_byte8 G_currentServerNumber[80]="";


/* internal prototype */
static T_doubleLinkListElement ConnUIGetActiveServerElement (T_void);
static T_void ConnUISaveServerList(T_void);
static E_Boolean ConnUIDestroyElement (T_doubleLinkListElement killme);
static T_void ConnUISaveCurrentRecord(T_void);


E_Boolean ConnMainUI (T_void)
{
    DebugRoutine ("ConnMainUI");

    /* load the conn main ui form */
    ConnMainUIInit ();
    FormGenericControl (&G_mainExit);

    /* here, save the new server list file */
    ConnUISaveServerList();

    /* now, get rid of the linked list */
    DoubleLinkListTraverse (G_connUIServerList,ConnUIDestroyElement);
    DoubleLinkListDestroy  (G_connUIServerList);


    DebugEnd();
    return (G_quitGame);
}


T_void ConnMainUIControl (E_formObjectType objtype,
					  T_word16 objstatus,
					  T_word32 objID)
{
    T_TxtboxID TxtboxID;
    T_connUIStruct server ;
    T_doubleLinkListElement element=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugRoutine ("ConnMainUIControl");

    if (objtype==FORM_OBJECT_BUTTON && objstatus==BUTTON_ACTION_RELEASED)
    {
        if (objID==CONNUI_CONNECT_BUTTON)
        {
            /* save current record info */
            ConnUISaveCurrentRecord();
            /* connect button selected */
            /* call dialin with init and dial strings */
//            G_mainExit = ClientDialIn (G_currentModemInit,G_currentServerNumber);

//            TxtboxID=FormGetObjID(CONNUI_INIT_TEXT);
//            strcpy (server.servinit,TxtboxGetData (TxtboxID));
//            TxtboxID=FormGetObjID(CONNUI_DIAL_TEXT);
//            strcpy (server.servphone,TxtboxGetData (TxtboxID));
//            TxtboxID=FormGetObjID(CONNUI_SERVER_NAME_TEXT);
//            strcpy (server.servname,TxtboxGetData (TxtboxID));
//          strcpy (server.servinit,G_currentModemInit);
            strcpy (server.servphone,G_currentServerNumber);
            strcpy (server.servname,G_currentServerName);
            SMClientConnectSetServerInfo(&server) ;
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_SERVER_SELECTED,
                TRUE) ;
            G_mainExit=TRUE;
        }
        else if (objID==CONNUI_EXIT_BUTTON)
        {
            /* exit button selected */
            G_quitGame=TRUE;
            G_mainExit=TRUE;
            SMClientConnectSetFlag(
                CLIENT_CONNECT_FLAG_EXIT,
                TRUE) ;
        }
        else if (objID==304)
        {
            /* save button selected */
            ConnUISaveCurrentRecord();
        }
        else if (objID==305)
        {
            /* delete button selected */
            /* get server element */
            element=ConnUIGetActiveServerElement();

            /* remove it from the list */
            if (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
            {
                ConnUIDestroyElement (element);
            }

            /* update list */
            ConnUIUpdate();
        }
    }
    else if (objtype==FORM_OBJECT_TEXTBOX)
    {
        if (objID==500)
        {
            /* selection box was changed */
            ConnUIUpdate();
        }
        else if (objID==501)
        {
            TxtboxID=FormGetObjID(501);
            strcpy (G_currentServerName,TxtboxGetData(TxtboxID));
        }
        else if (objID==502)
        {
            /* server number field changed */
            TxtboxID=FormGetObjID(502);
            strcpy (G_currentServerNumber,TxtboxGetData(TxtboxID));
        }
        else if (objID==503)
        {
            /* modem init string changed */

            /* update data */
            TxtboxID=FormGetObjID(503);
            strcpy (G_currentModemInit,TxtboxGetData(TxtboxID));
        }
    }

    DebugEnd();
}



T_void ConnMainUIInit (T_void)
{
    T_byte8 tempstr[60];
    T_byte8 name[64];
    T_byte8 phone[64];
    FILE *fp;
    T_connUIStruct *p_conn;

    DebugRoutine ("ConnMainUIInit");

    /* init string defaults */
    strcpy (name,"");
    strcpy (phone,"");

puts("Loading connect form") ;  fflush(stdout) ;
	/* load the form for this page */
	FormLoadFromFile ("CONNECT.FRM");

    /* initialize linked list */
    G_connUIServerList = DoubleLinkListCreate();
    DebugCheck (G_connUIServerList != DOUBLE_LINK_LIST_BAD);

    /* load the server list for this page */
    fp = fopen ("servlist.txt","r");
//    DebugCheck (fp != NULL);

    if (fp != NULL)
    {
        /* remove comment */
        fgets (tempstr,60,fp);
        /* get modem init string */
        fgets (tempstr,60,fp);
        memcpy (G_currentModemInit,tempstr+2,strlen(tempstr)-1);

        /* read in server list */
        while (feof(fp)==FALSE)
        {
            /* get a line */
            fgets (tempstr,60,fp);
            /* strip newline character if exists */
            if (tempstr[strlen(tempstr)-1]=='\n')  tempstr[strlen(tempstr)-1]='\0';
            /* ignore comments and blank lines */
            switch (tempstr[0])
            {
                case 'N':
                /* copy to name field */
                memcpy (name,tempstr+2,strlen(tempstr)-1);
                break;

                case 'P':
                /* copy to phone number field */
                memcpy (phone,tempstr+2,strlen(tempstr)-1);
                /* add field to linked list */
                p_conn=(T_connUIStruct *)MemAlloc(sizeof(T_connUIStruct));
                strcpy (p_conn->servname,name);
                strcpy (p_conn->servphone,phone);
                p_conn->This = DoubleLinkListAddElementAtEnd(G_connUIServerList,p_conn);
                DebugCheck (p_conn->This != DOUBLE_LINK_LIST_ELEMENT_BAD);
                break;
            }
        }

        /* close down the file */
        fclose (fp);
    }
	/* set the form callback routine to MainUIControl */
	FormSetCallbackRoutine (ConnMainUIControl);

    /* update the display */
    ConnUIUpdate();

    GraphicUpdateAllGraphicsBuffered ();
    DebugEnd();
}


static E_Boolean ConnUIDestroyElement (T_doubleLinkListElement killme)
{
    T_connUIStruct *p_conn;
    DebugRoutine ("ConnUIDestroyElement");

    /* get element data */
    p_conn=(T_connUIStruct *)DoubleLinkListRemoveElement (killme);

    /* kill element data */
    MemFree (p_conn);

    killme=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugEnd();

    return (TRUE);
}


static T_void ConnUIUpdate (T_void)
{
    T_byte8 *seldata;
    T_word32 datalen=0;
    T_TxtboxID TxtboxID;
    T_connUIStruct *p_conn;
    T_word16 selnum;
    T_doubleLinkListElement element=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugRoutine ("ConnUIUpdate");

    selnum=TxtboxGetSelectionNumber(FormGetObjID(500));

    /* get current server structure */
    element=ConnUIGetActiveServerElement();
    if (element==DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* clear out boxes to default */
        strcpy (G_currentServerName,"");
        strcpy (G_currentServerNumber,"");
    }
    else
    {
        /* get element data */
        p_conn=(T_connUIStruct *)DoubleLinkListElementGetData(element);
        strcpy (G_currentServerName,p_conn->servname);
        strcpy (G_currentServerNumber,p_conn->servphone);
    }

    /* copy data into text boxes */
    /* copy name field */
    TxtboxID=FormGetObjID(501);
    TxtboxSetData (TxtboxID,G_currentServerName);
    /* copy phone field */
    TxtboxID=FormGetObjID(502);
    TxtboxSetData (TxtboxID,G_currentServerNumber);
    /* copy modem init field */
    TxtboxID=FormGetObjID(503);
    TxtboxSetData (TxtboxID,G_currentModemInit);

    /* clean out current selection list */
    TxtboxID=FormGetObjID(500);
    TxtboxSetData(TxtboxID,"");
    TxtboxCursTop (TxtboxID);

    /* create selection list of servers */
    element=DoubleLinkListGetFirst(G_connUIServerList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_conn=(T_connUIStruct *)DoubleLinkListElementGetData(element);
        datalen+=strlen(p_conn->servname)+2;
        element=DoubleLinkListElementGetNext(element);
    }
    seldata=MemAlloc (datalen);
    strcpy (seldata,"");

    element=DoubleLinkListGetFirst(G_connUIServerList);
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_conn=(T_connUIStruct *)DoubleLinkListElementGetData(element);
        sprintf (seldata,"%s%s\r",seldata,p_conn->servname);
        element=DoubleLinkListElementGetNext(element);
    }

    TxtboxSetData (TxtboxID,seldata);
    MemFree (seldata);
    TxtboxCursSetRow (TxtboxID,selnum);

    DebugEnd();
}


/* routine returns pointer to 'selected' server structure */
static T_doubleLinkListElement ConnUIGetActiveServerElement (T_void)
{
    T_TxtboxID *TxtboxID;
    T_word16 selectnumber;
    T_word16 cnt;
    T_doubleLinkListElement element=DOUBLE_LINK_LIST_ELEMENT_BAD;
    T_doubleLinkListElement retvalue=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugRoutine ("ConnUIGetActiveServerElement");

    /* get selection number */
    TxtboxID=FormGetObjID(500);
    DebugCheck (TxtboxID != NULL);

    /* get current selection number of textbox */
    selectnumber=TxtboxGetSelectionNumber (TxtboxID);
    cnt=0;

    /* get list of server names */
    element=DoubleLinkListGetFirst (G_connUIServerList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* if cnt == selection number then we are looking */
        /* at the correct element */

        if (cnt==selectnumber)
        {
            /* correct element, get pointer to data and retuen */
            retvalue=element;
            break;
        }
        else
        {
            cnt++;
            element=DoubleLinkListElementGetNext (element);
        }
    }

    DebugEnd();

    return (retvalue);
}


/* writes out current server list to SERVLST.TXT*/
static T_void ConnUISaveServerList(T_void)
{
    FILE *fp;
    T_byte8 stmp[80];
    T_connUIStruct *p_conn;
    T_doubleLinkListElement element;

    DebugRoutine ("ConnUISaveServerList");
    fp = fopen ("servlist.txt","w");
    DebugCheck (fp != NULL);

    /* put comment */
    fputs ("# server definintion file\n",fp);

    /* put modem init string */
    sprintf (stmp,"I:%s\n",G_currentModemInit);
    fputs (stmp,fp);

    /* scan through all server entries and place data */
    element=DoubleLinkListGetFirst (G_connUIServerList);

    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        p_conn=NULL;
        /* get this server data */
        p_conn=(T_connUIStruct *)DoubleLinkListElementGetData(element);

        DebugCheck (p_conn != NULL);
        /* write name */
        sprintf (stmp,"N:%s\n",p_conn->servname);
        fputs (stmp,fp);

        /* write number */
        sprintf (stmp,"P:%s\n",p_conn->servphone);
        fputs (stmp,fp);

        element=DoubleLinkListElementGetNext(element);
    }

    /* write end comment */
    fputs ("# end of file\n",fp);
    fclose (fp);

    DebugEnd();
}

/* routine saves current changes into element */
static T_void ConnUISaveCurrentRecord(T_void)
{
    T_connUIStruct *p_conn;
    T_doubleLinkListElement element=DOUBLE_LINK_LIST_ELEMENT_BAD;

    DebugRoutine ("ConnUISaveCurrentRecord");

    /* get active record */
    element=ConnUIGetActiveServerElement();

    if (element==DOUBLE_LINK_LIST_ELEMENT_BAD)
    {
        /* no record for this info, make one */
        p_conn=(T_connUIStruct *)MemAlloc(sizeof(T_connUIStruct));
        p_conn->This = DoubleLinkListAddElementAtEnd(G_connUIServerList,p_conn);
        element=p_conn->This;
        DebugCheck (p_conn->This != DOUBLE_LINK_LIST_ELEMENT_BAD);
    }
    else
    {
        /* get element info */
        p_conn=(T_connUIStruct *)DoubleLinkListElementGetData(element);
    }

    /* set new data */
    strcpy (p_conn->servname,G_currentServerName);
    strcpy (p_conn->servphone,G_currentServerNumber);

    /* update ui screens */
    ConnUIUpdate();

    DebugEnd();
}


/****************************************************************************/
/*  Routine:  ConnMainUIStart                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ConnMainUIStart starts up the connection ui screen.                   */
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
/*    ConnMainUIInit                                                        */
/*    FormGenericControlStart                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ConnMainUIStart(T_void)
{
    DebugRoutine ("ConnMainUIStart");

    /* load the conn main ui form */
    ConnMainUIInit();

    /* here, save the new server list file */
    FormGenericControlStart() ;

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  ConnMainUIEnd                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ConnMainUIEnd ends the connection ui screen.                          */
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
/*    DoubleLinkListTraverse                                                */
/*    DoubleLinkListDestroy                                                 */
/*    FormGenericControlEnd                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ConnMainUIEnd(T_void)
{
    DebugRoutine("ConnMainUIEnd") ;

    /* here, save the new server list file */
    ConnUISaveServerList();

    /* now, get rid of the linked list */
    DoubleLinkListTraverse(
        G_connUIServerList,
        ConnUIDestroyElement);

    DoubleLinkListDestroy(G_connUIServerList);

    FormGenericControlEnd() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ConnMainUIUpdate                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ConnMainUIUpdate updates the UI for the connection ui screen.         */
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
/*    FormGenericControlUpdate                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/28/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ConnMainUIUpdate(T_void)
{
    DebugRoutine("ConnMainUIUpdate") ;

    FormGenericControlUpdate();

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  ConnUI.C                                                */
/****************************************************************************/

