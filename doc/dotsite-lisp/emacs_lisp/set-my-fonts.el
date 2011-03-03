;; in $HOME/.emacs:
;;(add-to-list 'load-path "~/.emacs.d/")
;;(load "set-my-fonts")

(defun define-my-fixed-font (font-size)
 "trying hard to remember how many stars"
  (setq font-size
	(if (eq font-size 'nil)
	    "13"
	  (number-to-string font-size)))
  (concat
   "-*-fixed-medium-r-normal-*-"
   font-size
   "-*-*-*-*-*-"))
(defun define-my-font-for (charset-name font-name &optional last)
 "combine charset name with font name"
  (concat
   charset-name
   ":"
   font-name
   (if (eq last 'nil)
       ", ")))
(defun define-my-fontset (fontset-name &optional font-name &optional font-size)
  "define fontset to use specified fonts, fixed if not specified"
  (create-fontset-from-fontset-spec
   (if (not (eq font-name 'nil))
     (concat
      (define-my-fixed-font font-size)
      fontset-name
      ","
      (define-my-font-for "chinese-big5-1" font-name)
      (define-my-font-for "chinese-big5-2" font-name)
      (define-my-font-for "chinese-gb2312" font-name)
      (define-my-font-for "chinese-sisheng" font-name)
      (define-my-font-for "chinese-cns11643-3" font-name)
      (define-my-font-for "chinese-cns11643-4" font-name)
      (define-my-font-for "chinese-cns11643-5" font-name)
      (define-my-font-for "chinese-cns11643-6" font-name)
      (define-my-font-for "chinese-cns11643-7" font-name)
      (define-my-font-for "japanese-jisx0208" font-name)
      (define-my-font-for "japanese-jisx0208-1978" font-name)
      (define-my-font-for "japanese-jisx0212" font-name)
      (define-my-font-for "japanese-jisx0213-1" font-name)
      (define-my-font-for "japanese-jisx0213-2" font-name t))
     (concat
      (define-my-fixed-font font-size)
      fontset-name))))

(defun set-my-font-if-in-x (&optional font-name)
 "detect x window system"
  (if				       
      (not
       (eq window-system 'nil))
      (progn
	(define-my-fontset "fontset-wenquanyi" font-name)
	(set-default-font "fontset-wenquanyi")
	(add-to-list
	 'after-make-frame-functions
	 (lambda (new-frame)
	   (select-frame new-frame)
	   (set-default-font "fontset-wenquanyi"))))))

;; default
;;(define-my-fontset "fontset-fixed")
;;(set-default-font "fontset-fixed")

;; use different fonts
;;(set-my-font-if-in-x "-*-firefly new sung-medium-r-normal--12-*-iso10646-1")
;;(set-my-font-if-in-x "-*-wenquanyi bitmap song-medium-r-normal--12-*-iso10646-1")
;;(set-my-font-if-in-x "wqy16st")
(set-my-font-if-in-x "wenquanyi")

;; other settings
;;(setq initial-frame-alist '((top . 1) (left . 1) (width . 100) (height . 33)))
;;(prefer-coding-system 'utf-8)
;;(global-font-lock-mode)

;; TODO:
;; 1. use a list of charset and map operation instead of hard code function call
;; 2. detect whether specified font family is available, use only fixed then
;; 3. some faces such as tooltips at mouse over are not set
