;; This buffer is for notes you don't want to save, and for Lisp evaluation.
;; If you want to create a file, visit that file with C-x C-f,
;; then enter the text in that file's own buffer.

'(concat "I am " (concat "a " "new"))
(concat "I am " (concat "a " "new") "man in " (buffer-name))

(substring "I am a newman" 2 8)

(+ 2 fill-column)

'hello

fill-column

(+)

(set 'flowers '(money1 money2))
(set 'flowers '(flowers))

(setq two 2)
(setq two (+ 2 two))
two

(setq other "*Help*")
(switch-to-buffer other-buffer)

(message "I am a newman")

'(I am a newman)

(buffer-file-name)
(buffer-name)
(current-buffer)
(switch-to-buffer (other-buffer))
(switch-to-buffer "*info*")
(other-buffer)
#<buffer *info*>

(buffer-size)
(point)

"I am a newman"
(defun multiply-by-seven (number)
  "Multiply The NUMBER by 7"
  (interactive "NInput NUMBER:")
  (message "%d" (* 7 number)))

(multiply-by-seven 7)

(defun echo-region (region)
  "ECHO your region character"
  (interactive "B")
  (message "%s" region))
(message "region")

(let ((newman 1)
      (oldman 2))
  (message "you are %d" oldman))

(defun judge-animal (ANIMAL)
  "Printf Message in echo area depend on ANIMAL"
  ;(interactive "Please Input ANIMAL: p")
  (if (equal ANIMAL 'fierce)
      (message "It's a tiger")
    (message "It's not a iger")))

(judge-animal 'fierce)

(defun double-number (NUMBER)
  "Doublue The NUMBER"
  (interactive "NDouble NUMBER:")
  (message "The Result is:%d" (* NUMBER NUMBER)))

(defun judge-fill-column (NUMBER)
  "Judge if the curren fill-colum is lower than NUMBER"
  (interactive "NTest NUMBER:")
  (if (< fill-column NUMBER)
      (message "Current fill-column is lower than your input")))

(defun simplified-begaining-of-buffer ()
  "Move the cusor at the begain of the buffer"
  (interactive)
  (push-mark)
  (goto-char (point-min)))

(defun simplified-end-to-buffer ()
  "Goto the buffer's end and mark current position"
  (interactive)
  (push-mark)
  (goto-char (point-max)))

(defun whether-buffer-exist (buff)
  "Test the buffer exist?"
  (interactive "BInput Buffer:")
  (if (get-buffer buff)
      (message "BUFFER %s EXIST" buff)
    (message "BUFFER %s DOESE NOT EXIST" buff)))

(defun append-to-buffer (buffer start end)
  "Append to specified buffer the text of the region.
  (interactive "BAppend to buffer: \nr")
  (let ((oldbuf (current-buffer)))
    (save-excursion
      (set-buffer (get-buffer-create buffer))
      (insert-buffer-substring oldbuf start end))))

(defun insert-buffer-simple (buffer)
  "Insert after point the contents of BUFFER.
     Puts mark after the inserted text.
     BUFFER may be a buffer or a buffer name."
  (interactive "*bInsert buffer: "))
(or (bufferp buffer)
    (setq buffer (get-buffer buffer)))
(let (start end newmark)
  (save-excursion
    (save-excursion
      (set-buffer buffer)
      (setq start (point-min) end (point-max)))
    (insert-buffer-substring buffer start end)
    (setq newmark (point)))
  (push-mark newmark)))

(defun beginning-of-buffer (&optional arg)
  "Move point to the beginning of the buffer; leave mark at previous position.
With arg N, put point N/10 of the way from the beginning.

If the buffer is narrowed, this command uses the beginning and size
of the accessible part of the buffer.

Don't use this command in Lisp programs!
\(goto-char (point-min)) is faster and avoids clobbering the mark."
  (interactive "P")
  (push-mark)
  (let ((size (- (point-max) (point-min))))
    (goto-char (if arg
		   (+ (point-min)
		      (if (> size 10000)
			  ;; Avoid overflow for large buffer sizes!
			  (* (prefix-numeric-value arg)
			     (/ size 10))
			(/ (+ 10 (* size (prefix-numeric-value arg))) 10)))
		 (point-min))))
  (if arg (forward-line 1)))

(defun prefix-experiment (arg)
  "Find out what is the different between P & N"
  (interactive "P") 					;;P,raw's purpose  p, N
  (message "The value you passed is %s" arg))		;;(prefix-numeric-value arg)))

(prefix-numeric-value '(3))

(defun judge-fill-column-optional (&optional NUMBER)
  "Test if the value of fill-column bigger than argument"
  (interactive "P")
  (if NUMBER
      (if (> NUMBER fill-column)
	  (message "Your number is LARGER than fill-column. %d < %d" NUMBER fill-column)
	(message "Your number is SMALLER than fill-column. %d < %d" NUMBER fill-column))
    (if (> 35 fill-column)
	(message "35 is LARGER than fill-column")
      (message "35 is SMALLER than fill-column"))))

(defun excursion-restriction ()
  "Must save-excusion outermost?"
  (interactive)
  (save-restriction
      (save-excursion
	(widen)
	(switch-to-buffer (other-buffer))
	(switch-to-buffer (other-buffer)))))


(defun what-line-modify ()
  "Print the current buffer line number and narrowed line number of point."
  (interactive)
  (let ((opoint (point)) start)
    (save-restriction					;;modified the order of re.&ex.
      (save-excursion
	(goto-char (point-min))
	(widen)
	(forward-line 0)
	(setq start (point))
	(goto-char opoint)
	(forward-line 0)
	(if (/= start 1)
	    (message "line %d (narrowed line %d)"
		     (1+ (count-lines 1 (point)))
		     (1+ (count-lines start (point))))
	  (message "Line %d" (1+ (count-lines 1 (point)))))))))


(scroll-bar-mode t)
