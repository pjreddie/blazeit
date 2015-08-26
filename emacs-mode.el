;;; blazeit.el --- Major mode for BlazeIt

;; Copyright (C) 2011 Pavel Panchekha <me@pavpanchekha.com>

;; Author: Pavel Panchekha <me@pavpanchekha.com>
;; Version: 0.1.0
;; Keywords: blazeit

;; This program is free software: you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation, either version 3 of the
;; License, or (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;; A major mode for the BlazeIt proof assistant.
;; Provides syntax highlighting and entities.

(defgroup blazeit nil
  "The BlazeIt theorem prover"
  :prefix "blazeit-"
  :group 'languages)

(defcustom blazeit-program "/usr/bin/blazeit"
  "The path to the blazeit executable"
  :group 'blazeit
  :type 'string)

(defvar blazeit-mode-hook
  '(prettify-symbols-mode variable-pitch-mode))

(defvar blazeit-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "C-c C-c") 'blazeit-run-term)
    map)
  "Keymap for BlazeIt major mode")

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.#\\'" . blazeit-mode))

(defconst blazeit-font-lock-keywords
  '(("\\<\\(def\\|ind\\|fun\\||\\)\\>" . font-lock-keyword-face)
    ("\\<\\(\\w+\\)\\>" . font-lock-variable-name-face)))

(defconst blazeit-pretty-symbols
  '(("->" . ?→) ("=>" . ?⇒) ("fun" . ?ƒ) ("Type" . ?★) ("." . ?○)))

(defvar blazeit-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?_ "w" st)
    st)
  "Syntax table for BlazeIt major mode")

