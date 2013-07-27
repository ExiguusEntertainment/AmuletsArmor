/****************************************************************************/
/*    FILE:  SM.H                                                           */
/****************************************************************************/
#ifndef _SMCCHOOSE_H_
#define _SMCCHOOSE_H_

#include "SMACHINE.H"

/* State constants: */
#define SMCCHOOSE_WAIT_FOR_LIST_STATE                      0
#define SMCCHOOSE_CHOICES_STATE                            1
#define SMCCHOOSE_CREATE_STATE                             2
#define SMCCHOOSE_REQUEST_CREATE_STATE                     3
#define SMCCHOOSE_LOAD_STATE                               4
#define SMCCHOOSE_DOWNLOAD_CHARACTER_STATE                 5
#define SMCCHOOSE_DISPLAY_STATS_STATE                      6
#define SMCCHOOSE_CHECK_PASSWORD_FOR_LOAD_STATE            7
#define SMCCHOOSE_CHANGE_PASSWORD_STATE                    8
#define SMCCHOOSE_ENABLE_BEGIN_STATE                       9
#define SMCCHOOSE_DELETE_STATE                             10
#define SMCCHOOSE_DELETE_CHARACTER_STATE                   11
#define SMCCHOOSE_PLAY_GAME_STATE                          12
#define SMCCHOOSE_CREATE_UPLOAD_STATE                      13
#define SMCCHOOSE_EXIT_STATE                               14
#define SMCCHOOSE_UNKNOWN_STATE                            15

#define NUMBER_SMCCHOOSE_STATES    SMCCHOOSE_UNKNOWN_STATE
#define SMCCHOOSE_INITIAL_STATE    SMCCHOOSE_WAIT_FOR_LIST_STATE

/* Conditional flags: */
#define SMCCHOOSE_FLAG_ENTER_COMPLETE                      0
#define SMCCHOOSE_FLAG_EXIT                                1
#define SMCCHOOSE_FLAG_CHOOSE_CREATE                       2
#define SMCCHOOSE_FLAG_CHOOSE_LOAD                         3
#define SMCCHOOSE_FLAG_CHOOSE_DELETE                       4
#define SMCCHOOSE_FLAG_CREATE_COMPLETE                     5
#define SMCCHOOSE_FLAG_CREATE_ABORT                        6
#define SMCCHOOSE_FLAG_CREATE_STATUS_OK                    7
#define SMCCHOOSE_FLAG_CREATE_STATUS_ERROR                 8
#define SMCCHOOSE_FLAG_CREATE_STATUS_ABORT                 9
#define SMCCHOOSE_FLAG_LOAD_STATUS_OK                      10
#define SMCCHOOSE_FLAG_LOAD_STATUS_INCORRECT               11
#define SMCCHOOSE_FLAG_DOWNLOAD_COMPLETE                   12
#define SMCCHOOSE_FLAG_BEGIN                               13
#define SMCCHOOSE_FLAG_PASSWORD_ENTERED                    14
#define SMCCHOOSE_FLAG_CHANGE_PASSWORD                     15
#define SMCCHOOSE_FLAG_PASSWORD_OK                         16
#define SMCCHOOSE_FLAG_PASSWORD_NOT_OK                     17
#define SMCCHOOSE_FLAG_CHANGE_PASSWORD_COMPLETE            18
#define SMCCHOOSE_FLAG_CHANGE_PASSWORD_ABORT               19
#define SMCCHOOSE_FLAG_ENABLE_COMPLETE                     20
#define SMCCHOOSE_FLAG_DELETE_PASSWORD_OK                  21
#define SMCCHOOSE_FLAG_DELETE_PASSWORD_NOT_OK              22
#define SMCCHOOSE_FLAG_DELETE_COMPLETE                     23
#define SMCCHOOSE_FLAG_CHOOSE_REDRAW                       24
#define SMCCHOOSE_FLAG_CREATE_UPLOAD_OK                    25
#define SMCCHOOSE_FLAG_UNKNOWN                             26
#define NUMBER_SMCCHOOSE_FLAGS    SMCCHOOSE_FLAG_UNKNOWN


T_stateMachineHandle SMCChooseInitialize(T_void) ;

T_void SMCChooseFinish(T_void) ;

T_void SMCChooseUpdate(T_void) ;

E_Boolean SMCChooseCheckFlag(
              T_stateMachineHandle handle,
              T_word32 flag) ;

T_void SMCChooseSetFlag(
           T_word16 flag,
           E_Boolean state) ;

#endif

/****************************************************************************/
/*    END OF FILE:  SM.H                                                    */
/****************************************************************************/
