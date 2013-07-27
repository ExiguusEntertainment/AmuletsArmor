/****************************************************************************/
/*    FILE:  HARDFORM.H                                                     */
/****************************************************************************/
#include "GENERAL.H"
#include "MOUSEMOD.H"

#define HARDFORM_GOTO_PLACE_OFFSET 20000
#define HOUSES_START_HARD_FORM 20

#define HARD_FORM_BANK                           0
#define HARD_FORM_STORE                          1
#define HARD_FORM_INN                            2
#define HARD_FORM_HOUSE                          3
#define HARD_FORM_TOWN                           4
#define HARD_FORM_GUILD                          5
#define HARD_FORM_UNKNOWN                        120

typedef T_void (*T_hardFormStart)(T_word32 formNum) ;
typedef T_void (*T_hardFormHandleMouse)
                   (E_mouseEvent event,
                    T_word16 x,
                    T_word16 y,
                    T_buttonClick buttons) ;
typedef T_void (*T_hardFormUpdate)(T_void) ;
typedef T_void (*T_hardFormEnd)(T_void) ;

T_void HardFormStart(T_word32 formNum) ;

T_void HardFormHandleMouse(
           E_mouseEvent event,
           T_word16 x,
           T_word16 y,
           T_buttonClick buttons) ;

T_void HardFormUpdate(T_void) ;

T_void HardFormEnd(T_void) ;
E_Boolean HardFormIsOpen(T_void);


/****************************************************************************/
/*    END OF FILE:  HARDFORM.H                                              */
/****************************************************************************/
