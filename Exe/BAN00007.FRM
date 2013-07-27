# communication screen def script

# form definition file for ammo banner display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,209,0,UI/3DUI/AMMBAR

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed


# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

3,300,304,4,0,0,UI/COMMON/CLOSEWIN
3,301,296,18,1,0,UI/3DUI/AMMSELUP
3,302,296,37,1,0,UI/3DUI/AMMSELUP
3,303,296,56,1,0,UI/3DUI/AMMSELUP
3,304,296,75,1,0,UI/3DUI/AMMSELUP
3,305,296,94,1,0,UI/3DUI/AMMSELUP
3,306,296,113,1,0,UI/3DUI/AMMSELUP
3,307,296,132,1,0,UI/3DUI/AMMSELUP
3,308,214,26,0,0,UI/3DUI/AMMGET
3,309,214,45,0,0,UI/3DUI/AMMGET
3,310,214,64,0,0,UI/3DUI/AMMGET
3,311,214,83,0,0,UI/3DUI/AMMGET
3,312,214,102,0,0,UI/3DUI/AMMGET
3,313,214,121,0,0,UI/3DUI/AMMGET
3,314,214,140,0,0,UI/3DUI/AMMGET

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

#5,500,284,32,312,41,0,0,1,3,0,0,0,0,FontMedium
#ENDOFTEXT

# TYPE FIELDS
5,500,214,18,292,25,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,501,214,37,292,44,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,502,214,56,292,63,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,503,214,75,292,82,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,504,214,94,292,101,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,505,214,113,292,120,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,506,214,132,292,139,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# number fields
5,507,270,26,292,33,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,508,270,45,292,52,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,509,270,64,292,71,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,510,270,83,292,90,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,511,270,102,292,109,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,512,270,121,292,128,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT
5,513,270,140,292,147,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

#end of form definition file
