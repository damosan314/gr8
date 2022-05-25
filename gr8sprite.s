	;; gr8sprite.s
	;;
	;; implements a 256x192 GR8 mode - enabling the use of bytes for coordinates vs. words.
	;; 
	
	.global _init_fast
	.global _plot_pixel_256
	.global _plot_pixel_256_fast		
	.global _plot_pixel_256_opt	
	.global _plot_pixel_256_full

	;; page zero locations (stomping all over FP space)

	destination = $d4	; address of gfx buffer, used by bitmap and copy
	offset256   = $d6	; low byte of 256 column table (uses $d6/$d7)
	basegfx     = $e0	; used to tell the assembler what buffer we're writing to
	xb          = $e2	; used by plot_256, plot_line
	yb          = $e3	; used by plot_256, plot_line	
	argument    = $e6	; used by plot (destination pixel)
	fptr        = $f0	; used by init_fast
	lasty		= $f2
	mask		= $f3

	.data

	.align 256

yindexlo:	.res 192

	.align 256

yindexhi:	.res 192

	.align 256

byteoffset256_table:
	.byte 4, 4, 4, 4, 4, 4, 4, 4
	.byte 5, 5, 5, 5, 5, 5, 5, 5
	.byte 6, 6, 6, 6, 6, 6, 6, 6
	.byte 7, 7, 7, 7, 7, 7, 7, 7
	.byte 8, 8, 8, 8, 8, 8, 8, 8
	.byte 9, 9, 9, 9, 9, 9, 9, 9
	.byte 10, 10, 10, 10, 10, 10, 10, 10
	.byte 11, 11, 11, 11, 11, 11, 11, 11
	.byte 12, 12, 12, 12, 12, 12, 12, 12
	.byte 13, 13, 13, 13, 13, 13, 13, 13
	.byte 14, 14, 14, 14, 14, 14, 14, 14
	.byte 15, 15, 15, 15, 15, 15, 15, 15
	.byte 16, 16, 16, 16, 16, 16, 16, 16
	.byte 17, 17, 17, 17, 17, 17, 17, 17
	.byte 18, 18, 18, 18, 18, 18, 18, 18
	.byte 19, 19, 19, 19, 19, 19, 19, 19
	.byte 20, 20, 20, 20, 20, 20, 20, 20
	.byte 21, 21, 21, 21, 21, 21, 21, 21
	.byte 22, 22, 22, 22, 22, 22, 22, 22
	.byte 23, 23, 23, 23, 23, 23, 23, 23
	.byte 24, 24, 24, 24, 24, 24, 24, 24
	.byte 25, 25, 25, 25, 25, 25, 25, 25
	.byte 26, 26, 26, 26, 26, 26, 26, 26
	.byte 27, 27, 27, 27, 27, 27, 27, 27
	.byte 28, 28, 28, 28, 28, 28, 28, 28
	.byte 29, 29, 29, 29, 29, 29, 29, 29
	.byte 30, 30, 30, 30, 30, 30, 30, 30
	.byte 31, 31, 31, 31, 31, 31, 31, 31
	.byte 32, 32, 32, 32, 32, 32, 32, 32
	.byte 33, 33, 33, 33, 33, 33, 33, 33
	.byte 34, 34, 34, 34, 34, 34, 34, 34
	.byte 35, 35, 35, 35, 35, 35, 35, 35

	.align 256

