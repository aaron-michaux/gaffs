 
# Grammar Analysis of First Follow Sets

Gaffs is a tool for analyzing context free grammars, and discovering their
first-follow sets.

## Examples

## Lexer


## Grammar

The input file format is in `gaff` format, which is inspired by EBNF, but simpler in my opinion. A `gaff` parser for the `gaff` format is given at the end of this section.

```
// Tokens are "ALL-CAPs", or strings

Grammar: TSTART Rule* EOF ;

Rule: IDENTIFIER ':' ElementList ('|' ElementList)* ';' ;

ElementList: (Element ElementSuffix?)+ ;

Element: IDENTIFIER | STRING | '(' ElementList ')' ;

ElementSuffix: '*' | '+' | '?' ;
```

