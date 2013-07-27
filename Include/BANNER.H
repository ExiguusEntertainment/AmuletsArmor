/****************************************************************************/
/*    FILE:  BANNER.H                                                         */
/****************************************************************************/

#ifndef _BANNER_H_
#define _BANNER_H_

#include "BUTTON.H"
#include "EQUIP.H"
#include "FORM.H"
#include "GENERAL.H"

#define BANNER_BASE_COLOR 77
#define BANNER_BUTTON_INV 0
#define BANNER_BUTTON_EQP 1
#define BANNER_BUTTON_STS 2
#define BANNER_BUTTON_OPT 3
#define BANNER_BUTTON_COM 4
#define BANNER_BUTTON_FIN 5
#define BANNER_BUTTON_AMO 6
#define BANNER_BUTTON_NOT 7
#define BANNER_BUTTON_STL 8

typedef enum
{
    BANNER_FORM_CURRENT,
    BANNER_FORM_INVENTORY,
    BANNER_FORM_EQUIPMENT,
    BANNER_FORM_STATISTICS,
    BANNER_FORM_OPTIONS,
    BANNER_FORM_COMMUNICATE,
    BANNER_FORM_FINANCES,
    BANNER_FORM_AMMO,
    BANNER_FORM_NOTES,
    BANNER_FORM_JOURNAL,
    BANNER_FORM_LOOK,
    BANNER_FORM_CONTROL,
    //BANNER_FORM_ESC_MENU,
    BANNER_FORM_UNKNOWN,
} E_bannerFormType;

T_void BannerInit(T_void);
T_void BannerUpdate(T_void);
T_void BannerFinish(T_void);

T_void BannerOpenForm (E_bannerFormType formtype);
T_void BannerCloseForm (T_void);
T_void BannerOpenLast (T_void);

T_void BannerOpenFormByButton (T_buttonID buttonID);
T_void BannerCloseFormByButton (T_buttonID buttonID);

T_void BannerFormControl (E_formObjectType objtype,
					      T_word16 objstatus,
					      T_word32 objID);


E_Boolean BannerIsOpen (T_void);
E_Boolean BannerFormIsOpen (E_bannerFormType formtype);

T_void BannerRevertToMenu (T_buttonID buttonID);

/* Routines added by LES on 11/13/95: */
T_void BannerCreateBottomButtons(T_void) ;
T_void BannerDestroyBottomButtons(T_void) ;
T_void BannerRedrawBottomButtons(T_void);

/* potion routines */
T_void PotionInit (T_void);
T_void PotionUpdate (T_void);
T_void PotionFinish (T_void);

T_void BannerStatusBarInit (T_void);
T_void BannerStatusBarUpdate (T_void);
T_void BannerStatusBarFinish (T_void);
T_void BannerUpdateManaDisplay(T_void);
T_void BannerDisplayFinancesPage (T_void);
T_void BannerDisplayAmmoPage(T_void);

E_Boolean BannerFinancesWindowIsAt (T_word16 x, T_word16 y);
E_Boolean BannerAmmoWindowIsAt (T_word16 x, T_word16 y);

T_void BannerAddSpellButton (T_byte8 slot);
T_void BannerRemoveSpellButton (T_byte8 slot);
E_Boolean BannerUseButtonIsDown(T_void);
E_equipBoltTypes BannerGetSelectedAmmoType (T_void);

T_void BannerUIModeOn (T_void);
T_void BannerUIModeOff (T_void);

T_void BannerInitSoundOptions (T_void);
E_Boolean BannerButtonsOk (T_void);

#endif

/****************************************************************************/
/*    END OF FILE:  BANNER.H                                                  */
/****************************************************************************/
