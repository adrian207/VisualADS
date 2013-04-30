
(in-package racer)

(defconstant +answer-marker+ ":answer")
(defconstant +ok-marker+ ":ok")
(defconstant +error-marker+ ":error")

(defparameter *default-racer-host* "localhost")
(defparameter *default-racer-tcp-port* 8088)

(defparameter *service-request-verbose* nil)
(defparameter *with-server-connection* nil)

(defparameter *socket* nil)

(defparameter *verbose-connection* t)

(defparameter *lracer-readtable* *readtable*)

(defparameter *use-unique-name-assumption* nil)

(defmacro with-unique-name-assumption (&body forms)
  `(let ((*use-unique-name-assumption* t))
    .,forms))

(defmacro without-unique-name-assumption (&body forms)
  `(let ((*use-unique-name-assumption* nil))
     .,forms))

(defvar *signature-checks* t)
  
(defmacro without-signature-checks (&body forms)
  `(let ((*signature-checks* nil))
     .,forms))

;;; ======================================================================


(defstruct (racer-boolean 
            (:predicate racer-boolean-p)
            (:conc-name boolean-))
  value)

(defmethod print-object ((object racer-boolean) stream)
  (declare (ignorable stream))
  (if (string= (boolean-value object) "false")
    (write-string "#F" stream)
    (write-string "#T" stream)))

(defparameter *true* (make-racer-boolean :value "true"))
(defparameter *false* (make-racer-boolean :value "false"))

(defmethod make-load-form ((object racer-boolean) &optional env)
  (declare (ignore env))
  (cond ((eq object *true*) '*true*)
        ((eq object *false*) '*false*)))

(defun true-value-reader (stream subchar arg)
  (declare (ignore stream subchar arg))
  *true*)

(defun false-value-reader (stream subchar arg)
  (declare (ignore stream subchar arg))
  *false*)

(defun true? (object)
  (and (racer-boolean-p object)
       (string= (boolean-value object) "true")))

(defun false? (object)
  (and (racer-boolean-p object)
       (string= (boolean-value object) "false")))


(set-dispatch-macro-character #\# #\T 'true-value-reader)
(set-dispatch-macro-character #\# #\F 'false-value-reader)
(set-dispatch-macro-character #\# #\t 'true-value-reader)
(set-dispatch-macro-character #\# #\f 'false-value-reader)



;;; ======================================================================


#+:mcl
(setf ccl::*tcp-read-timeout* 10000)

(defun open-socket (host port 
		    #+:allegro &key
		    #+:allegro (external-format excl:*default-external-format*))
  #+:allegro
  (let ((external-format (excl:find-external-format external-format))
        (socket (socket:make-socket :remote-port port :remote-host host)))
    (setf (stream-external-format socket) external-format)
    socket)
  #+:mcl
  (ccl::open-tcp-stream (if (or (string= host "localhost") (string= host "127.0.0.1"))
                            (ccl::local-interface-ip-address)
                            host)
                          port)
  #+:lispworks
  (comm:open-tcp-stream host port)
  #+:clisp
  (socket:socket-connect port host))

(defun close-socket (socket)
  (close socket)
  (setf *socket* nil))

(defun open-server-connection (&key (host *default-racer-host*)
                                    (port *default-racer-tcp-port*))
  (when *socket* (close *socket*))
  (setf *socket* (open-socket host port)))

(defun reopen-server-connection ()
  (unless *with-server-connection*
    (error "Not in a with-server-connection context."))
  (when *socket* (close *socket*))
  (setf *socket* (open-socket *default-racer-host* *default-racer-tcp-port*)))

(defun close-server-connection ()
  (when *socket*
    (close-socket *socket*)))
  

#-:mcl
(defmacro maybe-print-results (&body forms)
  `(progn .,forms))

#+:mcl
(defmacro maybe-print-results (&body forms)
  `(progn .,(mapcar (lambda (form) `(maybe-print-result (multiple-value-list ,form))) forms)))

(defun maybe-print-result (results)
  #+:ccl
  (when ccl:*verbose-eval-selection*
    (unless (null results)
      (print (first results))))
  (values-list results))

(defmacro with-server-connection ((&key tbox abox
                                        (host *default-racer-host*)
                                        (port *default-racer-tcp-port*))
                                  &body forms)
  `(let ((*with-server-connection* t)
         (*default-racer-host* ',host)
         (*default-racer-tcp-port* ',port)
         (*socket* (or *socket* (open-socket ,host ,port))))
     (unwind-protect
       (progn ,@(if tbox `((in-tbox ,tbox :init nil)) nil)
              ,@(if abox `((in-abox ,abox)) nil)
              (maybe-print-results .,forms))
       (progn
         (princ ":eof" *socket*)
         (terpri *socket*)
         (close-socket *socket*)))))



(defmacro with-standard-io-syntax-1 (&body body)
  (let ((package-sym (gensym)))
    `(let ((,package-sym *package*))
       (with-standard-io-syntax 
         (let ((*package* ,package-sym)
               (*print-case* :downcase))
           .,body)))))

