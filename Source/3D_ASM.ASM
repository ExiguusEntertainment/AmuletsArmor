
	;IDEAL
	;JUMPS
	;P386
	;P387		; Allow 386 processor


	;MASM
	.MODEL FLAT	;32-bit OS/2 model
	.CODE
	;IDEAL

; EAX = working variable (pixel in use)
; EBX = look-up table for shading (p_shade)
; ECX = how many pixels to draw
; EDX = delta in texture (step rate)
; ESI = position in texture
; EDI = screen position (p_pixel)

     extrn _G_CurrentTexturePos:dword
     extrn _G_textureStepX:dword
     extrn _G_textureStepY:dword

     extrn _G_textureAndX:dword
     extrn _G_textureAndY:dword
     extrn _G_textureShift:dword

     extrn _G_objColumnStart:word
     extrn _G_objColumnEnd:word

     extrn _G_translucentTable:byte

     public DTlCA_16                ; These labels are for debugging only
     public DTCA64_16
     public DTRA64_16
     public DOCA_16

     public DrawTranslucentAsm_
     public DrawTranslucentSeeThroughAsm_
     public DrawSeeThroughAsm_
     public DrawObjectColumnAsm_
     public DrawTranslucentObjectColumnAsm_

     public DrawTextureColumnAsm1_
     public DrawTextureColumnAsm2_
     public DrawTextureColumnAsm4_
     public DrawTextureColumnAsm8_
     public DrawTextureColumnAsm16_
     public DrawTextureColumnAsm32_
     public DrawTextureColumnAsm64_
     public DrawTextureColumnAsm128_
     public DrawTextureColumnAsm256_

     public DrawTransparentColumnAsm1_
     public DrawTransparentColumnAsm2_
     public DrawTransparentColumnAsm4_
     public DrawTransparentColumnAsm8_
     public DrawTransparentColumnAsm16_
     public DrawTransparentColumnAsm32_
     public DrawTransparentColumnAsm64_
     public DrawTransparentColumnAsm128_
     public DrawTransparentColumnAsm256_

     public DrawTranslucentColumnAsm1_
     public DrawTranslucentColumnAsm2_
     public DrawTranslucentColumnAsm4_
     public DrawTranslucentColumnAsm8_
     public DrawTranslucentColumnAsm16_
     public DrawTranslucentColumnAsm32_
     public DrawTranslucentColumnAsm64_
     public DrawTranslucentColumnAsm128_
     public DrawTranslucentColumnAsm256_

;     public DrawTextureColumnAsm_
     public DrawTextureRowAsm_
     public GraphicsMode13X_

     public DrawTextureRowAsm1_
     public DrawTextureRowAsm2_
     public DrawTextureRowAsm4_
     public DrawTextureRowAsm8_
     public DrawTextureRowAsm16_
     public DrawTextureRowAsm32_
     public DrawTextureRowAsm64_
     public DrawTextureRowAsm128_
     public DrawTextureRowAsm256_

     public DrawTransRowAsm1_
     public DrawTransRowAsm2_
     public DrawTransRowAsm4_
     public DrawTransRowAsm8_
     public DrawTransRowAsm16_
     public DrawTransRowAsm32_
     public DrawTransRowAsm64_
     public DrawTransRowAsm128_
     public DrawTransRowAsm256_
     public Div32by32To1616Asm_
     public FindInterXAsm_

     public ClearSampleAsm_
     public _ClearVar1

     public DivZOn_
     public DivZOff_

     public ColorizeMemAsm_
     public ShadeMemAsm_

; Algorithm:
; ----------
;    for (y=top; y<bottom; y++, p_pixel += 320)  {
;        *p_pixel = p_shade[p_texture[(textureOffset>>16)&63]] ;
;        textureOffset += textureStep ;
;    }
;

;; *** Solid fill -- best speed? ***
;Proc DrawTextureColumnAsm_ Near
;       push ebp
;       mov ebp, [dword ptr _G_CurrentTexturePos]
;       shl esi, 10
;       shl edx, 10
;       xor eax, eax
;       shld eax, esi, 6
;       mov al, [ebp+eax]
;       xlat
;dtca_loop:
;       mov [edi], al
;       add edi, 320
;;       add esi, edx
;       loop dtca_loop
;       pop ebp
;       ret
; EndP

DrawObjectColumnAsmSolid_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 10
       shl edx, 10
       xor eax, eax
       shld eax, esi, 6
