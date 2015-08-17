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

(defvar blazeit-mode-hook
  '(prettify-symbols-mode variable-pitch-mode))

(defvar blazeit-mode-map
  (let ((map (make-keymap)))
    ;;(define-key map "\C-j" 'newline-and-typecheck)
    map)
  "Keymap for BlazeIt major mode")

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.#\\'" . blazeit-mode))

(defconst blazeit-font-lock-keywords
  '(("\\<\\(def\\|ind\\|fun\\||\\)\\>" . font-lock-keyword-face)
    ("\\<\\(\\w+\\)\\>" . font-lock-variable-name-face)))

(defconst blazeit-pretty-symbols
  '(("->" . ?→) ("=>" . ?⇒) ("fun" . "ƒ")))

(defvar blazeit-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?_ "w" st)
    st)
  "Syntax table for BlazeIt major mode")

(define-derived-mode blazeit-mode prog-mode "#"
  "Major mode for editing BlazeIt proof files"
  :syntax-table blazeit-mode-syntax-table
  (set (make-local-variable 'prettify-symbols-alist) blazeit-pretty-symbols)
  (set (make-local-variable 'font-lock-defaults) '(blazeit-font-lock-keywords))
  (font-lock-fontify-buffer))

(provide 'blazeit-mode)