(defun service-request (message &key (host *default-racer-host*) 
                                   (port *default-racer-tcp-port*)
                                   result-type
                                   (sna-error-p t)
                                   (sna-value ':no-connection))
  (when *use-unique-name-assumption*
    (setf message (format nil "(with-unique-name-assumption ~A)" message)))
  (when (null *signature-checks*)
    (setf message (format nil "(without-signature-checks ~A)" message)))
  (with-standard-io-syntax-1
    (cond ((null *socket*)
           (let ((*socket* (open-socket host port)))
             (if (null *socket*)
               (if sna-error-p
                 (error "Cannot connect to Racer.")
                 sna-value)
               (unwind-protect
                 (service-request message :result-type result-type)
                 (progn
                   (princ ":eof" *socket*)
                   (terpri *socket*)
                   (close-socket *socket*))))))
          (t (when *service-request-verbose*
               (format t "~A~%" message))
             (format *socket* "~A" message)
	     (terpri *socket*)
             ;;(princ #\Linefeed *socket*)
             (force-output *socket*)
             (parse-racer-reply (read-line-1 *socket*) result-type)))))

(defun parse-racer-reply (message &optional result-type)
  (let* ((answer (search +answer-marker+ message))
         (ok (search +ok-marker+ message))
         (error (search +error-marker+ message))
         (*readtable* *lracer-readtable*)
         (old-readtable-case (readtable-case *readtable*)))
    (unwind-protect
      (progn 
        (setf (readtable-case *readtable*) :preserve)
        (cond (answer
               (multiple-value-bind (number pos-1)
                                    (read-from-string message t nil
                                                      :start (length +answer-marker+))
                 (declare (ignore number))
                 (multiple-value-bind (result pos-2)
                                      (extract-string message pos-1)
                   (let* ((final-result
                           (transform-result 
                            (case result-type
                              (symbol (intern (remove-bars result)))
                              (otherwise 
                               (read-from-string result)))))
                          (message
                           (substitute #\Newline #\Tab
				       (read-from-string 
				        (substitute #\/ #\\
						    message)
				        t nil :start pos-2))))
                     (when (and *verbose-connection* 
                                (not (string= message "")))
                       (princ message))
                     final-result))))
              (ok 
               (multiple-value-bind (number pos-1)
                                    (read-from-string message t nil 
                                                      :start (length +ok-marker+))
                 (declare (ignore number))
                 (let ((message
                        (substitute #\Newline #\Tab
                                    (read-from-string 
				     (substitute #\/ #\\
					         message)
				     t nil :start pos-1))))
                   (when (and *verbose-connection* 
                              (not (string= message "")))
                     (princ message))
                   t)))
              (error
               (multiple-value-bind (number pos-1)
                                    (read-from-string message t nil 
                                                      :start (length +error-marker+))
                 (declare (ignore number))
                 (error (substitute #\- #\~ (subseq message pos-1)))))
              (t (error "Unknown reply message format from Racer: ~A. ~
                         Probably the server connection is not set up properly." message))))
      (setf (readtable-case *readtable*) old-readtable-case))))

(defun extract-string (message start) 
  (let ((end (find-end-position message)))
    (values (transform (subseq message (1+ start) end))
            (1+ end))))

(defun transform (string)
  (let* ((length (length string))
         (result (make-array length :adjustable t :element-type 'character :fill-pointer 0)))
    (loop with skip = nil
          for i from 0 below length 
          for ch = (aref string i) do
          (if skip
              (setf skip nil)
            (if (char= ch #\\)
                (if (< i (1- length))
                    (let ((next-ch (aref string (1+ i))))
                      ;(terpri)
                      ;(princ next-ch)
                      (cond ((char= next-ch #\") ;(princ "\\\" replaced by \"") (terpri)
                             (setf skip t)
                             (vector-push-extend #\" result))
                            ((char= next-ch #\S) ;(princ "\\S replaced by \"") (terpri)
                             (setf skip t)
                             (vector-push-extend #\\ result)
                             (vector-push-extend #\" result))
                            ((char= next-ch #\N) ;(princ "\\N replaced by Newline") (terpri)
                             (setf skip t)
                             (vector-push-extend #\Newline result))
                            ((char= next-ch #\\) ;(princ "\\\\ replaced by \\\\") (terpri)
                             (setf skip t)
                             (vector-push-extend #\\ result)
                             (vector-push-extend #\\ result))
                            ((char= next-ch #\|) ;(princ "\| replaced by \|") (terpri)
                             (setf skip t)
                             (vector-push-extend #\| result))
                            (t (error "Unknown escape sequence found in ~A." string))))
                  (vector-push-extend ch result))
              (vector-push-extend ch result))))
    result))

(defun find-end-position (message)
  (let* ((pos1 (position #\" message :from-end t))
         (pos2 (position #\" message :end pos1 :from-end t)))
    (position #\" message :end pos2 :from-end t)))


(defun remove-bars (string)
  (let ((len (length string)))
    (if (and (char= (aref string 0) #\|) (char= (aref string (1- len)) #\|))
      (subseq string 1 (1- len))
      string)))

#|
(defun read-line-1 (input-stream &optional eof-value)
  (let ((line nil))
    (loop for char = (read-char input-stream nil t)
          do 
          (cond ((eq char 't)
                 (return-from read-line-1 eof-value))
                ((char= char #\Linefeed)
                 (return-from read-line-1 (coerce (reverse line) 'string)))
                (t
                 (push char line))))))
|#

(defun read-line-1 (input-stream &optional eof-value)
  (let ((line nil))
    (loop for char = (read-char input-stream nil t)
          do 
          (cond ((eq char #\<)

                 ;;; Unreadable Object? 
                 ;;; Dann bis zum schliessenden #\> lesen, 
                 ;;; dann weiter.

                 ;;; #<....>; # entfernen

                 (pop line)

                 ;;; mach einen String daraus!

                 (push #\" line)

                 (loop for  char = (read-char input-stream nil t)
                       do 
                       (progn 
                         ;;;(princ char) 
                         (cond ((char= char #\>)
                                ;;; und weiter lesen
                                (push #\" line)
                                (return))
                               ((eq char t)
                                (return-from read-line-1 eof-value))
                               (t (push char line))))))

                ((eq char 't)
                 (return-from read-line-1 eof-value))
                ((or (char= char #\Linefeed)
                     (char= char #\Newline)
                     (char= char #\Return))
                 (return-from read-line-1 (coerce (reverse line) 'string)))
                (t
                 (push char line))))))

(defun transform-result (s-expr)
  (cond ((null s-expr)
         nil)
        ((consp s-expr)
         (mapcar #'transform-result s-expr))
        ((symbolp s-expr)
         (if (eq (symbol-package s-expr) (find-package "RACER-STRING"))
           (symbol-name s-expr)
           (let* ((symbol-name (symbol-name s-expr))
                  (pos-colons (search "::" symbol-name))
                  (package-name (and pos-colons
                                     (subseq symbol-name 
                                             0 
                                             pos-colons)))
                  (symbol-name-1 (and pos-colons
                                      (subseq symbol-name (+ pos-colons 2)))))
             (if (null package-name)
               s-expr
               (intern symbol-name-1 (find-package package-name))))))
        (t s-expr)))

(defun transform-s-expr (s-expr)
  (cond ((null s-expr)
         nil)
        ((eq s-expr t)
         t)
        ((consp s-expr)
         (mapcar #'transform-s-expr s-expr))
        ((symbolp s-expr)
         (let ((package-name (package-name (symbol-package s-expr))))
           (if (or (string= package-name "RACER")
                   (string= package-name "racer")
                   (string= package-name "COMMON-LISP")
                   (string= package-name "common-lisp")
                   (string= package-name "KEYWORD")
                   (string= package-name "keyword")
                   (string= package-name "CL-USER")
                   (string= package-name "cl-user")
                   (string= package-name "COMMON-LISP-USER")
                   (string= package-name "common-lisp-user")
                   (string= package-name "RACER-USER")
                   (string= package-name "racer-user")
                   (string= package-name "NRQL-SYMBOLS")
		   (string= package-name "nrql-symbols"))
             s-expr
             (intern (concatenate 'string
                                  (string-downcase package-name)
                                  "::"
                                  (string-downcase (symbol-name s-expr)))))))
        (t s-expr)))

;;; ======================================================================

(defmacro in-tbox (tbox-name &rest args)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(in-tbox ~S~{ ~S~})" 
                               (transform-s-expr tbox-name)
                               (transform-s-expr args)))))

(defun set-current-tbox (tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(in-tbox ~S :init nil)" 
                             (transform-s-expr tbox-name)))))

(defun init-tbox (tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(in-tbox ~S)" (transform-s-expr tbox-name)))))

(defmacro delete-tbox (tbox-name)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(delete-tbox ~S)" (transform-s-expr tbox-name)))))

(defun forget-tbox (tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(delete-tbox ~S)" (transform-s-expr tbox-name)))))

(defun delete-all-tboxes ()
  (with-standard-io-syntax-1
    (service-request "(delete-all-tboxes)")))

(defun clear-default-tbox ()
  (with-standard-io-syntax-1
    (service-request "(clear-default-tbox)")))

(defun current-tbox ()
  (with-standard-io-syntax-1
    (service-request "(current-tbox)")))

(defun current-abox ()
  (with-standard-io-syntax-1
    (service-request "(current-abox)")))

(defmacro in-abox (abox-name &optional tbox-name)
  (with-standard-io-syntax-1
    (if tbox-name
      `(service-request ,(format nil "(in-abox ~S ~S)" 
                                 (transform-s-expr abox-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(in-abox ~S)" 
                                 (transform-s-expr abox-name))))))

(defun init-abox (abox-name &optional tbox-name)
  (with-standard-io-syntax-1
    (if tbox-name
      (service-request (format nil "(in-abox ~S ~S)" 
                               (transform-s-expr abox-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(in-abox ~S)" 
                               (transform-s-expr abox-name))))))

(defun set-current-abox (abox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(in-abox ~S)" 
                             (transform-s-expr abox-name)))))

(defmacro delete-abox (abox-name)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(delete-abox ~S)" (transform-s-expr abox-name)))))

(defun forget-abox (tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-abox ~S)" (transform-s-expr tbox-name)))))

(defun delete-all-aboxes ()
  (with-standard-io-syntax-1
    (service-request "(delete-all-aboxes)")))

(defmacro in-knowledge-base (&rest args)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(in-knowledge-base~{ ~S~})" (transform-s-expr args)))))

(defun ensure-tbox-signature (tbox-name 
                                  &key atomic-concepts
                                  roles
                                  transitive-roles
                                  features
                                  attributes)
  (with-standard-io-syntax-1
    (service-request (format nil "(ensure-tbox-signature ~S ~
                                  :atomic-concepts ~S ~
                                  :roles ~S ~
                                  :transitive-roles ~S ~
                                  :features ~S ~
                                  :attributes ~S)"
                             (transform-s-expr tbox-name)
                             (transform-s-expr atomic-concepts) 
                             (transform-s-expr roles)
                             (transform-s-expr transitive-roles)
                             (transform-s-expr features)
                             (transform-s-expr attributes)))))

(defun ensure-abox-signature (abox-name
                              &key individuals
                              objects)
  (with-standard-io-syntax-1
    (service-request (format nil "(ensure-abox-signature ~S ~
                                  :individuals ~S ~
                                  :objects ~S)"
                             (transform-s-expr abox-name)
                             (transform-s-expr individuals)
                             (transform-s-expr objects)))))

(defmacro signature (&key atomic-concepts
                          roles
                          transitive-roles
                          features
                          attributes individuals objects)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(signature ~
                                    :atomic-concepts ~S ~
                                    :roles ~S ~
                                    :transitive-roles ~S ~
                                    :features ~S ~
                                    :attributes ~S ~
                                    :individuals ~S ~
                                    :objects ~S)"
                               (transform-s-expr atomic-concepts) 
                               (transform-s-expr roles) 
                               (transform-s-expr transitive-roles)
                               (transform-s-expr features)
                               (transform-s-expr attributes)
                               (transform-s-expr individuals)
                               (transform-s-expr objects)))))

(defmacro implies (left right)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(implies ~S ~S)" (transform-s-expr left)
                               (transform-s-expr right)))))

(defmacro define-primitive-concept (name &optional definition)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-primitive-concept ~S ~S)" 
                               (transform-s-expr name) (transform-s-expr definition)))))

(defmacro defprimconcept (name &optional definition)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-primitive-concept ~S ~S)" 
                               (transform-s-expr name)
                               (transform-s-expr definition)))))

(defun add-concept-axiom (tbox left right &key inclusion-p)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-concept-axiom ~S ~S ~S :inclusion-p ~S)"
                             (transform-s-expr tbox)
                             (transform-s-expr left)
                             (transform-s-expr right)
                             (transform-s-expr inclusion-p)))))

(defun add-role-axioms (tbox role-name  &key cd-attribute feature-p feature transitive-p transitive parents 
                             parent inverse inverse-feature-p domain range symmetric reflexive)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-role-axioms ~S ~S ~
                                  :cd-attribute ~S ~
                                  :feature-p ~S ~
                                  :feature ~S ~
                                  :transitive-p ~S ~
                                  :transitive ~S ~
                                  :parent ~S ~
                                  :parents ~S ~
                                  :inverse ~S ~
                                  :inverse-feature-p ~S ~
                                  :domain ~S ~
                                  :range ~S ~
                                  :symmetric ~S ~
                                  :reflexive ~S)"
                             (transform-s-expr tbox)
                             (transform-s-expr role-name)
                             (transform-s-expr cd-attribute)
                             (transform-s-expr feature-p)
                             (transform-s-expr feature)
                             (transform-s-expr transitive-p)
                             (transform-s-expr transitive) 
                             (transform-s-expr parents)
                             (transform-s-expr parent)
                             (transform-s-expr inverse)
                             (transform-s-expr inverse-feature-p)
                             (transform-s-expr domain)
                             (transform-s-expr range) 
                             (transform-s-expr symmetric)
                             (transform-s-expr reflexive)))))

(defmacro equivalent (left right)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(equivalent ~S ~S)" 
                               (transform-s-expr left)
                               (transform-s-expr right)))))

(defmacro define-concept (left right)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-concept ~S ~S)" 
                               (transform-s-expr left)
                               (transform-s-expr right)))))

(defun add-disjointness-axiom (tbox concept-name group-name  &optional form)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-disjointness-axiom ~S ~S ~S ~
                                ~{ ~S~})" 
                           (transform-s-expr tbox)
                           (transform-s-expr concept-name)
                           (transform-s-expr group-name)
                           (transform-s-expr form)))))

(defmacro disjoint (&rest concept-names)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(disjoint ~{ ~S~})"
                             (transform-s-expr concept-names)))))

(defmacro define-disjoint-primitive-concept (name disjoint-list definition)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-disjoint-primitive-concept ~S ~S ~S)" 
                              (transform-s-expr name)
                              (transform-s-expr disjoint-list)
                              (transform-s-expr definition)))))

(defmacro define-primitive-role (&rest params)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-primitive-role ~{ ~S~})" (transform-s-expr params))))) 

(defmacro defprimrole (&rest params)
  (with-standard-io-syntax-1
    `(define-primitive-role .,(transform-s-expr params))))

(defmacro define-distinct-individual (individual-name  &optional (concept '+top-symbol+))
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-distinct-individual ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr concept)))))

(defmacro define-individual (individual-name  &optional (concept '+top-symbol+))
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-individual ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr concept)))))

(defmacro define-primitive-attribute (name  &key (parent nil)
                                            (parents nil)
                                            (domain nil)
                                            (range nil)
                                            (inverse nil)
                                            (symmetric nil))
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-primitive-attribute ~S ~
                                    :parent ~S ~
                                    :parents ~S ~
                                    :domain ~S ~
                                    :range ~S ~
                                    :inverse ~S ~
                                    :symmetric ~S)"
                               (transform-s-expr name)
                               (transform-s-expr parent)
                               (transform-s-expr parents)
                               (transform-s-expr domain)
                               (transform-s-expr range)
                               (transform-s-expr inverse)
                               (transform-s-expr symmetric)))))

(defmacro define-concrete-domain-attribute (name &key domain type)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-concrete-domain-attribute ~S ~
                                    :domain ~S ~
                                    :type ~S)"
                               (transform-s-expr name)
                               (transform-s-expr domain)
                               (transform-s-expr type)))))

(defmacro define-datatype-property (&rest params)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-datatype-property ~{ ~S~})" (transform-s-expr params))))) 

(defun add-datatype-property (&rest params)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-datatype-property ~{ ~S~})" (transform-s-expr params)))))

(defmacro state (&body forms)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(state ~{ ~S~})" (transform-s-expr forms)))))

(defmacro instance (name concept)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(instance ~S ~S)" 
                               (transform-s-expr name) (transform-s-expr concept)))))

(defun add-concept-assertion (abox individual-name concept)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-concept-assertion ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr individual-name)
                             (transform-s-expr concept)))))

(defmacro related (left-name right-name role-name)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(related ~S ~S ~S)" 
                               (transform-s-expr left-name)
                               (transform-s-expr right-name)
                               (transform-s-expr role-name)))))

(defun add-role-assertion (abox predecessor-name filler-name role-term)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-role-assertion ~S ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr predecessor-name)
                             (transform-s-expr filler-name)
                             (transform-s-expr role-term)))))


(defmacro constraints (&body forms)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(constraints ~{ ~S~})" (transform-s-expr forms)))))

(defun add-constraint-assertion (abox-name constraint)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-constraint-assertion ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr constraint)))))

(defmacro constrained (individual object attribute)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(constrained ~S ~S ~S)" 
                               (transform-s-expr individual)
                               (transform-s-expr object)
                               (transform-s-expr attribute)))))

(defun add-attribute-assertion (abox-name
                                    individual
                                    object
                                    attribute)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-attribute-assertion ~S ~S ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr individual)
                             (transform-s-expr object)
                             (transform-s-expr attribute)))))



(defmacro forget ((&key (tbox nil tbox-supplied-p)
                        (abox nil abox-supplied-p))
                  &body assertions)
  (with-standard-io-syntax-1
    (cond ((and tbox-supplied-p abox-supplied-p)
           `(service-request ,(format nil "(forget (:tbox ~S :abox ~S)~
                                           ~{ ~S~})" (transform-s-expr tbox)
                                      (transform-s-expr abox)
                                      (transform-s-expr assertions))))
          (tbox-supplied-p
           `(service-request ,(format nil "(forget (:tbox ~S)~
                                           ~{ ~S~})" (transform-s-expr tbox)
                                      (transform-s-expr assertions))))
          (abox-supplied-p
           `(service-request ,(format nil "(forget (:abox ~S)~
                                           ~{ ~S~})" (transform-s-expr abox)
                                      (transform-s-expr assertions))))
          (t `(service-request ,(format nil "(forget ()~
                                             ~{ ~S~})" (transform-s-expr assertions)))))))

(defun forget-concept-axiom (tbox left right &key (inclusion-p nil))
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-concept-axiom ~S ~S ~S :inclusion-p ~S)" 
                             (transform-s-expr tbox)
                             (transform-s-expr left)
                             (transform-s-expr right)
                             (transform-s-expr inclusion-p)))))


(defun forget-role-axioms (tbox role &key
                                   (cd-attribute nil)
                                   (parents nil) 
                                   (parent nil)
                                   (transitive nil)
                                   (transitive-p nil)
                                   (feature nil)
                                   (feature-p nil)
                                   (domain nil)
                                   (range nil)
                                   (inverse nil)
                                   (symmetric nil)
                                   (reflexive nil)
                                   (reflexive-p nil)
                                   (datatype nil)
                                   (annotation-p nil))
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-concept-axiom ~S ~S :cd-attribute ~S :parents ~S :parent ~S :transitive ~S  :transitive-p ~S :feature ~S :feature-p ~S :domain ~S :range ~S :inverse ~S :symmetric ~S :reflexive ~S :reflexive-p ~S :datatype ~S :annotation-p ~S)" 
                             (transform-s-expr tbox)
                             (transform-s-expr role)
                             (transform-s-expr cd-attribute)
                             (transform-s-expr parents)
                             (transform-s-expr parent)
                             (transform-s-expr transitive)
                             (transform-s-expr transitive-p)
                             (transform-s-expr feature)
                             (transform-s-expr feature-p)
                             (transform-s-expr domain)
                             (transform-s-expr range)
                             (transform-s-expr inverse)
                             (transform-s-expr symmetric)
                             (transform-s-expr reflexive)
                             (transform-s-expr reflexive-p)
                             (transform-s-expr datatype)
                             (transform-s-expr annotation-p)))))

(defun forget-statement (tbox abox assertions)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-statement ~S ~S ~S)" 
                             (transform-s-expr tbox)
                             (transform-s-expr abox)
                             (transform-s-expr assertions)))))

(defun forget-concept-assertion (abox individual-name concept)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-concept-assertion ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr individual-name)
                             (transform-s-expr concept)))))

(defun forget-role-assertion (abox predecessor-name filler-name role-term)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-role-assertion ~S ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr predecessor-name)
                             (transform-s-expr filler-name)
                             (transform-s-expr role-term)))))


(defun forget-disjointness-axiom  (tbox concept-name group-name  &optional form)
  (with-standard-io-syntax-1
    (if form
      (service-request (format nil "(forget-disjointness-axiom ~S ~S ~S ~S)" 
                               (transform-s-expr tbox)
                               (transform-s-expr concept-name)
                               (transform-s-expr group-name)
                               (transform-s-expr form)))
      (service-request (format nil "(forget-disjointness-axiom ~S ~S ~S)" 
                               (transform-s-expr tbox)
                               (transform-s-expr concept-name)
                               (transform-s-expr group-name))))))

(defun forget-disjointness-axiom-statement  (tbox concepts)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-disjointness-axiom-statement ~S ~S)" 
                             (transform-s-expr tbox)
                             (transform-s-expr concepts)))))

(defun forget-individual (individual-name &optional (abox nil abox-supplied-p))

  (with-standard-io-syntax-1
    (if abox-supplied-p
      (service-request (format nil "(forget-individual ~S ~S)" 
                               (transform-s-expr individual-name)
                               (transform-s-expr abox)))
      (service-request (format nil "(forget-individual ~S)" 
                               (transform-s-expr individual-name))))))


(defmacro concept-satisfiable? (concept-1 
                                &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-satisfiable? ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-satisfiable? ~S)"
                                 (transform-s-expr concept-1))))))

(defun concept-satisfiable-p (concept-term tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(concept-satisfiable-p ~S ~S)" 
                             (transform-s-expr concept-term)
                             (transform-s-expr tbox)))))

(defmacro concept-subsumes? (concept-1 concept-2
                                       &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-subsumes? ~S ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr concept-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-subsumes? ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr concept-2))))))