;       mov al, [ebp+eax]
;       xlat
       mov al, 255
dtca_loop:
       mov [edi], al
       add edi, 320
;       add esi, edx
       loop dtca_loop
       pop ebp
       ret
DrawObjectColumnAsmSolid_ EndP

DOCA_16:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca16_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca16_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_15:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca15_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca15_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_14:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca14_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca14_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_13:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca13_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca13_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_12:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca12_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca12_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_11:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca11_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca11_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_10:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca10_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca10_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_9:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca9_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca9_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_8:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca8_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca8_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_7:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca7_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca7_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_6:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca6_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca6_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_5:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca5_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca5_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_4:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca4_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca4_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_3:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca3_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca3_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_2:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca2_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca2_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_1:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, cx
       jg DOCA_0

       mov al, [ebp+eax]      ; Find the pixel at that location
       test al, al             ; Is the pixel zero?
       je doca1_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

doca1_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
DOCA_0:
       ret

DOCA_Table      dd DOCA_0
                dd DOCA_1
                dd DOCA_2
                dd DOCA_3
                dd DOCA_4
                dd DOCA_5
                dd DOCA_6
                dd DOCA_7
                dd DOCA_8
                dd DOCA_9
                dd DOCA_10
                dd DOCA_11
                dd DOCA_12
                dd DOCA_13
                dd DOCA_14
                dd DOCA_15


DrawObjectColumnAsm_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
doca_preloop:
       shld eax, esi, 16
       cmp ax, [_G_objColumnStart]
       jge doca_loop
       add edi, 320
       add esi, edx
       loop doca_preloop
       jmp doca_end

doca_loop:
       test cx, 0FFF0h
       je doca_next
       push cx
       mov cx, [_G_objColumnEnd]
       call DOCA_16
       pop cx
       sub cx, 010h
       jmp doca_loop
doca_next:
       movzx eax, cx
       mov cx, [_G_objColumnEnd]
       call [dword ptr DOCA_Table + eax*4]
doca_end:
       pop ebp                ; restore the valuable ebp register
       ret
DrawObjectColumnAsm_ EndP

DrawObjectColumnAsmOld_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
docao_preloop:
       shld eax, esi, 16
       cmp ax, [_G_objColumnStart]
       jge docao_loop
       add edi, 320
       add esi, edx
       loop docao_preloop
       jmp docao_end

docao_loop:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, [_G_objColumnEnd]
       jg docao_end

       mov al, [ebp+eax]      ; Find the pixel at that location
       and al, al             ; Is the pixel zero?
       je docao_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov [edi], al          ; draw it

docao_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
       loop docao_loop         ; loop cx times

docao_end:
       pop ebp                ; restore the valuable ebp register
       ret
DrawObjectColumnAsmOld_ EndP

DrawTranslucentObjectColumnAsm_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dotca_preloop:
       shld eax, esi, 16
       cmp ax, [_G_objColumnStart]
       jge dotca_loop
       add edi, 320
       add esi, edx
       loop dotca_preloop
       jmp dotca_end

dotca_loop:
       xor eax, eax           ; Clear out ax
       shld eax, esi, 16      ; Get the upper 16 bits

       cmp ax, [_G_objColumnEnd]
       jg dotca_end

       movzx eax, [byte ptr ebp+eax]      ; Find the pixel at that location
       and al, al             ; Is the pixel zero?
       je dotca_skip           ; Yes, therefore transparent, do not draw

       xlat                   ; Shade the color
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi

       mov [edi], al          ; draw it

dotca_skip:
       add edi, 320           ; next line
       add esi, edx           ; add the step rate
       loop dotca_loop         ; loop cx times

dotca_end:
       pop ebp                ; restore the valuable ebp register
       ret
DrawTranslucentObjectColumnAsm_ EndP

DrawTextureColumnAsm1_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 16
       shl edx, 16
dtca_loop1:
       xor eax, eax
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop1
       pop ebp
       ret
DrawTextureColumnAsm1_ EndP

DrawTextureColumnAsm2_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 15
       shl edx, 15
dtca_loop2:
       xor eax, eax
       shld eax, esi, 1
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop2
       pop ebp
       ret
DrawTextureColumnAsm2_ EndP

DrawTextureColumnAsm4_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 14
       shl edx, 14
