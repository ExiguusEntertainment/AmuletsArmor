/*-------------------------------------------------------------------------*
 * File:  COMWIN.C
 *-------------------------------------------------------------------------*/
/**
 * The Communications Window is the UI for sending fun sound clips or
 * text to other players in the area.  We originally thought it might be
 * good to make it so only players nearby can hear these sayings, but
 * later gave up on that.
 *
 * @addtogroup COMWIN
 * @brief Communications Window User Interface
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CLI_SEND.H"
#include "COMWIN.H"
#include "GENERAL.H"
#include "PLAYER.H"
#include "STATS.H"

#define NUM_CANNED_SAYINGS 40
#define NUM_SAMPLE_MAPS 4

static E_Boolean G_comwinIsOpen=FALSE;
static T_byte8 G_fielddata[300]="";
//static T_comwinCannedSayingStruct G_cannedSayings[MAX_CANNED_SAYINGS];
static T_byte8 *G_cannedSayingTexts[NUM_CANNED_SAYINGS] =
{
    "Come here!",
    "Follow me.",
    "Greetings.",
    "Help!",
    "Hey you!",
    "Thats nice.",
    "Sorry.",
    "Wait!",
    "Yeah, right.",

    "Ahem.",
    "I apologise.",
    "Come here!",
    "Death becomes you.",
    "I failed.",
    "Follow.",
    "Help me!",
    "Nothrak!",
    "Hmm.. Not bad.",
    "Salutations.",
    "I think not.",

    "Assistance!",
    "Come hither.",
    "Defend thyself!",
    "Halt!",
    "Hurrah.",
    "How goes there?",
    "You jest.",
    "Pardon me.",
    "To me!",
    "Who goes there?",

    "<belch>",
    "My bad.",
    "Come here!",
    "Come on!",
    "Fantastic.",
    "Help!",
    "Hey you!",
    "Whatr you lookin at?",
    "Stand fast.",
    "Yeah."
};

T_word16 G_sampleNumbers[NUM_CANNED_SAYINGS] =
{
    7008,
    7007,
    7006,
    7005,
    7002,
    7003,
    7004,
    7001,
    7000,

    7020,
    7019,
    7018,
    7017,
    7016,
    7015,
    7014,
    7013,
    7012,
    7011,
    7010,

    7030,
    7029,
    7028,
    7027,
    7026,
    7025,
    7024,
    7023,
    7022,
    7021,

    7040,
    7039,
    7038,
    7037,
    7036,
    7035,
    7034,
    7033,
    7032,
    7031
};

T_byte8 G_sampleSetMarkers[NUM_SAMPLE_MAPS+1] =
{
    9,20,30,40,50
};

E_comwinSampleType G_characterSampleMaps[CLASS_UNKNOWN] =
{
    COMWIN_SAMPLE_SET_CITIZEN,
    COMWIN_SAMPLE_SET_FIGHTER,
    COMWIN_SAMPLE_SET_WIZARD,
    COMWIN_SAMPLE_SET_WIZARD,
    COMWIN_SAMPLE_SET_CITIZEN,
    COMWIN_SAMPLE_SET_ROGUE,
    COMWIN_SAMPLE_SET_CITIZEN,
    COMWIN_SAMPLE_SET_ROGUE,
    COMWIN_SAMPLE_SET_FIGHTER,
    COMWIN_SAMPLE_SET_FIGHTER,
    COMWIN_SAMPLE_SET_WIZARD
};


T_void ComwinInitCommunicatePage(T_void)
{
    E_statsClassType playerclass;
    E_comwinSampleType sampleset;
    T_word16 start,end;
    T_word32 length=0;
    T_word16 i;

    DebugRoutine ("ComwinInitCommunicatePage");

    playerclass=StatsGetPlayerClassType();
    sampleset=G_characterSampleMaps[playerclass];
    /* set up canned sayings list */
    /* 1st, scan through all sample names for this class sample map */
    /* group and find total length */
    if (sampleset > 0) start=G_sampleSetMarkers[sampleset-1];
    else start=0;
    end=G_sampleSetMarkers[sampleset];

    strcpy (G_fielddata,G_cannedSayingTexts[start]);
    for (i=start+1;i<end;i++)
    {
       sprintf (G_fielddata,"%s\r%s",G_fielddata,G_cannedSayingTexts[i]);
    }

    DebugEnd();
}


T_void ComwinDisplayCommunicatePage(T_void)
{
   T_TxtboxID TxtboxID;
   T_buttonID buttonID;

   DebugRoutine ("ComwinDisplayCommunicatePage");

   G_comwinIsOpen=TRUE;

   TxtboxID=FormGetObjID(500);
   TxtboxSetData(TxtboxID,G_fielddata);

   /* attach buttons */
   buttonID=FormGetObjID(301);
   ButtonSetCallbacks (buttonID,NULL,ComwinSay);

   DebugEnd();
}


T_void ComwinCloseCommunicatePage (T_void)
{
   G_comwinIsOpen=FALSE;
}


E_Boolean ComwinIsOpen (T_void)
{
   return (G_comwinIsOpen);
}


T_void ComwinSay (T_buttonID buttonID)
{
   T_TxtboxID TxtboxID, TxtboxID2;
   T_word16 saynum;
   T_byte8 *tosay;
   E_statsClassType playerclass;
   E_comwinSampleType sampleset;

   DebugRoutine ("ComwinSay");

   playerclass=StatsGetPlayerClassType();
   sampleset=G_characterSampleMaps[playerclass];

   TxtboxID=FormGetObjID(501);
   tosay=TxtboxGetData(TxtboxID);

   if (strcmp(tosay,"\0")!=0 && strcmp(tosay,"\r\0")!=0)
   {
      ClientSendMessage(tosay);
      if (strcmp(tosay, "@DIVIDE BY ZERO ERROR") == 0)
          StatsToggleGodMode() ;
   }
   else
   {
      TxtboxID2=FormGetObjID(500);
      saynum=TxtboxGetSelectionNumber (TxtboxID2);
      if (sampleset > 0)
      {
         saynum=saynum+G_sampleSetMarkers[sampleset-1];
      }
      saynum=G_sampleNumbers[saynum];
/*    ClientCreateGlobalAreaSound(
           PlayerGetX16(),
           PlayerGetY16(),
           1000,
           saynum) ;
*/
//      ClientSyncSendActionAreaSound(saynum,1000);
      PlayerMakeSoundGlobal(saynum,1000);
//    SoundPlayByNumber(saynum, 255) ;
   }

   TxtboxSetData(TxtboxID,"\0");

   DebugEnd();
}

T_void ComwinSayNth(T_word16 saynum)
{
   E_statsClassType playerclass;
   E_comwinSampleType sampleset;

   DebugRoutine ("ComwinSay");

   playerclass=StatsGetPlayerClassType();
   sampleset=G_characterSampleMaps[playerclass];

   if (sampleset > 0)
   {
      saynum += G_sampleSetMarkers[sampleset-1];
   }
   if (saynum < G_sampleSetMarkers[sampleset])  {
       saynum=G_sampleNumbers[saynum];
       PlayerMakeSoundGlobal(saynum,1000);
   }

   DebugEnd();
}

/* @} */
/*-------------------------------------------------------------------------*
 * End of File:  COMWIN.C
 *-------------------------------------------------------------------------*/