(defun concept-subsumes-p (subsumer subsumee tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(concept-subsumes-p ~S ~S ~S)" 
                             (transform-s-expr subsumer)
                             (transform-s-expr subsumee)
                             (transform-s-expr tbox)))))

(defmacro concept-equivalent? (concept-1 concept-2
                                         &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-equivalent? ~S ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr concept-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-equivalent? ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr concept-2))))))

(defun concept-equivalent-p (subsumer subsumee tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(concept-equivalent-p ~S ~S ~S)" 
                             (transform-s-expr subsumer)
                             (transform-s-expr subsumee)
                             (transform-s-expr tbox)))))

(defmacro concept-disjoint? (concept-1 concept-2
                                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-disjoint? ~S ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr concept-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-disjoint? ~S ~S)"
                                 (transform-s-expr concept-1)
                                 (transform-s-expr concept-2))))))

(defun concept-disjoint-p (subsumer subsumee tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(concept-disjoint-p ~S ~S ~S)" 
                             (transform-s-expr subsumer)
                             (transform-s-expr subsumee)
                             (transform-s-expr tbox)))))

(defmacro concept? (concept-name 
                    &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept? ~S ~S)"
                                 (transform-s-expr concept-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept? ~S)"
                                 (transform-s-expr concept-name))))))

(defun concept-p (concept-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(concept-p ~S ~S)"
                               (transform-s-expr concept-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(concept-p ~S)"
                               (transform-s-expr concept-name))))))

(defmacro concept-is-primitive? (concept-name 
                    &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-is-primitive? ~S ~S)"
                                 (transform-s-expr concept-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-is-primitive? ~S)"
                                 (transform-s-expr concept-name))))))

(defun concept-is-primitive-p (concept-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(concept-is-primitive-p ~S ~S)"
                               (transform-s-expr concept-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(concept-is-primitive-p ~S)"
                               (transform-s-expr concept-name))))))

(defun alc-concept-coherent (concept-term &key (logic :k))
  (with-standard-io-syntax-1
    (service-request (format nil "(alc-concept-coherent ~S :logic ~s)"
                             (transform-s-expr concept-term)
                             (transform-s-expr logic)))))


(defmacro role-subsumes? (role-term-1 role-term-2
                                       &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-subsumes? ~S ~S ~S)"
                                 (transform-s-expr role-term-1)
                                 (transform-s-expr role-term-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-subsumes? ~S ~S)"
                                 (transform-s-expr role-term-1)
                                 (transform-s-expr role-term-2))))))

(defun role-subsumes-p (role-term-1 role-term-2 tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-subsumes-p ~S ~S ~S)" 
                             (transform-s-expr role-term-1)
                             (transform-s-expr role-term-2)
                             (transform-s-expr tbox)))))


(defmacro role? (role-name 
                    &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role? ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role? ~S)"
                                 (transform-s-expr role-name))))))

(defun role-p (role-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(role-p ~S ~S)"
                               (transform-s-expr role-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(role-p ~S)"
                               (transform-s-expr role-name))))))

(defmacro transitive? (role-name 
                    &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(transitive? ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(transitive? ~S)"
                                 (transform-s-expr role-name))))))

(defun transitive-p (role-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(transitive-p ~S ~S)"
                               (transform-s-expr role-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(transitive-p ~S)"
                               (transform-s-expr role-name))))))

(defmacro feature? (role-name 
                    &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(feature? ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(feature? ~S)"
                                 (transform-s-expr role-name))))))

(defun feature-p (role-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(feature-p ~S ~S)"
                               (transform-s-expr role-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(feature-p ~S)"
                               (transform-s-expr role-name))))))

(defmacro cd-attribute? (role-name 
                    &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(cd-attribute? ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(cd-attribute? ~S)"
                                 (transform-s-expr role-name))))))

(defun cd-attribute-p (role-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(cd-attribute-p ~S ~S)"
                               (transform-s-expr role-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(cd-attribute-p ~S)"
                               (transform-s-expr role-name))))))

(defun classify-tbox (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(classify-tbox ~S)"
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(classify-tbox)")))))

(defun check-tbox-coherence (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(check-tbox-coherence ~S)"
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(check-tbox-coherence)")))))

(defmacro tbox-classified? (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(tbox-classified? ~S)"
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(tbox-classified?)")))))

(defun tbox-classified-p (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(tbox-classified-p ~S)"
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(tbox-classified-p)")))))

(defmacro tbox-coherent? (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(tbox-coherent? ~S)"
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(tbox-coherent?)")))))

(defun tbox-coherent-p (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(tbox-coherent-p ~S)"
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(tbox-coherent-p)")))))

(defun realize-abox (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(realize-abox ~S)"
                               (transform-s-expr abox-name)))
      (service-request (format nil "(realize-abox)")))))

(defmacro abox-realized? (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(abox-realized? ~S)"
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(abox-realized?)")))))

(defun abox-realized-p (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(abox-realized-p ~S)"
                               (transform-s-expr abox-name)))
      (service-request (format nil "(abox-realized-p)")))))

(defmacro abox-consistent? (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(abox-consistent? ~S)"
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(abox-consistent?)")))))

(defun abox-consistent-p (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(abox-consistent-p ~S)"
                               (transform-s-expr abox-name)))
      (service-request (format nil "(abox-consistent-p)")))))

(defmacro abox-una-consistent? (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(abox-una-consistent? ~S)"
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(abox-una-consistent?)")))))

(defun abox-una-consistent-p (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(abox-una-consistent-p ~S)"
                               (transform-s-expr abox-name)))
      (service-request (format nil "(abox-una-consistent-p)")))))

(defmacro abox-prepared? (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(abox-prepared? ~S)"
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(abox-prepared?)")))))

(defun abox-prepared-p (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(abox-prepared-p ~S)"
                               (transform-s-expr abox-name)))
      (service-request (format nil "(abox-prepared-p)")))))

(defun check-abox-coherence (&optional (abox-name nil abox-name-supplied-p) (filename nil))
  (let ((*verbose-connection* t))
    (with-standard-io-syntax-1
      (if abox-name-supplied-p
        (service-request (format nil "(check-abox-coherence ~S ~S)"
                                 (transform-s-expr abox-name)
                                 (transform-s-expr filename)))
        (service-request (format nil "(check-abox-coherence)"))))))

(defmacro individual? (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual? ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual? ~S)" 
                                 (transform-s-expr individual-name))))))

(defun individual-p (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(individual-p ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(individual-p ~S)"
                               (transform-s-expr individual-name))))))