dtca_loop4:
       xor eax, eax
       shld eax, esi, 2
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop4
       pop ebp
       ret
DrawTextureColumnAsm4_ EndP

DrawTextureColumnAsm8_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 13
       shl edx, 13
dtca_loop8:
       xor eax, eax
       shld eax, esi, 3
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop8
       pop ebp
       ret
DrawTextureColumnAsm8_ EndP

DrawTextureColumnAsm16_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 12
       shl edx, 12
dtca_loop16:
       xor eax, eax
       shld eax, esi, 4
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop16
       pop ebp
       ret
DrawTextureColumnAsm16_ EndP

DrawTextureColumnAsm32_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 11
       shl edx, 11
dtca_loop32:
       xor eax, eax
       shld eax, esi, 5
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop32
       pop ebp
       ret
DrawTextureColumnAsm32_ EndP

DTCA64_16:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_15:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_14:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_13:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_12:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_11:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_10:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_9:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_8:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_7:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_6:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_5:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_4:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_3:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_2:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_1:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, ecx
       add esi, edx
DTCA64_0:
       ret

DTCA64_Table      dd DTCA64_0
                  dd DTCA64_1
                  dd DTCA64_2
                  dd DTCA64_3
                  dd DTCA64_4
                  dd DTCA64_5
                  dd DTCA64_6
                  dd DTCA64_7
                  dd DTCA64_8
                  dd DTCA64_9
                  dd DTCA64_10
                  dd DTCA64_11
                  dd DTCA64_12
                  dd DTCA64_13
                  dd DTCA64_14
                  dd DTCA64_15
                  dd DTCA64_16

DrawTextureColumnAsm64_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 10
       shl edx, 10
dtca64_loop1:
       test cx, 0FFF0h
       je dtca64_loop2
       push ecx
       mov ecx, 320
       call DTCA64_16
       pop ecx
       sub cx, 010h
       jmp dtca64_loop1

dtca64_loop2:
       mov eax, ecx
       mov ecx, 320
       call [dword ptr DTCA64_Table+eax*4]

dtca64_end:
       pop ebp
       ret
DrawTextureColumnAsm64_ EndP

DrawTextureColumnAsm128_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 9
       shl edx, 9
dtca_loop128:
       xor eax, eax
       shld eax, esi, 7
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop128
       pop ebp
       ret
DrawTextureColumnAsm128_ EndP

DrawTextureColumnAsm256_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 8
       shl edx, 8
dtca_loop256:
       xor eax, eax
       shld eax, esi, 8
       mov al, [ebp+eax]
       xlat
       mov [edi], al
       add edi, 320
       add esi, edx
       loop dtca_loop256
       pop ebp
       ret
DrawTextureColumnAsm256_ EndP

;Proc DrawTextureColumnAsm_ Near
;       push ebp
;       mov ebp, [dword ptr _G_CurrentTexturePos]
;dtca_loop:
;       push esi
;       sar esi, 16
;;;       and esi, 03Fh
;       and esi, [dword ptr _G_textureAndY]
;       mov al, [ebp+esi]
;       pop esi
;       xlat
;       mov [edi], al
;       add edi, 320
;;;       add edi, 80
;       add esi, edx
;       loop dtca_loop
;       pop ebp
;       ret

; EndP

; Algorithm:
; ----------
;     while (start != end)  {
;        start++ ;
;        *(p_pixel++) = p_shade[GG_myTexture2[(((x>>16)&63)<<6)|((y>>16)&63)]] ;
;        x += dx ;
;        y += dy ;
;    }
;
; New algorithm:
;     while (start != end)  {
;        start++ ;
;        *(p_pixel++) = p_shade[GG_myTexture2[((p>>16)&4095)]] ;
;        p += dp ;
;    }
;

; EAX = working variable (pixel in use)
; EBX = look-up table for shading (p_shade)
; ECX = how many pixels to draw
; ESI = x Position
; EDX = y Position
; EDI = screen position (p_pixel)

DrawTextureRowAsm_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop:
       push esi
       push edx
;;       and esi, 03F0000h
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 6  ; <---- change
;;       sar esi, 10
       sar edx, 16
       and edx, [dword ptr _G_textureAndY]
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop
       pop ebp
       ret
DrawTextureRowAsm_ EndP

DrawTextureRowAsm1_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
      xor edx, edx
dtra_loop1:
       push esi
