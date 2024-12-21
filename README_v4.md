## The ThreeDee (.ddd) Language V4

**Teammate: Andrew Marten Medina**

**UNI: amm2478**

**Video demo can be found under Demo.txt**

## What has changed

In prior iterations of this project, I implemented a scanner and parser for my 3D printing language that would output ASTs based on lexical tokens and a context-free grammar I developed. I also implemented a code generation step that produced relevant GCode based on a given AST and demonstrated how it works when you run it on a 3D printing simulator like https://ncviewer.com.

Now, V4 implements two optimizations that we have learned about in class, namely constant folding and dead code elimination.

Constant folding involves taking expressions like "X = 4 + 5" and simplifying the node structure such that rather than assigning to an expression, we assign to an integer and utilize our prior helper function of `do_math` to perform the calculation and cut out the middle step, so that the produced AST shows us the final calculation immediately rather than the intermediate step of the expression. The ultimate function for this is `fold_constants`.

As for dead code elimination, this relies on whether or not the code was actually PRINTed in the end, since this indicates that the variable itself was actually used. For this, helpers like `determine_used_variables` were developed. If the variable was not printed, then its initialization and all subsequent modifications to it are stripped from the AST and we are left with just the parts relevant to what was eventually PRINTed. The ultimate function for this is `eliminate_dead_code`.

## Usage (same as before)

1. Install Flex (`brew install Flex`) and GCC (`brew install gcc`).
2. Run `chmod +x run_scanner.sh` to make the scanner executable.
3. Execute the scanner script with a `.ddd` file as the argument, e.g.:

```
./run_scanner.sh test_1_v4.ddd
```

## Five sample input programs and their expected outputs

### test_1_v4.ddd

#### Input

```
CREATE X HIGH
X = 3 + 6
PRINT X
```

#### Output

```
Original Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: HIGH
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: X
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 3
    OPERATOR: +
    INTEGER: 6
PRINT: PRINT
  IDENTIFIER: X

Folding constants...

* Folded 3 + 6 to 9

Eliminating dead code...

Optimized Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: HIGH
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: X
  ASSIGN: =
  INTEGER: 9
PRINT: PRINT
  IDENTIFIER: X

Generated GCode:
G92 X10 ; Initialize X to HIGH (10)
; Updated X to 9
M117 X9 ; Printed value of X
```

### test_2_v4.ddd

#### Input

```
CREATE X LOW
CREATE Y MEDIUM
X = 4
Y = 7
PRINT X
```

#### Output

```
Original Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: LOW
COMMAND: CREATE
  IDENTIFIER: Y
  PARAMETER: MEDIUM
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: X
  ASSIGN: =
  INTEGER: 4
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Y
  ASSIGN: =
  INTEGER: 7
PRINT: PRINT
  IDENTIFIER: X

Folding constants...

Eliminating dead code...

* Removed unused assignment Y

* Removed unused variable Y

Optimized Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: LOW
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: X
  ASSIGN: =
  INTEGER: 4
PRINT: PRINT
  IDENTIFIER: X

Generated GCode:
G92 X1 ; Initialize X to LOW (1)
; Updated X to 4
M117 X4 ; Printed value of X
```

### test_3_v4.ddd

#### Input

```
CREATE X LOW
CREATE Y HIGH
CREATE Z MEDIUM
X = 11 + 5
Y = 3 * 0
Z = 8 - 7
PRINT Y
PRINT Z
```

#### Output

