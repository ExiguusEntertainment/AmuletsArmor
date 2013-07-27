# form definition file for prompt-display-dialogue window

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,26,16,UI/PROMPT/DLGBACK
1,101,279,30,UI/PROMPT/DLGSB
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)

3,300,88,162,0,28,UI/PROMPT/ACCEPT
3,301,172,162,0,1,UI/PROMPT/CANCEL
3,302,279,21,0,200,UI/PROMPT/BULLUP
3,303,279,149,0,208,UI/PROMPT/BULLDN

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

# title/question field
5,500,30,20,274,157,0,0,0,2,0,302,303,101,FontMedium
ENDOFTEXT

