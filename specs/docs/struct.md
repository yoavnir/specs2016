# Structure in Specifications
The specifications described in previous pages is linear. It is a series of **Data Field** units and **SET** units which are performed in sequence for each and every record.

But **specs** also allows some structure in specifications, including loops, conditional execution, run-in and run-out cycles. This page describes those structures.

## Conditions
The [Arithmetic-Logical Unit (ALU) page](alu.md) described **expressions** and used them in **PRINT** data fields. 

A specific type of expression is the **condition**, which is an expression that evaluates to either `1` (**true**) or `0` (**false**).  The result of such expression is often called *boolean*. In fact any expression can be used as a condition. In that case any zero value, empty string or NaN is treated as the boolean **false**, and anything else is treated as the boolean **true**.

Examples of condition expressions:
* `a < 2`
* `len(s) > 5`
* `#0 = 0`
* `sqrt(#4)=7`
* `#3 % 2` -- #3 is odd
* `!(#3 % 2)` -- #3 is even
* `sqrt(a)` -- Is the square root of `a` a positive number? That is equivalent to `a>0`, because for values of `a` smaller than zero `sqrt` returns `NaN`
* `#4 -= 1` -- returns the value of `#4` after subtracting `1`, and will be treated as **true** until #4 goes to zero.

## Conditional Execution
This is the if-statement from many programming languages. The general structure is as follows:
```
IF (condition) THEN 
    unit-sequence
ELSEIF (condition) THEN
    unit-sequence
ELSEIF (condition) THEN
    unit-sequence
ELSE
    unit-sequence
ENDIF
```
The `THEN` keywords are necessary. The `ELSEIF (condition) THEN` blocks are optional and you can have as many as you would like. The `ELSE (condition)` block is also optional, while the `ENDIF`is required. **NOTE:** A conditional execution structure without `ENDIF` will sometimes work, but this should not be relied on, and it can make nested conditional statements ambiguous.

In any conditional execution statement, at most one of the unit sequences is executed. In a conditional execution statement that includes an `ELSE` clause, exactly one unit-sequence is executed.

A simple, one-line example:
`specs  a: w1 1  /is/ nextword  IF "a%2" THEN  /odd/ nextword  ELSE  /even/ nextword  ENDIF`
| Input | Output |
| ----- | ------ |
| 0 | 0 is even |
| 1 | 1 is odd |
| 25 | 25 is odd |
| 404 | 404 is even|
| 2.5 | 2.5 is even |
| 3.5 | 3.5 is odd |
| hello | hello is even|
As usual with computers, the GIGO (garbage in -- garbage out) rule applies. The first four inputs are numbers and the output is correct. Attempting to use the remainder operator on 2.5 forces **specs** to treat that value as an integer, which evaluates to 2. Similarly, 3.5 evaluates to 3. All strings that don't look like numbers evaluate to zero when forced to be treated like integers, so `hello` turns out to be even.

## Loops
Loops take many forms in different programming languages: while-loops, for-loops, repeat-until, or even the dreaded conditional goto.  In **specs** there is only one kind of loop: the while-loop:
```
WHILE (condition) DO
    unit-sequence
DONE
```
As long as the *condition* evaluates to **true**, the unit sequence will be repeated over and over.
Consider the following simple loop:

`specs  a: w1 1  SET "#1:=a"  WHILE "#1>0" DO  /*/ n  SET "#1-=1"  DONE`

| Input | Output |
| ----- | ------ |
| -1 | -1 |
| 0 | 0 |
| 1 | 1* |
| 25 | 25************************* |
| 1.5 | 1.5** |
| hello | hello |
**Note:** In the Unix command-line you will have to escape the star with a backslash, otherwise it evaluates to a list of files at the root directory of the file system.

The above specification can be naïvely simplified some:

`specs  a: w1 1  SET "#1:=a"  WHILE "#1-=1" DO  /*/ n  DONE`

But that doesn't work correctly. Suppose the input was 3. So #1 is set to 3. In the first iteration it is reduced to 2 and a star is printed; in the second iteration it is reduced to 1 and a star is printed. But in the third iteration it is reduced to zero and the loop exists having printed only two stars. This is known as an *off-by-one error*. We can fix it like this:

`specs  a: w1 1  SET "#1:=a+1"  WHILE "#1-=1" DO  /*/ n  DONE`

This now returns the correct values for all the integers from zero and up.  But it tends to get into endless loops. Plug in inputs like `-1`, `1.2`, or `hello` and **specs** hangs. A future version will have a **while-guard** feature that will make specs exit with an error in a case like this, but it is up to the specification to avoid endless loops. Do not use a loop condition such as this unless it is certain that `#1` is positive integer when entering the loop for the first time.

## Run-In and Run-Out
To help with summaries and initialization, **specs** provides ways to execute certain units only at the Run-In cycle or only at the Run-Out cycle.

### Run-In
Run-In is just another name for the first iteration of the specification. You check for it with the boolean function `first()`:
```
# Print all the records that match the first record's first word.
specs if "first()" then
          set "#0:=word(1)"
      endif
      if "#0==word(1)" then
          1-* 1
      endif
```

### Run-Out
The Run-Out cycle runs *after* the last record is processed. It is only run if it has something to do. There are two ways to do things on the run-out cycles:
1. Using the boolean function `eof()`
2. Using the `EOF` keyword
The following enhancement of the run-in example will demonstrate both:
```
# Print all the records that match the first record's first word.
specs if "first()" then
          set "#0:=word(1)"
      endif
      if "#0==word(1)" then
          1-* 1
          set "#1+=1"
      endif
      if "eof()" then
          /In the run-out cycle./ 1
      endif
    EOF
      /There were/ nw
      print "#1" nw
      /records that began with/ nw
      print "#0" nw
```
Or a more practical example, from the *CMS Pipelines* book:
```
# Summing
specs
      1-*        1
   a: word 1     .
      set #0+=a
   eof
      /Total:/   1
      print #0   Next
```