(defmacro individual-instance? (individual-name concept &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-instance? ~S ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr concept)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-instance? ~S ~S)" 
                                 (transform-s-expr individual-name)
                                 (transform-s-expr concept))))))

(defun individual-instance-p (individual-name concept &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(individual-instance-p ~S ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr concept)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(individual-instance-p ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr concept))))))

(defmacro individuals-related? (individual-1 individual-2 role-term &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individuals-related? ~S ~S ~S ~S)"
                                 (transform-s-expr individual-1)
                                 (transform-s-expr individual-2)
                                 (transform-s-expr role-term)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individuals-related? ~S ~S ~S)" 
                                 (transform-s-expr individual-1)
                                 (transform-s-expr individual-2)
                                 (transform-s-expr role-term))))))

(defun individuals-related-p (individual-1 individual-2 role-term &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(individuals-related-p ~S ~S ~S ~S)"
                               (transform-s-expr individual-1)
                               (transform-s-expr individual-2)
                               (transform-s-expr role-term)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(individuals-related-p ~S ~S ~S)"
                               (transform-s-expr individual-1)
                               (transform-s-expr individual-2)
                               (transform-s-expr role-term))))))

(defmacro individuals-equal? (individual-1 individual-2 &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individuals-equal? ~S ~S ~S)"
                                 (transform-s-expr individual-1)
                                 (transform-s-expr individual-2)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individuals-equal? ~S ~S)" 
                                 (transform-s-expr individual-1)
                                 (transform-s-expr individual-2))))))

(defun individuals-equal-p (individual-1 individual-2 &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(individuals-equal-p ~S ~S ~S)"
                               (transform-s-expr individual-1)
                               (transform-s-expr individual-2)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(individuals-equal-p ~S ~S)"
                               (transform-s-expr individual-1)
                               (transform-s-expr individual-2))))))

(defmacro individuals-not-equal? (individual-1 individual-2 &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individuals-not-equal? ~S ~S ~S)"
                                 (transform-s-expr individual-1)
                                 (transform-s-expr individual-2)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individuals-not-equal? ~S ~S)" 
                                 (transform-s-expr individual-1)
                                 (transform-s-expr individual-2))))))

(defun individuals-not-equal-p (individual-1 individual-2 &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(individuals-not-equal-p ~S ~S ~S)"
                               (transform-s-expr individual-1)
                               (transform-s-expr individual-2)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(individuals-not-equal-p ~S ~S)"
                               (transform-s-expr individual-1)
                               (transform-s-expr individual-2))))))

(defmacro concept-synonyms (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-synonyms ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-synonyms ~S)"
                                 (transform-s-expr concept))))))

(defun atomic-concept-synonyms (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-concept-synonyms ~S ~S)"
                               (transform-s-expr concept)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-concept-synonyms ~S)"
                               (transform-s-expr concept))))))

(defmacro concept-descendants (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-descendants ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-descendants ~S)"
                                 (transform-s-expr concept))))))

(defun atomic-concept-descendants (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-concept-descendants ~S ~S)"
                               (transform-s-expr concept)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-concept-descendants ~S)"
                               (transform-s-expr concept))))))

(defmacro concept-ancestors (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-ancestors ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-ancestors ~S)"
                                 (transform-s-expr concept))))))

(defun atomic-concept-ancestors (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-concept-ancestors ~S ~S)"
                               (transform-s-expr concept)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-concept-ancestors ~S)"
                               (transform-s-expr concept))))))

(defmacro concept-parents (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-parents ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-parents ~S)"
                                 (transform-s-expr concept))))))

(defun atomic-concept-parents (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-concept-parents ~S ~S)"
                               (transform-s-expr concept)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-concept-parents ~S)"
                               (transform-s-expr concept))))))

(defmacro concept-children (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(concept-children ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(concept-children ~S)"
                                 (transform-s-expr concept))))))

(defun atomic-concept-children (concept &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-concept-children ~S ~S)"
                               (transform-s-expr concept)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-concept-children ~S)"
                               (transform-s-expr concept))))))

(defmacro role-synonyms (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-synonyms ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-synonyms ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-synonyms (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-synonyms ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-synonyms ~S)"
                               (transform-s-expr role))))))

(defmacro role-descendants (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-descendants ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-descendants ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-descendants (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-descendants ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-descendants ~S)"
                               (transform-s-expr role))))))

(defmacro role-ancestors (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-ancestors ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-ancestors ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-ancestors (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-ancestors ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-ancestors ~S)"
                               (transform-s-expr role))))))

(defmacro role-parents (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-parents ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-parents ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-parents (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-parents ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-parents ~S)"
                               (transform-s-expr role))))))

(defmacro role-children (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-children ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-children ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-children (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-children ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-children ~S)"
                               (transform-s-expr role))))))

(defmacro role-inverse (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-inverse ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-inverse ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-inverse (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-inverse ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-inverse ~S)"
                               (transform-s-expr role))))))

(defun all-tboxes ()
  (with-standard-io-syntax-1
    (service-request "(all-tboxes)")))

(defun find-tbox (tbox-name &optional (error-p t))
  (with-standard-io-syntax-1
    (service-request (format nil "(find-tbox ~S ~S)" (transform-s-expr tbox-name)
                             (transform-s-expr error-p)))))

(defun set-find-tbox (tbox-name new-value)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-find-tbox ~S ~S)" 
                             (transform-s-expr tbox-name)
                             (transform-s-expr new-value)))))

(defun (setf find-tbox) (new-value tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-find-tbox ~S ~S)" 
                             (transform-s-expr tbox-name)
                             (transform-s-expr new-value)))))

(defun all-atomic-concepts (&optional (tbox-name nil tbox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(all-atomic-concepts ~S :count ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-atomic-concepts)")))))  

(defun all-equivalent-concepts (&optional (tbox-name nil tbox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(all-equivalent-concepts ~S :count ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-equivalent-concepts)")))))  



(defun role-used-as-datatype-property-p (role-name tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-used-as-datatype-property-p ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox)))))


(defun role-is-used-as-datatype-property (role-name tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-is-used-as-datatype-property ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox)))))


(defun datatype-role-range (role-name tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(datatype-role-range ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox)))))

(defun datatype-role-has-range (role-name range tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(datatype-role-has-range ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr range)
                             (transform-s-expr tbox)))))





(defun all-roles (&optional (tbox-name nil tbox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(all-roles ~S :count ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-roles)")))))

(defun all-features (&optional (tbox-name nil tbox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(all-features ~S :count ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-features)")))))

(defun all-transitive-roles (&optional (tbox-name nil tbox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(all-transitive-roles ~S :count ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-transitive-roles)")))))

(defun all-attributes (&optional (tbox-name nil tbox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(all-attributes ~S :count ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-attributes)")))))

(defun attribute-type (attribute-name &optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(attribute-type ~S ~S)"
                               (transform-s-expr attribute-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(attribute-type ~S)"
                               (transform-s-expr attribute-name))))))

(defmacro attribute-domain (attribute-name &optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(attribute-domain ~S ~S)"
                                 (transform-s-expr attribute-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(attribute-domain ~S)"
                                 (transform-s-expr attribute-name))))))

(defun attribute-domain-1 (attribute-name &optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(attribute-domain-1 ~S ~S)"
                               (transform-s-expr attribute-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(attribute-domain-1 ~S)"
                               (transform-s-expr attribute-name))))))

(defmacro individual-direct-types (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-direct-types ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-direct-types ~S)" 
                                 (transform-s-expr individual-name))))))

(defun most-specific-instantiators (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(most-specific-instantiators ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(most-specific-instantiators ~S)"
                               (transform-s-expr individual-name))))))

(defmacro individual-types (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-types ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-types ~S)" 
                                 (transform-s-expr individual-name))))))

(defun instantiators (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(instantiators ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(instantiators ~S)"
                               (transform-s-expr individual-name))))))

(defmacro concept-instances (concept &optional (abox-name nil abox-name-supplied-p)
                                     (candidates nil candidates-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (if candidates-supplied-p
        `(service-request ,(format nil "(concept-instances ~S ~S ~S)"
                                   (transform-s-expr concept)
                                   (transform-s-expr abox-name)
                                   (transform-s-expr candidates)))
        `(service-request ,(format nil "(concept-instances ~S ~S)"
                                   (transform-s-expr concept)
                                   (transform-s-expr abox-name))))
      `(service-request ,(format nil "(concept-instances ~S)" 
                                 (transform-s-expr concept))))))

(defun retrieve-concept-instances (concept &optional (abox-name nil abox-name-supplied-p)
                                                (candidates nil candidates-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (if candidates-supplied-p
        (service-request (format nil "(retrieve-concept-instances ~S ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr abox-name)
                                 (transform-s-expr candidates)))
        (service-request (format nil "(retrieve-concept-instances ~S ~S)"
                                 (transform-s-expr concept)
                                 (transform-s-expr abox-name))))
      (service-request (format nil "(retrieve-concept-instances ~S)"
                               (transform-s-expr concept))))))

(defmacro individual-fillers (individual-name role-term &optional (abox-name nil abox-name-supplied-p) &key (told nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-fillers ~S ~S ~S :told ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr role-term)
                                 (transform-s-expr abox-name)
				 (transform-s-expr told)))
      `(service-request ,(format nil "(individual-fillers ~S ~S)" 
                                 (transform-s-expr individual-name)
                                 (transform-s-expr role-term))))))

(defun retrieve-individual-fillers (individual-name role-term &optional (abox-name nil abox-name-supplied-p) &key (told nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-individual-fillers ~S ~S ~S :told ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr role-term)
                               (transform-s-expr abox-name)
			       (transform-s-expr told)))
      (service-request (format nil "(retrieve-individual-fillers ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr role-term))))))

(defmacro related-individuals (role-term &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(related-individuals ~S ~S)"
                                 (transform-s-expr role-term)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(related-individuals ~S)" 
                                 (transform-s-expr role-term))))))

(defun retrieve-related-individuals (role-term &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-related-individuals ~S ~S)"
                               (transform-s-expr role-term)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(retrieve-related-individuals ~S)"
                               (transform-s-expr role-term))))))

(defmacro individual-filled-roles (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-filled-roles ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-filled-roles ~S)" 
                                 (transform-s-expr individual-name))))))

(defun retrieve-individual-filled-roles (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-individual-filled-roles ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(retrieve-individual-filled-roles ~S)"
                               (transform-s-expr individual-name))))))

(defmacro direct-predecessors (individual-name role-term &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(direct-predecessors ~S ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr role-term)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(direct-predecessors ~S ~S)" 
                                 (transform-s-expr individual-name)
                                 (transform-s-expr role-term))))))