```
Original Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: LOW
COMMAND: CREATE
  IDENTIFIER: Y
  PARAMETER: HIGH
COMMAND: CREATE
  IDENTIFIER: Z
  PARAMETER: MEDIUM
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: X
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 11
    OPERATOR: +
    INTEGER: 5
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Y
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 3
    OPERATOR: *
    INTEGER: 0
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Z
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 8
    OPERATOR: -
    INTEGER: 7
PRINT: PRINT
  IDENTIFIER: Y
PRINT: PRINT
  IDENTIFIER: Z

Folding constants...

* Folded 11 + 5 to 16

* Folded 3 * 0 to 0

* Folded 8 - 7 to 1

Eliminating dead code...

* Removed unused assignment X

* Removed unused variable X

Optimized Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: LOW
COMMAND: CREATE
  IDENTIFIER: Y
  PARAMETER: HIGH
COMMAND: CREATE
  IDENTIFIER: Z
  PARAMETER: MEDIUM
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Y
  ASSIGN: =
  INTEGER: 0
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Z
  ASSIGN: =
  INTEGER: 1
PRINT: PRINT
  IDENTIFIER: Y
PRINT: PRINT
  IDENTIFIER: Z

Generated GCode:
G92 X1 ; Initialize X to LOW (1)
G92 Y10 ; Initialize Y to HIGH (10)
G92 Z5 ; Initialize Z to MEDIUM (5)
; Updated Y to 0
; Updated Z to 1
M117 Y0 ; Printed value of Y
M117 Z1 ; Printed value of Z
```

### test_4_v4.ddd


#### Input

```
CREATE X MEDIUM
CREATE Y HIGH
X = 6
Y = 8 / 2
PRINT Y
```

#### Output

```
Original Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: MEDIUM
COMMAND: CREATE
  IDENTIFIER: Y
  PARAMETER: HIGH
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: X
  ASSIGN: =
  INTEGER: 6
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Y
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 8
    OPERATOR: /
    INTEGER: 2
PRINT: PRINT
  IDENTIFIER: Y

Folding constants...

* Folded 8 / 2 to 4

Eliminating dead code...

* Removed unused assignment X

* Removed unused variable X

Optimized Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: MEDIUM
COMMAND: CREATE
  IDENTIFIER: Y
  PARAMETER: HIGH
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Y
  ASSIGN: =
  INTEGER: 4
PRINT: PRINT
  IDENTIFIER: Y

Generated GCode:
G92 X5 ; Initialize X to MEDIUM (5)
G92 Y10 ; Initialize Y to HIGH (10)
; Updated Y to 4
M117 Y4 ; Printed value of Y
```

### test_5_v4.ddd

#### Input

```
CREATE X LOW
CREATE Y LOW
CREATE Z LOW
Y = 2 * 2
Z = 9 - 9
PRINT X
PRINT Z
```

#### Output

```
Original Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: LOW
COMMAND: CREATE
  IDENTIFIER: Y
  PARAMETER: LOW
COMMAND: CREATE
  IDENTIFIER: Z
  PARAMETER: LOW
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Y
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 2
    OPERATOR: *
    INTEGER: 2
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Z
  ASSIGN: =
  EXPRESSION: EXPRESSION
    INTEGER: 9
    OPERATOR: -
    INTEGER: 9
PRINT: PRINT
  IDENTIFIER: X
PRINT: PRINT
  IDENTIFIER: Z

Folding constants...

* Folded 2 * 2 to 4

* Folded 9 - 9 to 0

Eliminating dead code...

* Removed unused assignment Y

* Removed unused variable Y

Optimized Abstract Syntax Tree:
COMMAND: CREATE
  IDENTIFIER: X
  PARAMETER: LOW
COMMAND: CREATE
  IDENTIFIER: Z
  PARAMETER: LOW
ASSIGNMENT: ASSIGNMENT
  IDENTIFIER: Z
  ASSIGN: =
  INTEGER: 0
PRINT: PRINT
  IDENTIFIER: X
PRINT: PRINT
  IDENTIFIER: Z

Generated GCode:
G92 X1 ; Initialize X to LOW (1)
G92 Z1 ; Initialize Z to LOW (1)
; Updated Z to 0
M117 X1 ; Printed value of X
M117 Z0 ; Printed value of Z
```

## Simulating the generated GCode

Just like in V3, in order to verify the outputs of the generated G-code, please use https://ncviewer.com and paste the code into the section marked "GCode File", zoom out to get a full view of the provided grid, and then step through from start to finish in its simulation by using the play/stop/next buttons at the bottom of the screen.