byteoffset_table:
	.byte 0, 0, 0, 0, 0, 0, 0, 0
	.byte 1, 1, 1, 1, 1, 1, 1, 1
	.byte 2, 2, 2, 2, 2, 2, 2, 2
	.byte 3, 3, 3, 3, 3, 3, 3, 3
	.byte 4, 4, 4, 4, 4, 4, 4, 4
	.byte 5, 5, 5, 5, 5, 5, 5, 5
	.byte 6, 6, 6, 6, 6, 6, 6, 6
	.byte 7, 7, 7, 7, 7, 7, 7, 7
	.byte 8, 8, 8, 8, 8, 8, 8, 8
	.byte 9, 9, 9, 9, 9, 9, 9, 9
	.byte 10, 10, 10, 10, 10, 10, 10, 10
	.byte 11, 11, 11, 11, 11, 11, 11, 11
	.byte 12, 12, 12, 12, 12, 12, 12, 12
	.byte 13, 13, 13, 13, 13, 13, 13, 13
	.byte 14, 14, 14, 14, 14, 14, 14, 14
	.byte 15, 15, 15, 15, 15, 15, 15, 15
	.byte 16, 16, 16, 16, 16, 16, 16, 16
	.byte 17, 17, 17, 17, 17, 17, 17, 17
	.byte 18, 18, 18, 18, 18, 18, 18, 18
	.byte 19, 19, 19, 19, 19, 19, 19, 19
	.byte 20, 20, 20, 20, 20, 20, 20, 20
	.byte 21, 21, 21, 21, 21, 21, 21, 21
	.byte 22, 22, 22, 22, 22, 22, 22, 22
	.byte 23, 23, 23, 23, 23, 23, 23, 23
	.byte 24, 24, 24, 24, 24, 24, 24, 24
	.byte 25, 25, 25, 25, 25, 25, 25, 25
	.byte 26, 26, 26, 26, 26, 26, 26, 26
	.byte 27, 27, 27, 27, 27, 27, 27, 27
	.byte 28, 28, 28, 28, 28, 28, 28, 28
	.byte 29, 29, 29, 29, 29, 29, 29, 29
	.byte 30, 30, 30, 30, 30, 30, 30, 30
	.byte 31, 31, 31, 31, 31, 31, 31, 31
	.byte 32, 32, 32, 32, 32, 32, 32, 32
	.byte 33, 33, 33, 33, 33, 33, 33, 33
	.byte 34, 34, 34, 34, 34, 34, 34, 34
	.byte 35, 35, 35, 35, 35, 35, 35, 35
	.byte 36, 36, 36, 36, 36, 36, 36, 36
	.byte 37, 37, 37, 37, 37, 37, 37, 37
	.byte 38, 38, 38, 38, 38, 38, 38, 38
	.byte 39, 39, 39, 39, 39, 39, 39, 39

	.align 256

bitmask_table:
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01
	.byte $80, $40, $20, $10, $08, $04, $02, $01


	.code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; 
;;; _plot_pixel_256 & _plot_pixel_256_fast
;;;
;;; _plot_pixel_256 should be called the first time we plot on a
;;; new row.  As long as we're plotting on the same row we can
;;; call _plot_pixel_256_fast as the only item that changes is the
;;; column.
;;;
;;; assumes yindexhi/lo are page aligned
;;;
;;; stomps: x, y, a
;;; 39ish cycles
;;;
_plot_pixel_256:
	lda yb
	cmp lasty
	bne _plot_pixel_256_calcaddr
_plot_pixel_256_fast:		; call this if we're writing to same row
	ldx xb
	ldy	byteoffset256_table,x	; (4+)
r:	lda	$ffff,y	    			; (4+) load screen byte
	ora	bitmask_table,x	    	; (4+) xor it with pixel bitmask
w:	sta	$ffff,y	    			; (5) store it back to screen byte
	rts
_plot_pixel_256_calcaddr:
	sta lasty					; store the lasty
	tay			      			; (2) accum stores the row we're interested in
	lda	yindexhi,y	      		; (4+) get row address
	sta r+2
	sta w+2
	lda	yindexlo,y				; (4+)
	sta w+1
	sta r+1
	jmp _plot_pixel_256_fast

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; 
;;; _init_fast
;;;
;;; create Y index arrays
;;; 
_init_fast:
	ldy	#$00
	lda		basegfx
	sta		argument
	lda		basegfx+1
	sta		argument+1
if_loop:
	;;
	;; 	store address of line in argument
	;; 
	lda	argument
	sta	yindexlo,y
	lda	argument+1
	sta	yindexhi,y
	;;
	;; 	increment argument to next line
	;;
	clc
	lda	argument
	adc	#40
	sta	argument
	bcc	if_skip
	inc	argument+1
if_skip:
	iny
	cpy	#192
	bne	if_loop

	lda #$ff
	sta lasty

	lda	#<byteoffset256_table
	sta offset256
	lda #>byteoffset256_table
	sta	offset256+1
	rts

_plot_pixel_256_opt:
	ldy yb
	ldx byteoffset256_table,y
	lda yindexlo,y
	sta argument
	lda yindexhi,y
	sta argument+1
	lda argument,y
	ora bitmask_table,x
	sta argument,y
	rts

_plot_pixel_256_full:
	lda	#0
	sta xb
	sta yb
row_loop:
	jsr _plot_pixel_256_opt
	inc xb
	lda xb
	cmp #$ff
	bne row_loop
	lda #0
	sta xb
	inc yb
	lda yb
	cmp #192
	bne row_loop
	rts


	



