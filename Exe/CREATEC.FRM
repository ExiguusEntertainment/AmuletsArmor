# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,1,0,0,UI/CREATEC/CRC_BACK

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

# last character button
3,301,12,122,0,203,UI/CREATEC/CRC_LST
# next char button
3,302,110,122,0,205,UI/CREATEC/CRC_NXT
# accept button
3,303,133,182,0,28,UI/CREATEC/CRC_ACC
# cancel button
3,304,252,182,0,1,UI/CREATEC/CRC_CNC

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

# character description field
5,500,11,134,124,189,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# character name field
5,501,138,19,305,28,24,0,0,0,0,0,0,0,FontMedium
ENDOFTEXT

# character typed description field
#5,502,138,43,305,116,150,0,0,0,0,0,0,0,FontMedium
#ENDOFTEXT

# str field
5,503,202,135,229,144,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# con field
5,504,202,148,229,157,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# acc field
5,505,202,161,229,170,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# spd field
5,506,275,135,302,144,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# mag field
5,507,275,148,302,157,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# stl field
5,508,275,161,302,170,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# char title field
5,509,29,122,106,131,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# end of file