;;;       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
;;       sal esi, 0
;;;       sar edx, 16
;;;       and edx, 000h
       or esi, edx
       mov al, [ebp+esi]
;;;       pop edx
       pop esi
       xlat
       stosb
;;;       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop1
       pop ebp
       ret
DrawTextureRowAsm1_ EndP

DrawTextureRowAsm2_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop2:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 1
       sar edx, 16
       and edx, 001h
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop2
       pop ebp
       ret
DrawTextureRowAsm2_ EndP

DrawTextureRowAsm4_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop4:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 2
       sar edx, 16
       and edx, 003h
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop4
       pop ebp
       ret
DrawTextureRowAsm4_ EndP

DrawTextureRowAsm8_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop8:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 3
       sar edx, 16
       and edx, 007h
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop8
       pop ebp
       ret
DrawTextureRowAsm8_ EndP

DrawTextureRowAsm16_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop16:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 4
       sar edx, 16
       and edx, 00Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop16
       pop ebp
       ret
DrawTextureRowAsm16_ EndP

DrawTextureRowAsm32_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop32:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 5
       sar edx, 16
       and edx, 01Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop32
       pop ebp
       ret
DrawTextureRowAsm32_ EndP

 ; Solid fill -- best speed?
;Proc DrawTextureRowAsm64_ Near
;       push ebp
;       mov ebp, [dword ptr _G_CurrentTexturePos]
;       shl esi, 10
;       shl edx, 10
;       shl [dword ptr _G_textureStepX], 10
;       shl [dword ptr _G_textureStepY], 10
;       xor eax, eax
;       shld eax, esi, 6
;       shld eax, edx, 6
;       mov al, [ebp+eax]
;       xlat
;dtra_loop64:
;       rep stosb
;;       add edx, [dword ptr _G_textureStepY]
;;       add esi, [dword ptr _G_textureStepX]
;;       loop dtra_loop64
;       pop ebp
;       ret
; EndP

DTRA64_16:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_15:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_14:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_13:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_12:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_11:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_10:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_9:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_8:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_7:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_6:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_5:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_4:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_3:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_2:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_1:
       xor eax, eax
       shld eax, esi, 6
       shld eax, edx, 6
       mov al, [ebp+eax]
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, ecx
DTRA64_0:
       ret

DTRA64_Table dd DTRA64_0
             dd DTRA64_1
             dd DTRA64_2
             dd DTRA64_3
             dd DTRA64_4
             dd DTRA64_5
             dd DTRA64_6
             dd DTRA64_7
             dd DTRA64_8
             dd DTRA64_9
             dd DTRA64_10
             dd DTRA64_11
             dd DTRA64_12
             dd DTRA64_13
             dd DTRA64_14
             dd DTRA64_15

DrawTextureRowAsm64_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 10
       shl edx, 10
       shl [dword ptr _G_textureStepX], 10
       shl [dword ptr _G_textureStepY], 10

dtra64_loop1:
       test cx, 0FFF0h
       je dtra64_loop2
       push ecx
       mov ecx, [dword ptr _G_textureStepX]
       call DTRA64_16
       pop ecx
       sub ecx, 010h
       jmp dtra64_loop1
dtra64_loop2:
       mov eax, ecx
       and eax, 0Fh
       mov ecx, [dword ptr _G_textureStepX]
       call [dword ptr DTRA64_Table + eax*4]
       pop ebp
       ret
DrawTextureRowAsm64_ EndP

DrawTextureRowAsm128_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop128:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 7
       sar edx, 16
       and edx, 07Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop128
       pop ebp
       ret
DrawTextureRowAsm128_ EndP

DrawTextureRowAsm256_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dtra_loop256:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 8
       sar edx, 16
       and edx, 0FFh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       xlat
       stosb
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dtra_loop256
       pop ebp
       ret
DrawTextureRowAsm256_ EndP

GraphicsMode13X_ PROC Near
        mov ax, 013h
        int 10h

        mov dx, 03CEh
        mov al, 5
        out dx, al
        inc dx
        in al, dx
        and al, 0EFh
        out dx, al

        dec dx

        mov al, 6
        out dx, al
        inc dx
        in al, dx
        and al, 0FDh
        out dx, al

        mov dx, 03C4h
        mov al, 4
        out dx, al
        inc dx
        in al, dx
        and al, 0F7h
        or al, 4
        out dx, al

        mov dx, 03D4h
        mov al, 14h
        out dx, al
        inc dx
        in al, dx
        and al, 0BFh
        out dx, al

        dec dx

        mov al, 17h
        out dx, al
        inc dx
        in al, dx
        or al, 040h
        out dx, al

        ; Clear the screen
        mov edi, 0A0000h
        mov dx, 03C4h
        mov ax, 00F02h
        out dx, ax
        mov ecx, 4000
        xor eax, eax
        rep stosd
        ret
