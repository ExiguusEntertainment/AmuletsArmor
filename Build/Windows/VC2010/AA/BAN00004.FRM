# definition file for options display

# form definition file for inventory banner display

# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,209,0,UI/3DUI/OPTBAR
1,101,308,83,UI/3DUI/OPTSB
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed


# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

3,300,304,4,0,0,UI/COMMON/CLOSEWIN
# yes/no music and sound toggles
3,301,217,19,1,0,UI/CREATEC/CRC_TOG2
3,302,217,44,1,0,UI/CREATEC/CRC_TOG2
# up/down scrollbar buttons for credits
3,303,308,73,0,199,UI/COMMON/UPARROW
3,304,308,140,0,207,UI/COMMON/DNARROW

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

# credits box
5,500,214,73,306,148,0,0,0,2,0,303,304,101,FontTiny
ENDOFTEXT
# type=6 (FormAddSlider) is in the following format:
# type, numerical ID, x1, y1, x2
6,600,216,35,308
6,601,216,60,308

#end of form definition file