(defun retrieve-direct-predecessors (individual-name role-term &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-direct-predecessors ~S ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr role-term)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(retrieve-direct-predecessors ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr role-term))))))

(defun associated-aboxes (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(associated-aboxes ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(associated-aboxes)"))))

(defun associated-tbox (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(associated-tbox ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(associated-tbox)"))))

(defun all-aboxes ()
  (with-standard-io-syntax-1
    (service-request "(all-aboxes)")))

(defun find-abox (abox-name &optional (errorp t))
  (with-standard-io-syntax-1
    (service-request (format nil "(find-abox ~S ~S)"
                             (transform-s-expr abox-name)
                             (transform-s-expr errorp)))))

(defun set-find-abox (abox-name new-value)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-find-abox ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr new-value)))))

(defun (setf find-abox) (new-value abox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-find-abox ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr new-value)))))

(defun all-individuals (&optional (abox-name nil abox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-individuals ~S :count ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request "(all-individuals)"))))

(defun all-concept-assertions-for-individual (individual-name 
                                              &optional (abox-name nil abox-name-supplied-p)
                                              &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-concept-assertions-for-individual ~S ~S :count ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-concept-assertions-for-individual ~S)"
                               (transform-s-expr individual-name))))))

(defun all-role-assertions-for-individual-in-domain (individual-name 
                                                     &optional (abox-name nil abox-name-supplied-p)
                                                     &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-role-assertions-for-individual-in-domain ~S ~S :count ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-role-assertions-for-individual-in-domain ~S)"
                               (transform-s-expr individual-name))))))

(defun all-role-assertions-for-individual-in-range (individual-name
                                                    &optional (abox-name nil abox-name-supplied-p)
                                                    &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-role-assertions-for-individual-in-range ~S ~S :count ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request (format nil "(all-role-assertions-for-individual-in-range ~S)"
                               (transform-s-expr individual-name))))))

(defun all-concept-assertions (&optional (abox-name nil abox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-concept-assertions ~S :count ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request "(all-concept-assertions)"))))

(defun all-role-assertions (&optional (abox-name nil abox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-role-assertions ~S :count ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request "(all-role-assertions)"))))

(defun all-attribute-assertions (&optional (abox-name nil abox-name-supplied-p)
                                           &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-attribute-assertions ~S :count ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request "(all-attribute-assertions)"))))

(defun all-constraints (&optional (abox-name nil abox-name-supplied-p) &key (count nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-constraints ~S :count ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr count)))
      (service-request "(all-constraints)"))))

(defmacro clone-tbox (tbox-name &key
                                (new-name nil new-name-specified-p)
                                (overwrite nil))
  (with-standard-io-syntax-1
    (if new-name-specified-p
      `(service-request ,(format nil "(clone-tbox ~S :new-name ~S :overwrite ~S)"
                                 (transform-s-expr tbox-name)
                                 (transform-s-expr new-name)
                                 (transform-s-expr overwrite)))
      `(service-request ,(format nil "(clone-tbox ~S :overwrite ~S)"
                                 (transform-s-expr tbox-name)
                                 (transform-s-expr overwrite))))))

(defun create-tbox-clone (tbox-name &key
                                (new-name nil new-name-specified-p)
                                (overwrite nil))  
  (with-standard-io-syntax-1
    (if new-name-specified-p
      (service-request (format nil "(create-tbox-clone ~S :new-name ~S :overwrite ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr new-name)
                               (transform-s-expr overwrite)))
      (service-request (format nil "(create-tbox-clone-tbox ~S :overwrite ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr overwrite))))))

(defmacro clone-abox (abox-name &key
                                (new-name nil new-name-specified-p)
                                (overwrite nil))
  (with-standard-io-syntax-1
    (if new-name-specified-p
      `(service-request ,(format nil "(clone-abox ~S :new-name ~S :overwrite ~S)"
                                 (transform-s-expr abox-name)
                                 (transform-s-expr new-name)
                                 (transform-s-expr overwrite)))
      `(service-request ,(format nil "(clone-abox ~S :overwrite ~S)"
                                 (transform-s-expr abox-name)
                                 (transform-s-expr overwrite))))))

(defun create-abox-clone (abox-name &key
                                (new-name nil new-name-specified-p)
                                (overwrite nil))  
  (with-standard-io-syntax-1
    (if new-name-specified-p
      (service-request (format nil "(create-abox-clone ~S :new-name ~S :overwrite ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr new-name)
                               (transform-s-expr overwrite)))
      (service-request (format nil "(create-abox-clone ~S :overwrite ~S)"
                               (transform-s-expr abox-name)
                               (transform-s-expr overwrite))))))

(defun include-kb (pathname)  
  (with-standard-io-syntax-1
    (service-request (format nil "(include-kb ~S)" pathname))))

(defun racer-read-file (pathname)  
  (with-standard-io-syntax-1
    (service-request (format nil "(racer-read-file ~S)" pathname) :result-type 'symbol)))

(defun xml-read-tbox-file (pathname)  
  (with-standard-io-syntax-1
    (service-request (format nil "(xml-read-tbox-file ~S)" pathname) :result-type 'symbol)))

(defun rdfs-read-tbox-file (pathname)  
  (with-standard-io-syntax-1
    (service-request (format nil "(rdfs-read-tbox-file ~S)" 
                             pathname)
                     :result-type 'symbol)))

(defun daml-read-file (pathname &rest args)  
  (with-standard-io-syntax-1
    (service-request (format nil "(daml-read-file ~S~{ ~S~})" 
                             pathname
                             (transform-s-expr args)) :result-type 'symbol)))

(defun daml-read-document (url &rest args)  
  (with-standard-io-syntax-1
    (service-request (format nil "(daml-read-file ~S~{ ~S~})" 
                             url
                             (transform-s-expr args)) :result-type 'symbol)))

(defun owl-read-file (pathname &rest args)  
  (with-standard-io-syntax-1
    (service-request (format nil "(owl-read-file ~S~{ ~S~})" 
                             pathname
                             (transform-s-expr args)) :result-type 'symbol)))

(defun owl-read-document (url &rest args)  
  (with-standard-io-syntax-1
    (service-request (format nil "(owl-read-document ~S~{ ~S~})" 
                             url
                             (transform-s-expr args)) :result-type 'symbol)))

(defun dig-read-file (pathname &rest args)  
  (with-standard-io-syntax-1
    (service-request (format nil "(dig-read-file ~S~{ ~S~})" 
                             pathname
                             (transform-s-expr args)) :result-type 'symbol)))

(defun dig-read-document (url &rest args)  
  (with-standard-io-syntax-1
    (service-request (format nil "(dig-read-file ~S~{ ~S~})" 
                             url
                             (transform-s-expr args)) :result-type 'symbol)))

(defun taxonomy (&optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(taxonomy ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(taxonomy)"))))

(defun print-tbox-tree (&optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(print-tbox-tree ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(print-tbox-tree)"))))

(defun save-tbox (pathname
                    &optional (tbox-name nil tbox-name-supplied-p)
                    &key (syntax :krss-like)
                    (transformed nil)
                    (avoid-duplicate-definitions nil)
                    (if-exists :supersede)
                    (if-does-not-exist :create)
                    (uri nil)
                    (ns0 nil ns0-specified)
                    (anonymized nil))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (if ns0-specified
        (service-request (format nil "(save-tbox ~S ~S ~
                                      :syntax ~S ~
                                      :transformed ~S ~
                                      :avoid-duplicate-definitions ~S ~
                                      :if-exists ~S ~
                                      :if-does-not-exist ~S ~
                                      :uri ~S ~
                                      :ns0 ~S ~
                                      :anonymized ~S)"
                                 (transform-s-expr pathname)
                                 (transform-s-expr tbox-name)
                                 (transform-s-expr syntax)
                                 (transform-s-expr transformed)
                                 (transform-s-expr avoid-duplicate-definitions) 
                                 (transform-s-expr if-exists)
                                 (transform-s-expr if-does-not-exist)
                                 (transform-s-expr uri)
                                 (transform-s-expr ns0)
                                 (transform-s-expr anonymized)))
        (service-request (format nil "(save-tbox ~S ~S ~
                                      :syntax ~S ~
                                      :transformed ~S ~
                                      :avoid-duplicate-definitions ~S ~
                                      :if-exists ~S ~
                                      :if-does-not-exist ~S ~
                                      :uri ~S ~
                                      :anonymized ~S)"
                                 (transform-s-expr pathname)
                                 (transform-s-expr tbox-name)
                                 (transform-s-expr syntax)
                                 (transform-s-expr transformed)
                                 (transform-s-expr avoid-duplicate-definitions) 
                                 (transform-s-expr if-exists)
                                 (transform-s-expr if-does-not-exist)
                                 (transform-s-expr uri)
                                 (transform-s-expr anonymized))))
      (service-request (format nil "(save-tbox ~S)" (transform-s-expr pathname))))))

(defun save-abox (pathname
                    &optional (abox-name nil abox-name-supplied-p)
                    &key (syntax :krss) (transformed nil)
                    (if-exists :supersede) (if-does-not-exist :create))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(save-abox ~S ~S ~
                                    :syntax ~S ~
                                    :transformed ~S ~
                                    :if-exists ~S ~
                                    :if-does-not-exist ~S)"
                               pathname abox-name
                               syntax transformed if-exists if-does-not-exist))
      (service-request (format nil "(save-abox ~S)" (transform-s-expr pathname))))))

(defun save-kb (pathname
                &key 
                (tbox nil tbox-name-supplied-p)
                (abox nil abox-name-supplied-p)
                (if-exists :supersede)
                (if-does-not-exist :create)
                (uri nil)
                (ns0 nil ns0-specified)
                (syntax (if (or uri ns0-specified) :daml :krss)))
  (with-standard-io-syntax-1
    (cond ((and tbox-name-supplied-p abox-name-supplied-p)
           (service-request (format nil "(save-kb ~S ~
                                         :tbox ~S ~
                                         :abox ~S ~
                                         :syntax ~S ~
                                         :if-exists ~S ~
                                         :if-does-not-exist ~S ~
                                         :uri ~S ~
                                         :ns0 ~S)"
                                    (transform-s-expr pathname)
                                    (transform-s-expr tbox)
                                    (transform-s-expr abox)
                                    (transform-s-expr syntax)
                                    (transform-s-expr if-exists)
                                    (transform-s-expr if-does-not-exist)
                                    (transform-s-expr uri)
                                    (transform-s-expr ns0))))
          (tbox-name-supplied-p
           (service-request (format nil "(save-kb ~S ~
                                         :tbox ~S ~
                                         :syntax ~S ~
                                         :if-exists ~S ~
                                         :if-does-not-exist ~S ~
                                         :uri ~S ~
                                         :ns0 ~S)"
                                    (transform-s-expr pathname)
                                    (transform-s-expr tbox)
                                    (transform-s-expr syntax)
                                    (transform-s-expr if-exists)
                                    (transform-s-expr if-does-not-exist)
                                    (transform-s-expr uri)
                                    (transform-s-expr ns0))))
          (abox-name-supplied-p
           (service-request (format nil "(save-kb ~S ~
                                         :abox ~S ~
                                         :syntax ~S ~
                                         :if-exists ~S ~
                                         :if-does-not-exist ~S ~
                                         :uri ~S ~
                                         :ns0 ~S)"
                                    (transform-s-expr pathname)
                                    (transform-s-expr abox)
                                    (transform-s-expr syntax)
                                    (transform-s-expr if-exists)
                                    (transform-s-expr if-does-not-exist)
                                    (transform-s-expr uri)
                                    (transform-s-expr ns0))))
          (t
           (service-request (format nil "(save-kb ~S ~
                                         :syntax ~S ~
                                         :if-exists ~S ~
                                         :if-does-not-exist ~S ~
                                         :uri ~S ~
                                         :ns0 ~S)"
                                    (transform-s-expr pathname)
                                    (transform-s-expr syntax)
                                    (transform-s-expr if-exists)
                                    (transform-s-expr if-does-not-exist)
                                    (transform-s-expr uri)
                                    (transform-s-expr ns0)))))))

(defun compute-index-for-instance-retrieval (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(compute-index-for-instance-retrieval ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(compute-index-for-instance-retrieval)"))))

(defun ensure-subsumption-based-query-answering (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(ensure-subsumption-based-query-answering ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(ensure-subsumption-based-query-answering)"))))

(defun ensure-small-tboxes ()
  (with-standard-io-syntax-1
    (service-request "(ensure-small-tboxes)")))


(defun describe-tbox (&optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(describe-tbox ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(describe-tbox)"))))

(defun describe-abox (&optional (abox-name nil abox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(describe-abox ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(describe-abox)"))))

(defun describe-individual (individual-name &optional (abox-name nil abox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(describe-individual ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(describe-individual ~S)"
                               (transform-s-expr individual-name))))))

(defun describe-concept (concept &optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(describe-concept ~S ~S)"
                               (transform-s-expr concept)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(describe-concept ~S)"
                               (transform-s-expr concept))))))

(defun describe-role (role &optional (tbox-name nil tbox-name-supplied-p))  
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(describe-role ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(describe-role ~S)"
                               (transform-s-expr role))))))




(defmacro individual-attribute-fillers (individual-name attribute &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-attribute-fillers ~S ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr attribute)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-attribute-fillers ~S ~S)" 
                                 (transform-s-expr individual-name)
                                 (transform-s-expr attribute))))))

(defun retrieve-individual-attribute-fillers (individual-name attribute &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-individual-attribute-fillers ~S ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr attribute)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(retrieve-individual-attribute-fillers ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr attribute))))))


(defun told-value (object-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(told-value ~S ~S)"
                               (transform-s-expr object-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(told-value ~S)"
                               (transform-s-expr object-name))))))



(defmacro publish (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(publish ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(publish ~S)" 
                                 (transform-s-expr individual-name))))))

(defun publish-1 (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(publish-1 ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(publish-1 ~S)"
                               (transform-s-expr individual-name))))))

(defmacro unpublish (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(unpublish ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(unpublish ~S)" 
                                 (transform-s-expr individual-name))))))

(defun unpublish-1 (individual-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(unpublish-1 ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(unpublish-1 ~S)"
                               (transform-s-expr individual-name))))))

(defmacro subscribe (query-name query-concept &optional (abox-name nil abox-name-supplied-p)
                                &key (notification-method nil notification-method-supplied))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (if notification-method-supplied
        `(service-request ,(format nil "(subscribe ~S ~S ~S (:notification-method ~{ ~S~}))"
                                   (transform-s-expr query-name)
                                   (transform-s-expr query-concept)
                                   (transform-s-expr abox-name)
                                   (transform-s-expr notification-method)))
        `(service-request ,(format nil "(subscribe ~S ~S ~S)"
                                   (transform-s-expr query-name)
                                   (transform-s-expr query-concept)
                                   (transform-s-expr abox-name))))
      `(service-request ,(format nil "(subscribe ~S ~S)" 
                                 (transform-s-expr query-name)
                                 (transform-s-expr query-concept))))))

(defun subscribe-1 (query-name query-concept &optional (abox-name nil abox-name-supplied-p)
                               &key (notification-method nil notification-method-supplied))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (if notification-method-supplied
        (service-request (format nil "(subscribe ~S ~S ~S (:notification-method ~{ ~S~}))"
                                 (transform-s-expr query-name)
                                 (transform-s-expr query-concept)
                                 (transform-s-expr abox-name)
                                 (transform-s-expr notification-method)))
        (service-request (format nil "(subscribe ~S ~S ~S)"
                                 (transform-s-expr query-name)
                                 (transform-s-expr query-concept)
                                 (transform-s-expr abox-name))))
      (service-request (format nil "(subscribe ~S ~S)" 
                               (transform-s-expr query-name)
                               (transform-s-expr query-concept))))))

(defmacro unsubscribe (query-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(unsubscribe ~S ~S)"
                                 (transform-s-expr query-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(unsubscribe ~S)" 
                                 (transform-s-expr query-name))))))

(defun unsubscribe-1 (query-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(unsubscribe-1 ~S ~S)"
                               (transform-s-expr query-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(unsubscribe-1 ~S)" 
                               (transform-s-expr query-name))))))

(defmacro init-subscriptions (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(init-subscriptions~S)"
                                 (transform-s-expr abox-name)))
      `(service-request "(init-subscriptions)"))))


(defmacro transitive (role-name 
                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(transitive ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(transitive ~S)"
                                 (transform-s-expr role-name))))))

(defun role-is-transitive (role-name tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-is-transitive ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox-name)))))

(defmacro functional (role-name 
                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(functional ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(functional ~S)"
                                 (transform-s-expr role-name))))))

(defun role-is-functional (role-name tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-is-functional ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox-name)))))

(defmacro inverse (role-name inverse-role
                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(inverse ~S ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr inverse-role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(inverse ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr inverse-role))))))

