# form definition file for statistics banner display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,209,0,UI/3DUI/STABAR

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed


# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

3,300,304,4,0,0,UI/COMMON/CLOSEWIN

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

# player name field
5,500,214,16,313,23,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player level field
5,501,299,26,313,33,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player strength field
5,502,299,36,313,43,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player constitution field
5,503,299,46,313,53,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player accuracy field
5,504,299,56,313,63,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player stealth field
5,505,299,66,313,73,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player magic field
5,506,299,76,313,83,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player speed field
5,507,299,86,313,93,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player class field
5,508,240,102,313,109,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player class title field
5,509,240,112,313,119,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player current exp field
5,510,263,128,313,135,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# player needed exp to next level field
5,511,263,138,313,145,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

#end of form definition file
