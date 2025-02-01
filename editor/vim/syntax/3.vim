" keywords
syn keyword Keywords ret
hi link Keywords Keyword

syn keyword Delimiters bgn end
hi link Delimiters Keyword

syn keyword Conditions cnd otw
hi link Conditions Conditional

syn keyword Loops whl for rng
hi link Loops Repeat

" types
syn keyword Types nil int flt chr
hi link Types Type

" operators
syn keyword Operators set add mns tms div lor xor and eql ieq seq sup inf
hi link Operators Operator

" string
syn region Strings start=+"+ skip=+ \\"+ end=+"+ contains=EscapedCharacters
hi link Strings String

" escaped char
syn match EscapedCharacters /\\./
hi link EscapedCharacters Special

" characters
syn match Characters /'.'/
syn match Characters /'\\.'/
hi link Characters Character

" numbers
syn match Flt /[ ,(][-]\?[0-9]\+\.[0-9]*/
syn match Int /[ ,(][-]\?[0-9]\+/
hi link Flt Number
hi link Int Number

" functions
syn match Functions /[a-zA-Z][0-9a-zA-Z_]*\ze(/
hi link Functions Function

" use
syn match Use /use/ contained
hi link Use Keyword

syn match File /use\s\+.*/ contains=Use
hi link File Identifier

" comments
syn match Comments /\~\~\~.*/
hi link Comments Comment

