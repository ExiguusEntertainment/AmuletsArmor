# communication screen def script

# form definition file for finances banner display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,209,0,UI/3DUI/MONBAR2

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed


# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

3,300,304,4,0,0,UI/COMMON/CLOSEWIN
3,301,215,32,0,0,UI/3DUI/PGET1
3,302,215,44,0,0,UI/3DUI/GGET1
3,303,215,56,0,0,UI/3DUI/SGET1
3,304,215,68,0,0,UI/3DUI/CGET1
3,305,227,32,0,0,UI/3DUI/PGET5
3,306,227,44,0,0,UI/3DUI/GGET5
3,307,227,56,0,0,UI/3DUI/SGET5
3,308,227,68,0,0,UI/3DUI/CGET5

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

5,500,284,32,312,41,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,501,284,44,312,53,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,502,284,56,312,65,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,503,284,68,312,77,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,504,260,101,312,110,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,505,260,113,312,122,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,506,260,125,312,134,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

5,507,260,137,312,146,0,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

#end of form definition file
