# form definition file for connection (logon) screen

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name

# background
1,100,0,0,UI/CONNECT/CONBAK
# scroll bar
1,101,306,29,UI/CONNECT/SB
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

# scroll bar up button
3,300,306,19,0,200,UI/CONNECT/SBUP
# scroll bar down button
3,301,306,82,0,208,UI/CONNECT/SBDN
# connect button
3,302,162,94,0,14382,UI/CONNECT/CONNECT
# exit button
3,303,162,107,0,14354,UI/CONNECT/EXIT
# save button
3,304,239,94,0,14367,UI/CONNECT/SAVE
# delete button
3,305,239,107,0,14368,UI/CONNECT/DELETE

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

# server list field
5,500,163,18,303,91,20,0,0,4,0,300,301,101,FontMedium
ENDOFTEXT

# server name field
5,501,163,136,312,146,20,0,0,0,0,0,0,0,FontMedium
ENDOFTEXT

# server number field
5,502,163,159,312,169,20,0,0,0,0,0,0,0,FontMedium
ENDOFTEXT

# modem init string field
5,503,163,182,312,193,60,0,0,0,0,0,0,0,FontMedium
ENDOFTEXT


#end of form definition file