GraphicsMode13X_    EndP

DrawTransRowAsm1_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
      xor edx, edx
dttra_loop1:
       push esi
;;;       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
;;       sal esi, 0
;;;       sar edx, 16
;;;       and edx, 000h
       or esi, edx
       mov al, [ebp+esi]
;;;       pop edx
       pop esi
       test al, al
       je dttra_skip1
       xlat
       mov [edi], al
dttra_skip1:
       inc edi
;;;       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop1
       pop ebp
       ret
DrawTransRowAsm1_ EndP

DrawTransRowAsm2_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop2:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 1
       sar edx, 16
       and edx, 001h
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip2
       xlat
       mov [edi], al
dttra_skip2:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop2
       pop ebp
       ret
DrawTransRowAsm2_ EndP

DrawTransRowAsm4_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop4:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 2
       sar edx, 16
       and edx, 003h
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip4
       xlat
       mov [edi], al
dttra_skip4:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop4
       pop ebp
       ret
DrawTransRowAsm4_ EndP

DrawTransRowAsm8_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop8:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 3
       sar edx, 16
       and edx, 007h
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip8
       xlat
       mov [edi], al
dttra_skip8:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop8
       pop ebp
       ret
DrawTransRowAsm8_ EndP

DrawTransRowAsm16_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop16:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 4
       sar edx, 16
       and edx, 00Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip16
       xlat
       mov [edi], al
dttra_skip16:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop16
       pop ebp
       ret
DrawTransRowAsm16_ EndP

DrawTransRowAsm32_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop32:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 5
       sar edx, 16
       and edx, 01Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip32
       xlat
       mov [edi], al
dttra_skip32:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop32
       pop ebp
       ret
DrawTransRowAsm32_ EndP

DrawTransRowAsm64_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop64:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 6
       sar edx, 16
       and edx, 03Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip64
       xlat
       mov [edi], al
dttra_skip64:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop64
       pop ebp
       ret
DrawTransRowAsm64_ EndP

DrawTransRowAsm128_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop128:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 7
       sar edx, 16
       and edx, 07Fh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip128
       xlat
       mov [edi], al
dttra_skip128:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop128
       pop ebp
       ret
DrawTransRowAsm128_ EndP

DrawTransRowAsm256_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
dttra_loop256:
       push esi
       push edx
       sar esi, 16
       and esi, [dword ptr _G_textureAndX]
       sal esi, 8
       sar edx, 16
       and edx, 0FFh
       or esi, edx
       mov al, [ebp+esi]
       pop edx
       pop esi
       test al, al
       je dttra_skip256
       xlat
       mov [edi], al
dttra_skip256:
       inc edi
       add edx, [dword ptr _G_textureStepY]
       add esi, [dword ptr _G_textureStepX]
       loop dttra_loop256
       pop ebp
       ret
DrawTransRowAsm256_ EndP

Div32by32To1616Asm_ PROC Near
       push edx
       push ebx

       cdq                ; Extend eax into edx
       shld edx, eax, 16  ; Shift upper 16 bits of eax into edx
       sal eax, 16        ; Shift lower 16 bits of eax into upper of eax
       idiv ebx           ; Do the divide

       pop ebx            ; Restore everyone except the answer in eax
       pop edx
       ret
Div32by32To1616Asm_ EndP

FindInterXAsm_ PROC Near
       ; eax = deltaZ
       ; ebx = tanViewAngle
       ; esi = deltaX
       ; edi = zTop
       push ebx
       push esi
       push edi
       push ebx

       neg ebx
       imul ebx           ; -G_wall.deltaZ * tanViewAngle => (32:32)
       add edx, esi       ; (G_wall.deltaX<<32) + (above) => (32:32)

       cmp edx, 0         ; if (tempCalc != 0)  ... skip
       jne fi_skip1
       cmp eax, 0
       jne fi_skip1

       mov edx, 10000     ; interZ = 10000 << 16 ;
       xor eax, eax
       jmp fi_skip2
