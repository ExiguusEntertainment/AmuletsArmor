# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,0,0,UI/MAIL/MREAD_BK
1,101,305,58,UI/MAIL/MREAD_SB
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

# last button
3,300,4,4,0,59,UI/MAIL/MREAD_B6
# delete button
3,301,80,4,0,60,UI/MAIL/MREAD_B1
# reply button
3,302,132,4,0,61,UI/MAIL/MREAD_B2
# exit button
3,303,184,4,0,62,UI/MAIL/MREAD_B3
# next button
3,304,266,4,0,63,UI/MAIL/MREAD_B7

# sb up arrow
3,305,305,49,0,0,UI/MAIL/MWRI_B3
# sb dn arrow
3,306,305,189,0,0,UI/MAIL/MWRI_B4

# type=4 (FormAddTextButton) is in the following format:
# type, numerical ID (32 bits), x1, y1, text color,
# toggle type (0= normal button, 1=toggle button), hotkey scancode,
# picture name, font name, and button text on 3 seperate lines

# type=5 (FormAddTextBox) is in the following format:
# type, numerical ID (32 bits), x1, y1, x2, y2,
# maximum length (32 bits)0=not applicable,
# datatype (0=alphanumeric, 1=numeric only),
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

# main field
5,500,5,50,302,195,0,0,0,2,0,305,306,101,FontTiny
ENDOFTEXT

# from field
5,501,54,22,214,32,0,0,0,3,0,0,0,0,FontMedium
ENDOFTEXT

# date field
5,502,254,22,314,32,0,0,0,3,0,0,0,0,FontMedium
ENDOFTEXT

# subject field
5,503,54,36,314,46,0,0,0,3,0,0,0,0,FontMedium
ENDOFTEXT

#end of form definition file
