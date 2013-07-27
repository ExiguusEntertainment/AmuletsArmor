# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,0,0,UI/LOGON/LOGON_BK
#1,101,306,49,UI/LOGON/LOGON_S1
#1,102,148,120,UI/LOGON/LOGON_S2
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)
#3,300,148,110,0,0,UI/LOGON/LOGON_B1
#3,301,148,185,0,0,UI/LOGON/LOGON_B2
#3,302,306,39,0,0,UI/LOGON/LOGON_B1
#3,303,306,92,0,0,UI/LOGON/LOGON_B2
3,304,162,49,0,14374,UI/LOGON/LOGON_B3
3,305,200,49,0,14382,UI/LOGON/LOGON_B4
3,306,238,49,0,14368,UI/LOGON/LOGON_B5
3,307,276,49,0,14354,UI/LOGON/LOGON_B6
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
#5,500,6,38,303,101,0,0,0,2,0,302,303,101,FontMedium
#ENDOFTEXT

#5,501,6,109,145,194,0,0,0,4,0,300,301,102,FontMedium
#ENDOFTEXT

5,502,162,4,314,47,0,0,0,4,0,0,0,0,FontMedium
ENDOFTEXT

#5,503,163,153,313,180,0,0,1,3,0,0,0,0,FontMedium
#<*> = mail waiting
#<@> = dead character
#<empty> = free character slot
#ENDOFTEXT

#end of form definition file