fi_skip1:
       add eax, 08000h    ; tempCalc = (tempCalc+8000h) >> 16
       jnc fi_skip3
       inc edx
fi_skip3:
                          ; interZ = G_wall.zTop / tempCalc ;
       mov ebx, edi
       xor eax, eax
       idiv ebx           ; interZ => (32:32)
fi_skip2:
       ; interX = ((interZ * tanViewAngle) >> 16) ;
       shrd eax, edx, 16
       pop ebx
       imul ebx
       mov eax, edx
       shr eax, 16
       pop edi
       pop esi
       pop ebx
       ret
FindInterXAsm_ EndP

ClearSampleAsm_ PROC Near
     push eax
     push edi
     push ecx
     push ds
     push es

     push ds
     pop es
;     mov al, 0
     db 0B0h
_ClearVar1     db 000h
     mov edi, 0A0000h
     mov ecx, 0FA00h
     rep stosb

     pop es
     pop ds
     pop ecx
     pop edi
     pop eax
     ret
ClearSampleAsm_ EndP

OldZ     dd 0 ; (?)

DivZOn_ PROC Near
     push ebx
     xor ebx, ebx
     mov eax, [ebx]
     mov [dword ptr OldZ], eax
     mov eax, offset DivZ_
     mov [ebx], eax
     pop ebx
     ret
DivZOn_ EndP

DivZOff_ PROC Near
     push ebx
     xor ebx, ebx
     mov eax, [dword ptr OldZ]
     mov [ebx], eax
     pop ebx
     ret
DivZOff_ EndP

DivZ_ PROC Near
     xor eax, eax
     xor edx, edx
     dec eax
     dec edx
     iret
DivZ_ EndP

DrawTransparentColumnAsm1_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 16
       shl edx, 16
dtransca_loop1:
       xor eax, eax
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip1
       xlat
       mov [edi], al
dtransca_skip1:
       add edi, 320
       add esi, edx
       loop dtransca_loop1
       pop ebp
       ret
DrawTransparentColumnAsm1_ EndP

DrawTransparentColumnAsm2_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 15
       shl edx, 15
dtransca_loop2:
       xor eax, eax
       shld eax, esi, 1
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip2
       xlat
       mov [edi], al
dtransca_skip2:
       add edi, 320
       add esi, edx
       loop dtransca_loop2
       pop ebp
       ret
DrawTransparentColumnAsm2_ EndP

DrawTransparentColumnAsm4_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 14
       shl edx, 14
dtransca_loop4:
       xor eax, eax
       shld eax, esi, 2
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip3
       xlat
       mov [edi], al
dtransca_skip3:
       add edi, 320
       add esi, edx
       loop dtransca_loop4
       pop ebp
       ret
DrawTransparentColumnAsm4_ EndP

DrawTransparentColumnAsm8_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 13
       shl edx, 13
dtransca_loop8:
       xor eax, eax
       shld eax, esi, 3
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip4
       xlat
       mov [edi], al
dtransca_skip4:
       add edi, 320
       add esi, edx
       loop dtransca_loop8
       pop ebp
       ret
DrawTransparentColumnAsm8_ EndP

DrawTransparentColumnAsm16_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 12
       shl edx, 12
dtransca_loop16:
       xor eax, eax
       shld eax, esi, 4
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip5
       xlat
       mov [edi], al
dtransca_skip5:
       add edi, 320
       add esi, edx
       loop dtransca_loop16
       pop ebp
       ret
DrawTransparentColumnAsm16_ EndP

DrawTransparentColumnAsm32_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 11
       shl edx, 11
dtransca_loop32:
       xor eax, eax
       shld eax, esi, 5
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip6
       xlat
       mov [edi], al
dtransca_skip6:
       add edi, 320
       add esi, edx
       loop dtransca_loop32
       pop ebp
       ret
DrawTransparentColumnAsm32_ EndP

DrawTransparentColumnAsm64_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 10
       shl edx, 10
dtransca_loop64:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip7
       xlat
       mov [edi], al
dtransca_skip7:
       add edi, 320
       add esi, edx
       loop dtransca_loop64
       pop ebp
       ret
DrawTransparentColumnAsm64_ EndP

DrawTransparentColumnAsm128_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 9
       shl edx, 9
dtransca_loop128:
       xor eax, eax
       shld eax, esi, 7
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip8
       xlat
       mov [edi], al