(define-derived-mode blazeit-mode prog-mode "#"
  "Major mode for editing BlazeIt proof files"
  :syntax-table blazeit-mode-syntax-table
  :group 'blazeit
  (set (make-local-variable 'prettify-symbols-alist) blazeit-pretty-symbols)
  (set (make-local-variable 'font-lock-defaults) '(blazeit-font-lock-keywords))
  (use-local-map blazeit-mode-map)
  (font-lock-fontify-buffer))

(defvar blazeit-buffer-process nil)
(defvar blazeit-process-buffer nil)

(defun blazeit-start-process (buffer)
  (if (and (equal buffer blazeit-process-buffer)
           (eq (process-status blazeit-buffer-process) 'run))
      blazeit-buffer-process
    (let* ((pbuffer (get-buffer-create "*blazeit*"))
           (process (start-file-process "blazeit" pbuffer blazeit-program "--lisp")))
      (setf blazeit-buffer-process process
            blazeit-process-buffer buffer)
      process)))

(defun blazeit-parse-output (output)
  (cl-loop for line in (split-string output "\n" t)
           collect
           (cond
            ((string-match "^Input: \\(.*\\)$" line) (cons 'input (read (match-string 1 line))))
            ((string-match "^Left: \\(.*\\)$" line) (cons 'left (read (match-string 1 line))))
            ((string-match "^Right: \\(.*\\)$" line) (cons 'right (read (match-string 1 line))))
            ((string-match "^Type Check: \\(.*\\)$" line) (cons 'type-check (read (match-string 1 line))))
            ((string-match "^Didn't Type Check!$" line) nil)
            ((string-match "^Hole should have type: \\(.*\\)$" line)
             (cons 'hole (read (match-string 1 line))))
            ((string-match "^Output: \\(.*\\)$" line)
             (cons 'output (read (match-string 1 line))))
            ((string-match "^~$" line) nil))))

(defun blazeit-unparse (term &optional ctx)
  (pcase term
    (`nil "nil")
    (`error "*error*")
    (`(CTX (VAR ,name 0 nil nil ,type) ,(and rest `(CTX . ,_)))
     (format "%s:(%s) %s" name (blazeit-unparse type ctx) (blazeit-unparse rest  (cons name ctx))))
    (`(CTX (VAR ,name 0 nil nil ,type) ,rest)
     (format "%s:(%s) %s %s" name (blazeit-unparse type ctx)
             (propertize "⊢" 'face '(:foreground "yellow"))(blazeit-unparse rest  (cons name ctx))))
    (`(VAR nil ,n nil nil ,_a) (format "%s" (nth n ctx)))
    (`(VAR ,a ,_n nil nil ,_a) (format "%s" a))
    (`(APP nil 0 ,l ,r ,_a) (format "(%s %s)" (blazeit-unparse l ctx) (blazeit-unparse r ctx)))
    (`(FUN nil 0 (VAR ,arg 0 nil nil nil) ,body ,_a) (format "(fun %s => %s)" arg (blazeit-unparse body (cons arg ctx))))
    (`(FUN nil 0 (VAR ,arg 0 nil nil ,atype) ,body ,_a) (format "(fun %s:(%s) => %s)" arg (blazeit-unparse atype ctx) (blazeit-unparse body (cons arg ctx))))
    (`(IND ,name ,n nil nil nil . ,cases)
     (format "ind %s = %s" name (s-join " | " (mapcar (lambda (x) (blazeit-unparse x (cons name ctx))) cases))))
    (`(IND ,name ,n nil nil ,type . ,cases)
     (format "ind %s:(%s) = %s" name (blazeit-unparse type) (s-join " | " (mapcar (lambda (x) (blazeit-unparse x (cons name ctx))) cases))))
    (`(PI nil 0 (VAR _ 0 nil nil ,atype) ,body ,_a) (format "%s -> %s" (blazeit-unparse atype ctx) (blazeit-unparse body (cons '_ ctx))))
    (`(PI nil 0 (VAR ,arg 0 nil nil ,atype) ,body ,_a) (format "%s:(%s) -> %s" arg (blazeit-unparse atype ctx) (blazeit-unparse body (cons arg ctx))))
    (`(TYPE nil 0 nil nil ,_a) "Type")
    (`(CONS ,name ,n nil nil (IND ,tname ,_n nil nil ,_a . ,_cases)) (format "%s" name))
    (`(DEF ,name 0 (VAR ,name 0 nil nil nil) ,body ,_a) (format "def %s = %s" name (blazeit-unparse body ctx)))
    (`(DEF ,name 0 (VAR ,name 0 nil nil ,type) ,body ,_a) (format "def %s:(%s) = %s" name (blazeit-unparse type ctx) (blazeit-unparse body ctx)))
    (`(HOLE nil 0 nil nil ,_a) ".")
    (`(HOLE ,name 0 nil nil ,_a) (format ".%s" name))
    (`(ELIM ,name ,n ,motive ,arg . ,cases)
     (format "(%s %s %s %s)" name (blazeit-unparse motive ctx) (s-join " | " (mapcar (lambda (x) (blazeit-unparse x ctx)) cases)) (blazeit-unparse arg ctx)))
    (else (format "[[ERROR unparsing %s]]" else))))

(defun blazeit-prettify-string (str)
  (dolist (pair blazeit-pretty-symbols str)
    (setf str (replace-regexp-in-string (regexp-opt (list (car pair))) (char-to-string (cdr pair)) str))))

(defun blazeit-send-term (term)
  (let ((process (blazeit-start-process (current-buffer))) out)
    (set-process-filter process (lambda (process output) (setf out (blazeit-parse-output output))))
    (process-send-string process (concat term "\n"))
    (accept-process-output process 0.01)
    out))

(defun blazeit-run-term ()
  (interactive)
  (let* ((line (current-line))
         (pout (blazeit-send-term line))
         (successp (cdr (assoc 'type-check pout)))
         (holes (mapcar #'cdr (remove-if-not (lambda (x) (eq (car x) 'hole)) pout)))
         (output (cdr (assoc 'output pout))))
    (if holes
        (message "%s" (s-join "\n" (mapcar (lambda (x) (format "%s %s" (propertize "?" 'face '(:foreground "yellow")) (blazeit-prettify-string (blazeit-unparse x)))) (delete-dups holes))))
      (message "%s %s"
               (propertize (if successp "✔" "✘") 'face
                           `(:foreground ,(if successp "green" "red")))
               (blazeit-prettify-string (blazeit-unparse (or output 'error)))))
    (save-excursion
      (save-restriction
        (narrow-to-region (line-beginning-position) (line-end-position))
        (beginning-of-line)
        (dolist (hole holes)
          (search-forward ".")
          (replace-match (propertize "." 'help-echo (blazeit-prettify-string (blazeit-unparse hole)))))))))

(provide 'blazeit-mode)
