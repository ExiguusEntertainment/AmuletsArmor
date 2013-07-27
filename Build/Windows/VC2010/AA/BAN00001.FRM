# form definition file for inventory banner display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,209,0,UI/3DUI/INVBAR3

# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed


# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

3,300,304,4,0,0,UI/COMMON/CLOSEWIN
#3,301,214,130,0,0,UI/3DUI/USEBUTN
#3,302,231,130,0,0,UI/3DUI/EQPBUTN
3,302,214,140,0,0,UI/3DUI/2EQBUTN
#3,303,248,130,0,0,UI/3DUI/EXMBUTN
#3,304,265,130,0,0,UI/3DUI/DRPBUTN
#3,305,282,130,0,0,UI/3DUI/LSTBUTN
3,305,267,129,0,0,UI/COMMON/LTARROW
3,306,277,129,0,0,UI/COMMON/RTARROW
#3,306,299,130,0,0,UI/3DUI/NXTBUTN

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

5,500,213,129,265,136,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

5,501,287,129,314,136,0,0,1,3,0,0,0,0,FontTiny
ENDOFTEXT

#5,500,214,140,280,147,0,0,1,3,0,0,0,0,FontTiny
#ENDOFTEXT

#5,501,282,140,314,147,0,0,1,3,0,0,0,0,FontTiny
#ENDOFTEXT

#end of form definition file