dtransca_skip8:
       add edi, 320
       add esi, edx
       loop dtransca_loop128
       pop ebp
       ret
DrawTransparentColumnAsm128_ EndP

DrawTransparentColumnAsm256_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 8
       shl edx, 8
dtransca_loop256:
       xor eax, eax
       shld eax, esi, 8
       mov al, [ebp+eax]
       test al, al
       je dtransca_skip9
       xlat
       mov [edi], al
dtransca_skip9:
       add edi, 320
       add esi, edx
       loop dtransca_loop256
       pop ebp
       ret
DrawTransparentColumnAsm256_ EndP

DrawTranslucentColumnAsm1_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 16
       shl edx, 16
dlucentca_loop1:
       xor eax, eax
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip1
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi

       mov [edi], al
dlucentca_skip1:
       add edi, 320
       add esi, edx
       loop dlucentca_loop1
       pop ebp
       ret
DrawTranslucentColumnAsm1_ EndP

DrawTranslucentColumnAsm2_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 15
       shl edx, 15
dlucentca_loop2:
       xor eax, eax
       shld eax, esi, 1
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip2
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip2:
       add edi, 320
       add esi, edx
       loop dlucentca_loop2
       pop ebp
       ret
DrawTranslucentColumnAsm2_ EndP

DrawTranslucentColumnAsm4_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 14
       shl edx, 14
dlucentca_loop4:
       xor eax, eax
       shld eax, esi, 2
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip3
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip3:
       add edi, 320
       add esi, edx
       loop dlucentca_loop4
       pop ebp
       ret
DrawTranslucentColumnAsm4_ EndP

DrawTranslucentColumnAsm8_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 13
       shl edx, 13
dlucentca_loop8:
       xor eax, eax
       shld eax, esi, 3
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip4
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip4:
       add edi, 320
       add esi, edx
       loop dlucentca_loop8
       pop ebp
       ret
DrawTranslucentColumnAsm8_ EndP

DrawTranslucentColumnAsm16_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 12
       shl edx, 12
dlucentca_loop16:
       xor eax, eax
       shld eax, esi, 4
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip5
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip5:
       add edi, 320
       add esi, edx
       loop dlucentca_loop16
       pop ebp
       ret
DrawTranslucentColumnAsm16_ EndP

DrawTranslucentColumnAsm32_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 11
       shl edx, 11
dlucentca_loop32:
       xor eax, eax
       shld eax, esi, 5
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip6
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip6:
       add edi, 320
       add esi, edx
       loop dlucentca_loop32
       pop ebp
       ret
DrawTranslucentColumnAsm32_ EndP

DTlCA_16:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_16
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_16:
       add edi, 320
       add esi, edx
DTlCA_15:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_15
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_15:
       add edi, 320
       add esi, edx
DTlCA_14:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_14
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_14:
       add edi, 320
       add esi, edx
DTlCA_13:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_13
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_13:
       add edi, 320
       add esi, edx
DTlCA_12:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_12
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_12:
       add edi, 320
       add esi, edx
DTlCA_11:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_11
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_11:
       add edi, 320
       add esi, edx
DTlCA_10:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_10
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_10:
       add edi, 320
       add esi, edx
DTlCA_9:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_9
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_9:
       add edi, 320
       add esi, edx
DTlCA_8:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_8
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_8:
       add edi, 320
       add esi, edx
DTlCA_7:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_7
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_7:
       add edi, 320
       add esi, edx
DTlCA_6:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_6
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_6:
       add edi, 320
       add esi, edx
DTlCA_5:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_5
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_5:
       add edi, 320
       add esi, edx
DTlCA_4:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_4
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_4:
       add edi, 320
       add esi, edx
DTlCA_3:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_3
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_3:
       add edi, 320
       add esi, edx
DTlCA_2:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_2
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_2:
       add edi, 320
       add esi, edx
DTlCA_1:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dtlca_skip_1
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       mov al, [ecx+eax]          ; Get the translucent color in the table
       mov [edi], al
dtlca_skip_1:
       add edi, 320
       add esi, edx
DTlCA_0:
       ret

DTlCA_Table dd DTlCA_0
            dd DTlCA_1
            dd DTlCA_2
            dd DTlCA_3
            dd DTlCA_4
            dd DTlCA_5
            dd DTlCA_6
            dd DTlCA_7
            dd DTlCA_8
            dd DTlCA_9
            dd DTlCA_10
            dd DTlCA_11
            dd DTlCA_12
            dd DTlCA_13
            dd DTlCA_14
            dd DTlCA_15
            dd DTlCA_16