(defun inverse-of-role (role-name inverse-role tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(inverse-of-role ~S ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr inverse-role)
                             (transform-s-expr tbox-name)))))

(defmacro domain (role-name concept
                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(domain ~S ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(domain ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr concept))))))

(defun role-has-domain (role-name concept tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-has-domain ~S ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr concept)
                             (transform-s-expr tbox-name)))))

(defmacro range (role-name concept
                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(range ~S ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr concept)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(range ~S ~S)"
                                 (transform-s-expr role-name)
                                 (transform-s-expr concept))))))

(defun role-has-range (role-name concept tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-has-range ~S ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr concept)
                             (transform-s-expr tbox-name)))))

(defmacro implies-role (role-name-1 role-name-2
                      &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(implies-role ~S ~S ~S)"
                                 (transform-s-expr role-name-1)
                                 (transform-s-expr role-name-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(implies-role ~S ~S)"
                                 (transform-s-expr role-name-1)
                                 (transform-s-expr role-name-2))))))

(defun role-has-parent (role-name-1 role-name-2 tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-has-parent ~S ~S ~S)"
                             (transform-s-expr role-name-1)
                             (transform-s-expr role-name-2)
                             (transform-s-expr tbox-name)))))

(defun declare-disjoint (concept-names tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(declare-disjoint ~S ~S)"
                             (transform-s-expr concept-names)
                             (transform-s-expr tbox-name)))))

(defun get-tbox-language (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p 
      (service-request (format nil "(get-tbox-language)"))
      (service-request (format nil "(get-tbox-language ~S)"
                               (transform-s-expr tbox-name))))))

(defun get-abox-language (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(get-abox-language)"))
      (service-request (format nil "(get-abox-language ~S)"
                             (transform-s-expr abox-name))))))

(defmacro get-concept-definition (concept-name
                                  &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(get-concept-definition ~S ~S)"
                                 (transform-s-expr concept-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(get-concept-definition ~S)"
                                 (transform-s-expr concept-name))))))

(defun get-concept-definition-1 (concept-name tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(get-concept-definition-1 ~S ~S)"
                             (transform-s-expr concept-name)
                             (transform-s-expr tbox-name)))))

(defmacro get-concept-negated-definition (concept-name
                                  &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(get-concept-negated-definition ~S ~S)"
                                 (transform-s-expr concept-name)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(get-concept-negated-definition ~S)"
                                 (transform-s-expr concept-name))))))

(defun get-concept-negated-definition-1 (concept-name tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(get-concept-negated-definition-1 ~S ~S)"
                             (transform-s-expr concept-name)
                             (transform-s-expr tbox-name)))))

(defun get-meta-constraint (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(get-meta-constraint ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(get-meta-constraint)"))))

(defmacro role-range (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-range ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-range ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-range (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-range ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-range ~S)"
                               (transform-s-expr role))))))

(defmacro role-domain (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-domain ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-domain ~S)"
                                 (transform-s-expr role))))))

(defun atomic-role-domain (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(atomic-role-domain ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(atomic-role-domain ~S)"
                               (transform-s-expr role))))))

(defun logging-on (&optional filename)
  (with-standard-io-syntax-1
    (if (null filename)
      (service-request (format nil "(logging-on)"))
      (service-request (format nil "(logging-on ~S)" 
			       (transform-s-expr filename))))))

(defun logging-off ()
  (with-standard-io-syntax-1
    (service-request "(logging-off)")))

(defun get-namespace-prefix (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(get-namespace-prefix ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(get-namespace-prefix)"))))

(defun store-tbox-image (filename &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(store-tbox-image ~S ~S)"
                               filename (transform-s-expr tbox-name)))
      (service-request (format nil "(store-tbox-image ~S)"
                               filename)))))

(defun store-tboxes-image (tboxes filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(store-tboxes-image ~S ~S)"
                             (transform-s-expr tboxes) filename))))

(defun store-abox-image (filename &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(store-abox-image ~S ~S)"
                               filename (transform-s-expr abox-name)))
      (service-request (format nil "(store-abox-image ~S)"
                               filename)))))

(defun store-aboxes-image (aboxes filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(store-aboxes-image ~S ~S)"
                             (transform-s-expr aboxes) filename))))

(defun store-kb-image (filename &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(store-kb-image ~S ~S)"
                               filename (transform-s-expr abox-name)))
      (service-request (format nil "(store-kb-image ~S)"
                               filename)))))

(defun store-kbs-image (kbs filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(store-kb-image ~S ~S)"
                             (transform-s-expr kbs) filename))))

(defun restore-tbox-image (filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(restore-tbox-image ~S)"
                             filename))))

(defun restore-tboxes-image (filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(restore-tboxes-image ~S)"
                             filename))))

(defun restore-abox-image (filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(restore-abox-image ~S)"
                             filename))))

(defun restore-aboxes-image (filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(restore-aboxes-image ~S)"
                             filename))))

(defun restore-kb-image (filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(restore-kb-image ~S)"
                             filename))))

(defun restore-kbs-image (filename)
  (with-standard-io-syntax-1
    (service-request (format nil "(restore-kbs-image ~S)"
                             filename))))

(defun mirror (url1 url2)
  (with-standard-io-syntax-1
    (service-request (format nil "(mirror ~S ~S)"
                             url1 url2))))

(defun kb-ontologies (kb-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(kb-ontologies ~S)"
                             (transform-s-expr kb-name)))))

(defun compute-implicit-role-fillers (individual-name
                                      &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(compute-implicit-role-fillers ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(compute-implicit-role-fillers ~S)"
                               (transform-s-expr individual-name))))))

