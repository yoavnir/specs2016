# Basic Specifications

**specs** is a command-line utility for processing text. The basic format is as follows:
```
  specs [switches] [spec-units]
```
Most invocations of **specs** do not require command-line switches, so we will not describe them here. For a full description, you can go to [Command-Line Switches](cliswitch.md)

* [Spec Units](#spec-units)
* [Data Fields](#data-fields)
* [Alignment](#alignment)
* [Conversions](#conversions)
* [Words vs Fields](#words-vs-fields)
* [Examples](#examples)
* [Other Common Spec Units](#other-common-spec-units)

Spec Units
==========
Spec Units are the building blocks of a specs specification.  Each spec unit specifies an action to be taken by the program. The spec unit may span from one to several command line arguments. The collection of spec units in a single invocation of **specs** is called a *specification*.

The most common spec unit is a *data field*. They will be covered in the following section. Other kinds of spec units are covered in other pages

Data Fields
=========== 
Data fields are the most common type of *spec unit*. They consist of six arguments, four of which may be omitted:
```
    [fieldIdentifier] InputSource [STRIP] [conversion] OutputPlacement [alignment]
```
A *fieldIdentifier* is a single letter followed by a colon (like _a:_), that maps to the input or output of a single data field unit for later reference such as in later data fields or in expressions.  If the fieldIdentifier is at the start of the data field unit, it contains the input. If it is the OutputPlacement, it contains the output.  For example:
```
     a: w1 ucase b:
```
sets _a_ to the content of the first word of the input record, and sets _b_ to an upper-case version of the same.

The **InputSource** argument may be any of the following:

* A range of characters, such as `5`, `3-7`, or `5.8`, the last one indicating 8 characters starting in the 5th position. Note that the indexing of characters is 1- rather than 0-based. Negative values can be used for counting characters from the end, so -1 means the last character, -2 the penultimate character, etc.
* A range of words, such as `w5` or `words 5-7`, where words are separated by one or more `wordseparator` characters -- locale-defined whitespace by default. The word indexing is 1-based. Negative indexes can be used here as well.
* A range of fields, such as `fields 5` or `f5-7`, where fields are separated by exactly one `fieldseparator` characters -- a tab by default. The field indexing is 1-based. Negative indexes can be used here as well.
* **TODclock** - a floating point number, accurate to microseconds and giving seconds since the Unix epoch.
* **DTODclock** - a floating point number, accurate to microseconds and giving seconds since the Unix epoch. The difference is that TODclock shows the time when this run of *specs* begun, while DTODclock gives the time of producing the current record.
* **NUMBER** - A record counter as a 10-digit decimal number.  Read more about expressions on the [Arithmetic-Logical Unit](alu.md) page.
* **TIMEDIFF** - an 8-char decimal number indicating the number of seconds since the invocation of the program.
* An **ID** keyword followed by a previously defined **FieldIdentifier**.
* The **PRINT** keyword followed by a calculated expression
* A string literal, optionally enclosed by delimiters, such as `/TODclock/` or `'NUMBER'`. Note that to include the single quotes on the Unix command line requires you to enclose them in double quotes.
* A **SUBSTring** of another InputSource.

*SUBSTring* is formatted as follows:
```
     SUBSTRING [WORDSEP char] [FIELDSEP char] range|wordrange|fieldrange OF InputSource
```
*WORDSEP* and *FIELDSEP* are used to specify word separator and field separator characters respectively and the are covered in another page. This allows you to pick words or fields based on some other character. For example, suppose we are parsing the output of the `ls -l` command:
```
-rw-r--r--  1 root  wheel  2554 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/DiffieHellmanErrorPage.html
-rw-r--r--  1 root  wheel  2364 Oct 30 02:59 /Applications/Safari.app/Contents/Resources/en.lproj/InfoPlist.strings
-rw-r--r--  1 root  wheel  2048 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/JavaScriptErrorPage.html
-rw-r--r--  1 root  wheel  3000 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/OfflineReadingListErrorPage.html
-rw-r--r--  1 root  wheel  2038 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/ServerNotFoundErrorPage.html
-rw-r--r--  1 root  wheel   704 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/ServicesMenu.strings
-rw-r--r--  1 root  wheel  2038 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/StandardErrorPage.html
-rw-r--r--  1 root  wheel  2396 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/WebProcessCrashErrorPage.html
-rw-r--r--  1 root  wheel  6958 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/localizedStrings.js
```
We only want the file name, but there are two issues here. First, the file name is part of the long full path at the end. As words are defined as strings separated by spaces, the full path is the 9th (or last) word. But we don't want all of that word. We only want the last _field_ if we define a field to be separated by slash characters. So to get the last field we would use this input part
```
     SUBSTRING FIELDSEP / FIELD -1 OF WORD -1
```
    
The **OutputPlacement** argument specifies where to put the source:

* absolute position (such as `1`)
* range (such as `1-5` or `1.5`)
* `n` or `next` for placing the output immediately after the 
    previous output.
* `nw` or `nextword` for placing the output following a space 
    character after the previous output.
* `nf` or `nextfield` for placing the output following a tab 
    character after the previous output.
* `.` (a period) to indicate no output (useful for data fields with only field identifiers).
* A field identifier as explained above.

Alignment
=========    
The _alignment_ argument can be "left", "center", or "right", or even "centre" if you're so inclined.

Conversions
===========
The _conversion_ argument can specify any of the following conversions:

* **rot13** - encrypts the bytes using the ROT-13 cipher.
* **C2B** - converts characters to binary: "AB" --> "0010000100100010".
* **C2X** - converts characters to hexadecimal: "AB" --> "4142".
* **B2C** - converts binary to characters: "0010000100100010" --> "AB". Will throw an exception if called with an invalid character.
* **X2CH** - converts hexadecimal to characters: "4142" --> "AB". Will throw an exception if called with an invalid character.
* **b2x** - converts binary data to hex.
* **D2X** - convert decimal to hex: "314159265" --> "12b9b0a1".
* **X2D** - convert hex to decimal: "12b9b0a1" --> "314159265".
* **ucase** - converts text to uppercase.
* **lcase** - converts text to lowercase.
* **BSWAP** - byte swap. reverses the order of bytes: "AB" --> "BA"
* **ti2f format** - convert internal time format (8-byte microseconds since the epoch) to printable format using the conventions of strftime, plus %xf for fractional seconds, where x represents number of digits from 0 to 6.
* **tf2i format** - convert printable time format to the internal 8-byte representation. 
* **d2tf format** - convert a decimal number with up to six decimal places, representing seconds since the epoch, to printable format using the conventions of strftime, plus %xf for fractional seconds, where x represents number of digits from 0 to 6.
* **tf2d format** - convert printable time format to a decimal number, representing seconds since the epoch. 

Words vs Fields
===============
So what is the difference between a word and a field? Two consecutive fields are separated by one field separator, while two consecutive words can be separated by any number of word separators.

For example, Suppose our input is `hello,,,,there` and we define both the word separator and the field separator to be a comma (,). Then both word 1 and field 1 are `hello`. However word 2 is `there`, but so is field 5. What happened to fields 2, 3, and 4? They are all zero-length strings.

Examples
========
Let's use the output of ls we used earlier again:
```
-rw-r--r--  1 root  wheel  2554 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/DiffieHellmanErrorPage.html
-rw-r--r--  1 root  wheel  2364 Oct 30 02:59 /Applications/Safari.app/Contents/Resources/en.lproj/InfoPlist.strings
-rw-r--r--  1 root  wheel  2048 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/JavaScriptErrorPage.html
-rw-r--r--  1 root  wheel  3000 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/OfflineReadingListErrorPage.html
-rw-r--r--  1 root  wheel  2038 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/ServerNotFoundErrorPage.html
-rw-r--r--  1 root  wheel   704 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/ServicesMenu.strings
-rw-r--r--  1 root  wheel  2038 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/StandardErrorPage.html
-rw-r--r--  1 root  wheel  2396 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/WebProcessCrashErrorPage.html
-rw-r--r--  1 root  wheel  6958 Oct 30 09:46 /Applications/Safari.app/Contents/Resources/en.lproj/localizedStrings.js
```
Let's arrange the line so that the filename is centered in the first 35 columns, and convert the date (in words 6-8) to the internal format (seconds since the epoch):
```
    specs substr fs / field -1 of word -1 1.35 center    w6-8 tf2d "%b %d %H:%M" nw
```
This *specification* contains two data fields which I've separated using multiple space characters. The result comes out something like this:
```
    DiffieHellmanErrorPage.html     1572421577.000000
         InfoPlist.strings          1572397157.000000
     JavaScriptErrorPage.html       1572421577.000000
 OfflineReadingListErrorPage.html   1572421577.000000
   ServerNotFoundErrorPage.html     1572421577.000000
       ServicesMenu.strings         1572421577.000000
      StandardErrorPage.html        1572421577.000000
   WebProcessCrashErrorPage.html    1572421577.000000
        localizedStrings.js         1572421577.000000
```
You may get different results depending on when you run this. Why? Because the time format looks like this: `Oct 30 09:46`, represented by the strftime string `%b %d %H:%M`. The year is missing, so what does the C++ function do? It replaces unspecified values with current values. So in this case, the year was supplied as 2019, even though the dates in the output are from 2018.

## Other Common *Spec Units*
Here are a few more common spec units:
### WORDSEPARATOR or WS
This is used to set the word separator character to something other than the default. For example:
```
specs wordseparator / word 2 1
```
will output `bye` from the folowing input record:
```
/Good///bye/old///paint
```

### FIELDSEPARATOR or FS
This is used to set the field separator character to something other than the default (tab). For example:
```
specs fieldseparator , /</ 1 f2 n />/ n /</ nw f4 n />/ n
```
This will produce the output `<bye><>` from this input record:
```
Good,bye,old,,paint
```