DrawTranslucentColumnAsm64_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 10
       shl edx, 10
dtlca64_loop:
       test cx, 0FFF0h
       je dtlca64_last
       push ecx
       mov ecx, offset _G_translucentTable
       call DTlCA_16
       pop ecx
       sub cx, 010h
       jmp dtlca64_loop
dtlca64_last:
       mov eax, ecx
       mov ecx, offset _G_translucentTable
       call [dword ptr DTlCA_Table+eax*4]
       pop ebp
       ret
DrawTranslucentColumnAsm64_ EndP

DrawTranslucentColumnAsm64Old_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 10
       shl edx, 10
dlucentca_loop64:
       xor eax, eax
       shld eax, esi, 6
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip7
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip7:
       add edi, 320
       add esi, edx
       loop dlucentca_loop64
       pop ebp
       ret
DrawTranslucentColumnAsm64Old_ EndP

DrawTranslucentColumnAsm128_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 9
       shl edx, 9
dlucentca_loop128:
       xor eax, eax
       shld eax, esi, 7
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip8
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip8:
       add edi, 320
       add esi, edx
       loop dlucentca_loop128
       pop ebp
       ret
DrawTranslucentColumnAsm128_ EndP

DrawTranslucentColumnAsm256_ PROC Near
       push ebp
       mov ebp, [dword ptr _G_CurrentTexturePos]
       shl esi, 8
       shl edx, 8
dlucentca_loop256:
       xor eax, eax
       shld eax, esi, 8
       mov al, [ebp+eax]
       test al, al
       je dlucentca_skip9
       xlat
       mov ah, [edi]          ; Get the color underneath and
                              ; make the high byte
       add eax, offset _G_translucentTable
       push edi
       mov edi, eax
       mov al, [edi]          ; Get the translucent color in the table
       pop edi
       mov [edi], al
dlucentca_skip9:
       add edi, 320
       add esi, edx
       loop dlucentca_loop256
       pop ebp
       ret
DrawTranslucentColumnAsm256_ EndP

DrawTranslucentAsm_ PROC Near
       ; ecx = count
       ; esi = source
       ; edi = destination
       ; --- uses: ebx
       push ebx
       mov ebx, offset _G_translucentTable
       xor eax, eax
dtl_loop:
       mov ah, [edi]
       mov al, [esi]
       mov al, [ebx+eax]
       mov [edi], al
       inc esi
       inc edi
       loop dtl_loop
       pop ebx
       ret
DrawTranslucentAsm_ EndP

DrawTranslucentSeeThroughAsm_ PROC Near
       ; ecx = count
       ; esi = source
       ; edi = destination
       ; --- uses: ebx
       push ebx
       mov ebx, offset _G_translucentTable
       xor eax, eax
dtlst_loop:
       mov al, [esi]
       test al, 0FFh
       je dtlst_skip
       mov ah, [edi]
       mov al, [ebx+eax]
       mov [edi], al
dtlst_skip:
       inc esi
       inc edi
       loop dtlst_loop
       pop ebx
       ret
DrawTranslucentSeeThroughAsm_ EndP

DrawSeeThroughAsm_ PROC Near
       ; ecx = count
       ; esi = source
       ; edi = destination
       ; --- uses: ebx
       push ebx
       mov ebx, offset _G_translucentTable
       xor eax, eax
dst_loop:
       mov al, [esi]
       test al, 0FFh
       je dst_skip
       mov [edi], al
dst_skip:
       inc esi
       inc edi
       loop dst_loop
       pop ebx
       ret
DrawSeeThroughAsm_ EndP

ColorizeMemAsm_ PROC Near
     ; ecx = count
     ; esi = source
     ; edi = destination (can be the same as esi
     ; ebx = pointer to 256 byte translate table
colorize_loop:
       lodsb
       xlat
       stosb
       loop colorize_loop
       ret
ColorizeMemAsm_ EndP

ShadeMemAsm_ PROC Near
     ; ecx = count
     ; esi = source
     ; edi = destination
     ; ebx = pointer to 256 byte shade table entry
shade_loop:
       lodsb
       xlat
       stosb
       loop shade_loop
       ret
ShadeMemAsm_ EndP

End