(defun compute-all-implicit-role-fillers (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(compute-implicit-role-fillers ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(compute-implicit-role-fillers)"))))

(defun get-kb-signature (kb-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(get-kb-signature ~S)"
                             (transform-s-expr kb-name)))))

(defun get-tbox-signature (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(get-tbox-signature ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(get-tbox-signature)"))))

(defun get-abox-signature (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(get-abox-signature ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(get-abox-signature)"))))

(defmacro symmetric? (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(symmetric? ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(symmetric? ~S)"
                                 (transform-s-expr role))))))

(defun symmetric-p (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(symmetric-p ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(symmetric-p ~S)"
                               (transform-s-expr role))))))

(defmacro reflexive? (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(reflexive? ~S ~S)"
                                 (transform-s-expr role)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(reflexive? ~S)"
                                 (transform-s-expr role))))))

(defun reflexive-p (role &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(reflexive-p ~S ~S)"
                               (transform-s-expr role)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(reflexive-p ~S)"
                               (transform-s-expr role))))))

(defmacro tbox-cyclic? (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(tbox-cyclic? ~S)"
                                 (transform-s-expr tbox-name)))
      `(service-request "(tbox-cyclic?)"))))

(defun tbox-cyclic-p (&optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(tbox-cyclic-p ~S)"
                               (transform-s-expr tbox-name)))
      (service-request "(tbox-cyclic-p)"))))

(defun cyclic-concepts-in-tbox (tbox-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(cyclic-concepts-in-tbox ~S)"
                             (transform-s-expr tbox-name)))))

(defmacro constraint-entailed? (constraint &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(constraint-entailed? ~S ~S)"
                                 (transform-s-expr constraint)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(constraint-entailed? ~S)"
                                 (transform-s-expr constraint))))))

(defun constraint-entailed-p (constraint &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(constraint-entailed-p ~S ~S)"
                               (transform-s-expr constraint)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(constraint-entailed-p ~S)"
                               (transform-s-expr constraint))))))

(defmacro roles-equivalent (role-1 role-2 &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(roles-equivalent ~S ~S ~S)"
                                 (transform-s-expr role-1)
                                 (transform-s-expr role-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(roles-equivalent ~S ~S)"
                                 (transform-s-expr role-1)
                                 (transform-s-expr role-2))))))

(defun roles-equivalent-1 (role-1 role-2 &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(roles-equivalent-1 ~S ~S ~S)"
                               (transform-s-expr role-1)
                               (transform-s-expr role-2)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(role-equivalent-1 ~S ~S)"
                               (transform-s-expr role-1)
                               (transform-s-expr role-2))))))


(defmacro role-equivalent? (role-1 role-2 &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      `(service-request ,(format nil "(role-equivalent? ~S ~S ~S)"
                                 (transform-s-expr role-1)
                                 (transform-s-expr role-2)
                                 (transform-s-expr tbox-name)))
      `(service-request ,(format nil "(role-equivalent? ~S ~S)"
                                 (transform-s-expr role-1)
                                 (transform-s-expr role-2))))))

(defun role-equivalent-p (role-1 role-2 &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(role-equivalent-p ~S ~S ~S)"
                               (transform-s-expr role-1)
                               (transform-s-expr role-2)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(role-equivalent-p ~S ~S)"
                               (transform-s-expr role-1)
                               (transform-s-expr role-2))))))


(defun verify-with-concept-tree-list (tree-list &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(verify-with-concept-tree-list ~S ~S)"
                               (transform-s-expr tree-list)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(verify-with-concept-tree-list ~S)"
                               (transform-s-expr tree-list))))))


(defun verify-with-abox-individuals-list (individuals-list
                                          &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(verify-with-abox-individuals-list ~S ~S)"
                               (transform-s-expr individuals-list)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(verify-with-abox-individuals-list ~S)"
                               (transform-s-expr individuals-list))))))


(defmacro define-tbox (&whole form &rest args)
  (declare (ignore args))
  (with-standard-io-syntax-1
    `(service-request ,(format nil "~S" (transform-s-expr form)))))


