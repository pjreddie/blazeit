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
           (process (start-file-process "blazeit" pbuffer blazeit-program)))
      (setf blazeit-buffer-process process
            blazeit-process-buffer buffer)
      process)))

(defun blazeit-parse-output (output)
  (cl-loop for line in (split-string output "\n" t)
           collect
           (cond
            ((string-match "^Input: \\(.*\\)$" line) (cons 'input (match-string 1 line)))
            ((string-match "^Left: \\(.*\\)$" line) (cons 'left (match-string 1 line)))
            ((string-match "^Right: \\(.*\\)$" line) (cons 'right (match-string 1 line)))
            ((string-match "^Type Check: Null Term$" line) (cons 'type-check nil))
            ((string-match "^Type Check: \\(.*\\)$" line) (cons 'type-check (match-string 1 line)))
            ((string-match "^Didn't Type Check!$" line) nil)
            ((string-match "^Output: \\(.*\\)$" line) (cons 'output (match-string 1 line)))
            ((string-match "^Hole should have type: \\(.*\\)$" line)
             (cons 'hole (match-string 1 line)))
            ((string-match "^~$" line) nil)
            (t (message "Unknown line: %s" line) nil))))

(defun blazeit-prettify-string (str)
  (dolist (pair blazeit-pretty-symbols str)
    (setf str (replace-regexp-in-string (regexp-opt (list (car pair))) (char-to-string (cdr pair)) str))))

(defun blazeit-run-term ()
  (interactive)
  (let ((line (current-line)) (process (blazeit-start-process (current-buffer))))
    (set-process-filter process
                        (lambda (process output)
                          (let* ((pout (blazeit-parse-output output))
                                 (successp (cdr (assoc 'type-check pout)))
                                 (holes (mapcar #'cdr (remove-if-not (lambda (x) (eq (car x) 'hole)) pout)))
                                 (output (cdr (assoc 'output pout))))
                            (if holes
                                (message "%s" (s-join "\n" (mapcar (lambda (x) (format "%s %s" (propertize "?" 'face '(:foreground "yellow")) (blazeit-prettify-string x))) (delete-dups holes))))
                                (message "%s %s"
                                         (propertize (if successp "✔" "✘") 'face
                                                     (list :foreground (if successp "green" "red")))
                                         (blazeit-prettify-string (or output "*error*")))))))
    (process-send-string process (concat line "\n"))
    (accept-process-output process 0.01)))

(provide 'blazeit-mode)
