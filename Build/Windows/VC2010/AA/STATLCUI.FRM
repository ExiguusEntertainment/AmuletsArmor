# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,36,16,UI/LOADC/LOADC_BK

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)
3,300,45,167,0,14384,UI/LOADC/LOADC_B1
3,301,122,167,0,14367,UI/LOADC/LOADC_B2
3,302,199,167,0,14354,UI/LOADC/LOADC_B3

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

# name field
5,500,45,24,270,33,40,0,1,3,0,0,0,0,FontMedium
ENDOFTEXT

# password field
5,501,104,153,270,162,15,0,0,0,0,0,0,0,FontMedium
ENDOFTEXT

# level field
5,502,255,39,270,46,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# str field
5,503,255,49,270,56,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# con field
5,504,255,59,270,66,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# acc field
5,505,255,69,270,76,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# stl field
5,506,255,79,270,86,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# mag field
5,507,255,89,270,96,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# spd field
5,508,255,99,270,106,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# class field
5,509,196,109,270,116,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# tit field
5,510,196,119,270,126,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# curexp field
5,511,219,129,270,136,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

# curlev field
5,512,219,139,270,146,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT


#end of form definition file