(defmacro define-abox (&whole form &rest args)
  (declare (ignore args))
  (with-standard-io-syntax-1
    `(service-request ,(format nil "~S" (transform-s-expr form)))))


(defun print-abox-individuals (&key (stream t) (abox (current-abox)))
  (with-standard-io-syntax-1
    (service-request (format nil "(print-abox-individuals :stream ~S :abox ~S)" 
                             stream
                             (transform-s-expr abox)))))


(defun nominals-disjoint-p ()
  (with-standard-io-syntax-1
    (service-request (format nil "(nominals-disjoint-p)"))))


(defun set-nominals-disjoint-p (value)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-nominals-disjoint-p ~S)" 
                             (transform-s-expr value)))))


(defmacro individual-told-datatype-fillers (ind
                                            datatype-role
                                            &optional 
                                            (direct-p nil)
                                            (abox nil abox-specified))
  (if abox-specified
    `(with-standard-io-syntax-1
       (service-request ,(format nil "(retrieve-individual-told-datatype-fillers ~S ~S ~S ~S)"
                                 (transform-s-expr ind)
                                 (transform-s-expr datatype-role)
                                 (transform-s-expr direct-p)
                                 (transform-s-expr abox))))
    `(with-standard-io-syntax-1
       (service-request ,(format nil "(retrieve-individual-told-datatype-fillers ~S ~S ~S)"
                                 (transform-s-expr ind)
                                 (transform-s-expr datatype-role)
                                 (transform-s-expr direct-p))))))

(defun retrieve-individual-told-datatype-fillers (ind datatype-role 
                                                             &optional
                                                             (direct-p nil)
                                                             (abox nil abox-specifed))
  (with-standard-io-syntax-1
    (if abox-specifed
      (service-request (format nil "(retrieve-individual-told-datatype-fillers ~S ~S ~S ~S)"
                               (transform-s-expr ind)
                               (transform-s-expr datatype-role)
                               (transform-s-expr direct-p)
                               (transform-s-expr abox)))
      (service-request (format nil "(retrieve-individual-told-datatype-fillers ~S ~S ~S)"
                               (transform-s-expr ind)
                               (transform-s-expr datatype-role)
                               (transform-s-expr direct-p))))))








(defun set-server-timeout (timeout)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-server-timeout ~S)" 
                             (transform-s-expr timeout)))))


(defun get-server-timeout ()
  (with-standard-io-syntax-1
    (service-request (format nil "(get-server-timeout)"))))




(defun retrieve-individual-synonyms (individual &optional (told-only nil)
                                                     (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-individual-synonyms ~S ~S ~S)"
                               (transform-s-expr individual)
                               (transform-s-expr told-only)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(retrieve-individual-synonyms ~S ~S)"
                               (transform-s-expr individual)
                               (transform-s-expr told-only))))))

(defmacro individual-synonyms (individual &optional (told-only nil)
                                          (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-synonyms ~S ~S ~S)"
                                 (transform-s-expr individual)
                                 (transform-s-expr told-only)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-synonyms ~S ~S)" 
                                 (transform-s-expr individual)
                                 (transform-s-expr told-only))))))







(defun role-used-as-annotation-property-p (role-name tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-used-as-annotation-property-p ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox)))))


(defun role-is-used-as-annotation-property (role-name tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(role-is-used-as-annotation-property ~S ~S)"
                             (transform-s-expr role-name)
                             (transform-s-expr tbox)))))

(defun add-annotation-concept-assertion (abox individual-name concept)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-annotation-concept-assertion ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr individual-name)
                             (transform-s-expr concept)))))

(defun add-annotation-role-assertion (abox predecessor-name filler-name role-term)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-annotation-role-assertion ~S ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr predecessor-name)
                             (transform-s-expr filler-name)
                             (transform-s-expr role-term)))))

(defun all-annotation-concept-assertions (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-annotation-concept-assertions ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(all-annotation-concept-assertions)"))))


(defun all-annotation-role-assertions (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(all-annotation-role-assertions ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(all-annotation-role-assertions)"))))


(defun parse-expression (expr)
  (with-standard-io-syntax-1
    (service-request (format nil "(parse-expression ~S)" 
                             (transform-s-expr expr)))))



(defmacro individual-attribute-value (individual-name attribute-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      `(service-request ,(format nil "(individual-attribute-value ~S ~S ~S)"
                                 (transform-s-expr individual-name)
                                 (transform-s-expr attribute-name)
                                 (transform-s-expr abox-name)))
      `(service-request ,(format nil "(individual-attribute-value ~S ~S)" 
                                 (transform-s-expr individual-name)
                                 (transform-s-expr attribute-name))))))

(defun retrieve-individual-attribute-value (individual-name attribute-name &optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(retrieve-individual-attribute-value ~S ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr attribute-name)
                               (transform-s-expr abox-name)))
      (service-request (format nil "(retrieve-individual-attribute-value ~S ~S)"
                               (transform-s-expr individual-name)
                               (transform-s-expr attribute-name))))))


(defmacro with-critical-section (&body forms)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(with-critical-section ~{ ~S~})" (transform-s-expr forms)))))



(defun set-attribute-filler (abox individual value attribute &optional type)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-attribute-filler ~S ~S ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr individual)
                             (transform-s-expr value)
                             (transform-s-expr attribute)
                             (transform-s-expr type)))))

(defun add-datatype-role-filler (abox individual value role &optional type)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-datatype-role-filler ~S ~S ~S ~S ~S)" 
                             (transform-s-expr abox)
                             (transform-s-expr individual)
                             (transform-s-expr value)
                             (transform-s-expr role)
                             (transform-s-expr type)))))

(defmacro datatype-role-filler (individual value role &optional type)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(datatype-role-filler ~S ~S ~S ~S)" 
                               (transform-s-expr individual)
                               (transform-s-expr value)
                               (transform-s-expr role)
                               (transform-s-expr type)))))

(defmacro attribute-filler (individual value attribute &optional type)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(attribute-filler ~S ~S ~S ~S)" 
                               (transform-s-expr individual)
                               (transform-s-expr value)
                               (transform-s-expr attribute)
                               (transform-s-expr type)))))


(defun set-unique-name-assumption (value)
  (with-standard-io-syntax-1
    (service-request (format nil "(set-unique-name-assumption ~S)" (transform-s-expr value)))))




(defun internal-individuals-related-p (ind-predecessor-name-set
                                            ind-filler-name-set
                                            role-term
                                            abox)
  (with-standard-io-syntax-1
    (service-request (format nil "(internal-individuals-related-p ~S ~S ~S ~S)" 
                             (transform-s-expr ind-predecessor-name-set)
                             (transform-s-expr ind-filler-name-set)
                             (transform-s-expr role-term)
                             (transform-s-expr abox)))))

(defun get-tbox-version (tbox)
  (with-standard-io-syntax-1
    (service-request (format nil "(get-tbox-version ~S)" 
                             (transform-s-expr tbox)))))

(defun get-abox-version (abox)
  (with-standard-io-syntax-1
    (service-request (format nil "(get-abox-version ~S)" 
                             (transform-s-expr abox)))))




(defun create-tbox-internal-marker-concept (tbox-name &optional (concept-name 
                                                                 nil concept-name-supplied-p))
  (with-standard-io-syntax-1
    (if concept-name-supplied-p
      (service-request (format nil "(create-tbox-internal-marker-concept ~S ~S)"
                               (transform-s-expr tbox-name)
                               (transform-s-expr concept-name)))
      (service-request (format nil "(create-tbox-internal-marker-concept ~S)"
                               (transform-s-expr tbox-name))))))





(defun forget-constrained-assertion (abox individual-name object-name attribute-term)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-constrained-assertion ~S ~S ~S ~S)"
                               (transform-s-expr abox)
                               (transform-s-expr individual-name)
                               (transform-s-expr object-name)
                               (transform-s-expr attribute-term)))))

(defun forget-constraint (abox constraint)
  (with-standard-io-syntax-1
    (service-request (format nil "(forget-constraint ~S ~S)"
                             (transform-s-expr abox)
                             (transform-s-expr constraint)))))


(defun swrl-forward-chaining (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(swrl-forward-chaining ~S)"
                               (transform-s-expr abox-name)))
      (service-request "(swrl-forward-chaining)"))))


(defun prepare-racer-engine (&key (abox nil abox-name-supplied-p)
                                  (classify-tbox-p nil))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(prepare-racer-engine :abox ~S :classify-tbox-p ~S)"
                               (transform-s-expr abox) 
                               (transform-s-expr classify-tbox-p)))
      (service-request (format nil "(prepare-racer-engine :classify-tbox-p ~S)"
                               classify-tbox-p)))))


(defun enable-optimized-query-processing (&optional (include-datatype-roles t))
  (with-standard-io-syntax-1
    (service-request (format nil "(enable-optimized-query-processing ~S)"
                             (transform-s-expr include-datatype-roles)))))

(defun prepare-abox (&optional (abox-name nil abox-name-supplied-p))
  (with-standard-io-syntax-1
    (if abox-name-supplied-p
      (service-request (format nil "(prepare-abox ~S)"
                               (transform-s-expr abox-name)))
      (service-request (format nil "(prepare-abox)")))))



(defmacro same-as (individual-name-1 individual-name-2)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(same-as ~S ~S)" 
                               (transform-s-expr individual-name-1)
                               (transform-s-expr individual-name-2)))))

(defmacro same-individual-as (individual-name-1 individual-name-2)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(same-individual-as ~S ~S)" 
                               (transform-s-expr individual-name-1)
                               (transform-s-expr individual-name-2)))))

(defun add-same-individual-as-assertion (abox-name individual-name-1 individual-name-2)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-same-individual-as-assertion ~S ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr individual-name-1)
                             (transform-s-expr individual-name-2)))))


(defmacro different-from (individual-name-1 individual-name-2)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(different-from ~S ~S)" 
                               (transform-s-expr individual-name-1)
                               (transform-s-expr individual-name-2)))))

(defun add-different-from-assertion (abox-name individual-name-1 individual-name-2)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-different-from-assertion ~S ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr individual-name-1)
                             (transform-s-expr individual-name-2)))))


(defmacro all-different (&rest inds)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(all-different ~{ ~S~})"
                             (transform-s-expr inds)))))


(defun add-all-different-assertion (abox-name individual-name-set)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-all-different-assertion ~S ~S ~S)" 
                             (transform-s-expr abox-name)
                             (transform-s-expr individual-name-set)))))

(defun declare-current-knowledge-bases-as-persistent ()
  (with-standard-io-syntax-1
    (service-request (format nil "(declare-current-knowledge-bases-as-persistent)"))))


(defun triple-store-read-file (filename &rest args)
  (with-standard-io-syntax-1
    (service-request (format nil "(triple-store-read-file ~S~{ ~S~})" 
                             filename
                             (transform-s-expr args)) :result-type 'symbol)))

(defun use-triple-store (db &rest args)
  (with-standard-io-syntax-1
    (service-request (format nil "(use-triple-store ~S~{ ~S~})" 
                             db
                             (transform-s-expr args)) :result-type 'symbol)))

(defun materialize-inferences (kb-name &rest args)
  (with-standard-io-syntax-1
    (service-request (format nil "(materialize-inferences ~S~{ ~S~})" 
                             kb-name
                             (transform-s-expr args)))))


(defun triple-store-open-p (&optional db-name)
  (with-standard-io-syntax-1
    (service-request (format nil "(triple-store-open-p ~S)" 
                             (transform-s-expr db-name)))))

(defun close-triple-store (&key db (if-closed :error))
  (with-standard-io-syntax-1
    (service-request (format nil "(close-triple-store :db ~S :if-closed ~S)" 
                             (transform-s-expr db)
			     (transform-s-expr if-closed)))))

(defun open-triple-store (name &key (directory *default-pathname-defaults*)
				    (if-does-not-exist :error))
  (with-standard-io-syntax-1
      (service-request (format nil "(open-triple-store ~S :directory ~S :if-does-not-exist ~S)" 
			       (transform-s-expr name)
			       (transform-s-expr directory)
			       (transform-s-expr if-does-not-exist)))))

(defun create-triple-store (name &key (if-exists :error) 
				      (directory *default-pathname-defaults*))
  (with-standard-io-syntax-1
      (service-request (format nil "(create-triple-store ~S :directory ~S :if-exists ~S)" 
			       (transform-s-expr name)
			       (transform-s-expr directory)
			       (transform-s-expr if-exists)))))

(defun index-all-triples (&key db)
  (with-standard-io-syntax-1
    (service-request (format nil "(index-all-triples :db ~S)" 
                             (transform-s-expr db)))))

(defun server-case ()
  (with-standard-io-syntax-1
    (service-request (format nil "(server-case)")))) 


(defmacro define-event-assertion (assertion)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-event-assertion ~S)"
                               (transform-s-expr assertion)))))

(defun add-event-assertion (assertion &optional (abox nil abox-supplied-p))
  (if abox-supplied-p
    (with-standard-io-syntax-1
      (service-request (format nil "(add-event-assertion ~S ~S)" 
                               (transform-s-expr assertion)
                               (transform-s-expr abox))))
    (with-standard-io-syntax-1
      (service-request (format nil "(add-event-assertion ~S)" 
                               (transform-s-expr assertion))))))

(defmacro define-event-rule (head &rest body)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(add-event-rule ~S ~S)"
                               (transform-s-expr head)
                               (transform-s-expr body)))))

(defun add-event-rule (head body &optional (abox nil abox-supplied-p))
  (if abox-supplied-p
    (with-standard-io-syntax-1
      (service-request (format nil "(add-event-rule ~S ~S ~S)" 
                               (transform-s-expr head)
                               (transform-s-expr body)
                               (transform-s-expr abox))))
    (with-standard-io-syntax-1
      (service-request (format nil "(add-event-rule ~S ~S)" 
                               (transform-s-expr head)
                               (transform-s-expr body))))))
  
(defun timenet-answer-query (query &key (abox nil abox-supplied-p))
  (if abox-supplied-p
      (with-standard-io-syntax-1
	  (service-request (format nil "(timenet-answer-query ~S :abox ~S)" 
				   (transform-s-expr query)
				   (transform-s-expr abox))))
    (with-standard-io-syntax-1
	  (service-request (format nil "(timenet-answer-query ~S)" 
				   (transform-s-expr query))))))


(defmacro timenet-retrieve (query)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(timenet-retrieve ~S)"
                               (transform-s-expr query)))))


(defun enable-abduction (c-mode r-mode)
  (with-standard-io-syntax-1
    (service-request (format nil "(enable-abduction ~S ~S)" 
                             (transform-s-expr c-mode)
                             (transform-s-expr r-mode)))))

(defun disable-abduction ()
  (with-standard-io-syntax-1
    (service-request (format nil "(enable-abduction)"))))


                     
(defun racer-answer-query-with-explanation (&rest args)
  (with-standard-io-syntax-1
    (service-request (format nil "(racer-answer-query-with-explanation ~{ ~S~})" 
                             (transform-s-expr args)))))

(defmacro retrieve-with-explanation (&rest args)
  `(apply #'racer-answer-query-with-explanation ',args))

(defun pracer-answer-query (&rest args)
  (with-standard-io-syntax-1
    (service-request (format nil "(pracer-answer-query ~{ ~S~})" 
                             (transform-s-expr args)))))

(defmacro pretrieve (&rest args)
  `(apply #'pracer-answer-query ',args))

(defun sparql-answer-query (&rest args)
  (with-standard-io-syntax-1
    (service-request (format nil "(sparql-answer-query ~{ ~S~})" 
                             (transform-s-expr args)))))

(defmacro sparql-retrieve (&rest args)
  `(apply #'sparql-answer-query ',args))



(defun add-rule-axiom (abox lefthand-side righthand-side)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-rule-axiom ~S ~S ~S)"
                             (transform-s-expr abox)
                             (transform-s-expr lefthand-side)
                             (transform-s-expr righthand-side)))))

(defmacro define-rule (lefthand-side righthand-side)
  (with-standard-io-syntax-1
    `(service-request ,(format nil "(define-rule ~S ~S)"
                               (transform-s-expr lefthand-side)
                               (transform-s-expr righthand-side)))))

(defun get-role-datatype (role-name &optional (tbox-name nil tbox-name-supplied-p))
  (with-standard-io-syntax-1
    (if tbox-name-supplied-p
      (service-request (format nil "(get-role-datatype ~S ~S)"
                               (transform-s-expr role-name)
                               (transform-s-expr tbox-name)))
      (service-request (format nil "(get-role-datatype ~S)"
                               (transform-s-expr role-name))))))


(defun lcs (concept-1 concept-2)
  (with-standard-io-syntax-1
    (service-request (format nil "(lcs ~S ~S)" 
                             (transform-s-expr concept-1)
                             (transform-s-expr concept-2)))))


(defun lcs-unfold (concept-1 concept-2 &optional (tbox nil tbox-supplied-p))
  (if tbox-supplied-p
      (with-standard-io-syntax-1
	  (service-request (format nil "(lcs-unfold ~S ~S ~S)" 
				   (transform-s-expr concept-1)
				   (transform-s-expr concept-2)
				   (transform-s-expr tbox))))
    (with-standard-io-syntax-1
	  (service-request (format nil "(lcs-unfold ~S ~S)" 
				   (transform-s-expr concept-1)
				   (transform-s-expr concept-2))))))

(defun msc-k (individual k &optional (include-most-specific-instantiators nil)
                          (abox (current-abox) abox-supplied-p))
  (if abox-supplied-p
      (with-standard-io-syntax-1
	  (service-request (format nil "(msc-k ~S ~S ~S ~S)" 
				   (transform-s-expr individual)
				   (transform-s-expr k)
				   (transform-s-expr include-most-specific-instantiators)
				   (transform-s-expr abox))))
    (with-standard-io-syntax-1
	  (service-request (format nil "(msc-k ~S ~S ~S)" 
				   (transform-s-expr individual)
				   (transform-s-expr k)
				   (transform-s-expr include-most-specific-instantiators))))))

(defun add-prefix (prefix mapping)
  (with-standard-io-syntax-1
    (service-request (format nil "(add-prefix ~S ~S)" 
                             (transform-s-expr prefix)
                             (transform-s-expr mapping)))))

(defmacro define-prefix (prefix mapping)
  `(add-prefix ',prefix ',mapping))



(pushnew :lracer *features*)