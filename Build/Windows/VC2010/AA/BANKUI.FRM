# form definition file for bank display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,0,0,UI/BANK/BNK_BACK

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed


# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

# exit bank button
3,300,301,7,0,0,UI/3DUI/CLOSBUTN

# last bank inventory page button
3,301,5,160,0,0,UI/BANK/BNK_LAST
# next bank inventory page button
3,302,15,160,0,0,UI/BANK/BNK_NEXT
# deposit buttons
3,303,113,63,0,0,UI/BANK/BNK_FINL
3,304,113,75,0,0,UI/BANK/BNK_FINL
3,305,113,87,0,0,UI/BANK/BNK_FINL
3,306,113,99,0,0,UI/BANK/BNK_FINL
# withdraw buttons
3,307,123,63,0,0,UI/BANK/BNK_FINR
3,308,123,75,0,0,UI/BANK/BNK_FINR
3,309,123,87,0,0,UI/BANK/BNK_FINR
3,310,123,99,0,0,UI/BANK/BNK_FINR
# last user inventory page button
3,311,267,159,0,0,UI/3DUI/INVLST
# next user inventory page button
3,312,277,159,0,0,UI/3DUI/INVNXT

# type=4 (FormAddTextButton) is in the following format:
# type, numerical ID (32 bits), x1, y1, text color,
# toggle type (0= normal button, 1=toggle button), hotkey scancode,
# picture name, font name, and button text on 3 seperate lines

# type=5 (FormAddTextBox) is in the following format:
# type, numerical ID (32 bits), x1, y1, x2, y2,
# maximum length (32 bits), datatype (0=alphanumeric, 1=numeric only),
# justification (0=left justify, 1= center),
# field type, hotkey scancode,
# objID of scroll bar up arrow (0 if not applicable), objID of
# scroll bar down arrow, objID of scroll bar graphic,
# the name of the font to use,
# newline, and starting text followed by ENDOFTEXT
# field type is of the following:
# 0 = EDITABLE, SCROLLING FIELD
# 1 = EDITABLE, SCROLLING FORM
# 2 = NON-EDITABLE, SCROLLING FORM
# 3 = NON-EDITABLE, NON-SCROLLING FORM
# 4 = SELECTION BOX

#player carried weight field
5,500,213,159,265,166,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

#current player inventory page number field
5,501,287,159,314,166,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

#platinum saved in bank
5,502,51,62,103,71,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#gold saved in bank
5,503,51,74,103,83,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#silver saved in bank
5,504,51,86,103,95,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#copper saved in bank
5,505,51,98,103,107,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#carried platinum
5,506,178,62,206,71,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#carried gold
5,507,178,74,206,83,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#carried silver
5,508,178,86,206,95,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#carried copper
5,509,178,98,206,107,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#current bank inventory page number display
5,510,25,160,105,167,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

#current type of listed bank inventory
5,511,5,169,105,180,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#info bar
5,512,5,186,314,195,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#end of form definition file
