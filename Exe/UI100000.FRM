# form definition file for character creation UI
# first integer defines type of input

# type=1 (FormAddGraphic) is in the following format:
# type, numerical ID(32 bits), x1, y1, and graphic name
1,100,0,0,UI/MAIL/MMAIN_BK
1,101,305,30,UI/MAIL/MMAIN_SB
# type=2 (FormAddText) is in the following format:
# type, numerical ID, x1, y1, fcolor, bcolor, (newline),
# fontname, (newline), text to be displayed

# type=3 (FormAddButton) is in the following format:
# type, numerical ID(32 bits), x1, y1,
# toggle type (0=normal button, 1=toggle button), hotkey scancode,
# followed by the picture name (res file ID name)
3,300,3,3,0,59,UI/MAIL/MMAIN_B1
3,301,54,3,0,60,UI/MAIL/MMAIN_B2
3,302,105,3,0,61,UI/MAIL/MMAIN_B3
3,303,156,3,0,62,UI/MAIL/MMAIN_B4
3,304,207,3,0,63,UI/MAIL/MMAIN_B5
3,305,266,3,0,64,UI/MAIL/MMAIN_B6
3,306,41,22,0,65,UI/MAIL/MMAIN_B7
3,307,141,22,0,66,UI/MAIL/MMAIN_B7
3,308,282,22,0,67,UI/MAIL/MMAIN_B7
3,309,305,21,0,0,UI/MAIL/MMAIN_UP
3,310,305,188,0,0,UI/MAIL/MMAIN_DN

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
5,500,5,36,60,194,0,0,0,4,0,309,310,101,FontMedium
01/23/72
12/05/95
12/12/95
01/01/96
02/12/96
01/23/72
12/05/95
12/12/95
01/01/96
02/12/96
01/23/72
12/05/95
12/12/95
01/01/96
02/12/96
01/23/72
12/05/95
12/12/95
01/01/96
02/12/96
ENDOFTEXT
5,501,62,36,160,194,0,0,0,4,0,309,310,101,FontMedium
John D.
Mike McD
MCA
Bob Fisher
Stephen King
John D.
Mike McD
MCA
Bob Fisher
Stephen King
John D.
Mike McD
MCA
Bob Fisher
Stephen King
John D.
Mike McD
MCA
Bob Fisher
Stephen King
ENDOFTEXT
5,502,162,36,301,194,0,0,0,4,0,309,310,101,FontMedium
Nothing 'tall
Beastie Boys
Record deal
Play Chess?
Gunslinger storyline
Nothing 'tall
Beastie Boys
Record deal
Play Chess?
Gunslinger storyline
Nothing 'tall
Beastie Boys
Record deal
Play Chess?
Gunslinger storyline
Nothing 'tall
Beastie Boys
Record deal
Play Chess?
Gunslinger storyline
ENDOFTEXT
#end of form definition file
