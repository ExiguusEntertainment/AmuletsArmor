# communication screen def script
# form definition file for 'look/view' banner display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,209,0,UI/3DUI/VIEWBAR
1,101,308,112,UI/3DUI/VIEWSB
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)
3,300,304,4,0,0,UI/COMMON/CLOSEWIN
3,301,308,102,0,0,UI/COMMON/UPARROW
3,302,308,139,0,0,UI/COMMON/DNARROW

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

# monster/player name field
5,500,214,91,314,100,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# monster/player data field
5,501,214,102,306,147,0,0,0,2,0,301,302,101,FontTiny
ENDOFTEXT

#end of form definition file
