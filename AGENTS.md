# Agent Guidelines for specs2016

## General Project Guidelines

- When adding or removing tests in `specs/src/test/ProcessingTest.cc`, also update `count_processing_tests` in `specs/tests/valgrind_unit_tests.py`.
- When adding or removing tests in `specs/src/test/ALUUnitTest.cc`, also update `count_ALU_tests` in `specs/tests/valgrind_unit_tests.py`.
- When adding or removing tests in `specs/src/test/TokenTest.cc`, also update `count_token_tests` in `specs/tests/valgrind_unit_tests.py`.
- Keep any new `ProcessingTest.cc` regression cases appended at the end when possible, to avoid unnecessary renumbering.
- `MYASSERT(cond)` and `MYASSERT_WITH_MSG(cond, msg)` (defined in `specs/src/utils/ErrorReporting.h`) are **always-on runtime checks** that throw `SpecsException` via `MYTHROW`. They are *not* compiled out by `NDEBUG`. Do not add redundant `if`/`MYTHROW` guards that duplicate what a `MYASSERT` already covers.
- When building the project, always use the command-line `make clean all`. Do not skip the `clean` target, and do not use the `-j` argument.
- When changing the structure of any of the classes that have dump_ macros in `specs_gdb.py` and `specs.gdb` update the relevant macros as well.
- Command-line invocations of `specs` require ALL switches (`-i`, `-o`, `-f`, `--recfm`, `--shell`, etc.) to come *before* any spec units on the command line. `parseSwitches` (in `specs/src/test/specs.cc`) stops parsing switches as soon as it hits the first argument not starting with `-`, so a switch placed after a spec unit is silently ignored rather than applied. When writing or reviewing examples (e.g. in `specs/docs/guidebook.md` or other docs), always put switches first: `specs [switches] [spec-units]`, never `specs [spec-units] [switches]`.
- This project is based on the CMS pipelines stage `spec`. When it's not clear what the right thing to do is, it's a good idea to refer to chapter 16 and chapter 24 of the [CMS Pipelines User's Guide and Reference](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf).

## Syntax Highlighting Definition

`specs/utils/specs.xml` is a KDE-style syntax definition for specification files, used by Kate, KSyntaxHighlighting, skylighting and pandoc. It duplicates knowledge that lives in the sources, so it has to be kept in step with them:

- When adding, removing or renaming a spec-unit keyword or one of its accepted abbreviations in `specs/src/cli/tokens.cc` (the `SIMPLETOKEN` / `SIMPLETOKENV` block, or the `NEXTWORD`/`NEXTFIELD`/`NEXT` block), update the matching keyword list in `specs/utils/specs.xml`. Every abbreviation the tokenizer accepts is listed explicitly there.
- When changing `STRING_CONVERSIONS_LIST` or `PARAMETRIZED_CONVERSIONS_LIST` in `specs/src/processing/conversions.h`, update the `conversions` list in `specs/utils/specs.xml`.
- The `alufunctions` list in `specs/utils/specs.xml` is kept sorted alphabetically.
- After editing the file, check it with `xmllint --noout specs/utils/specs.xml`, and check the highlighting itself by rendering the sample specification `specs/utils/example.spec`:
  ```
  { echo '```specs'; cat specs/utils/example.spec; echo '```'; } | \
      pandoc -s --syntax-definition=specs/utils/specs.xml --highlight-style specs/utils/specs.theme \
             --metadata pagetitle=specs -o /tmp/example.html
  ```
  The `-s` (standalone) switch is what makes the output colourful: without it pandoc emits only the `<span class="...">` markup and none of the CSS that colours it, so the result looks black and white. Pandoc refuses to load a definition with unresolved `IncludeRules`, so this also verifies that the file stays self-contained. Keep it that way: do not add `IncludeRules` references to other syntax files (`##Comments`, `##Alerts`, and the like).
- To see *which* attribute produced each colour, read the class names out of the HTML (`co` = Comment, `kw` = Keyword, `cf` = ControlFlow, `dv` = DecVal, `va` = Variable, and so on).
- `specs/utils/specs.xml` is published as a GitHub release asset by `.github/workflows/release.yml` (the `syntax-xml` artifact).

## The Guidebook Highlighting Theme

The guidebook PDF is built with `--highlight-style specs/utils/specs.theme`, a skylighting theme derived from `pygments` (regenerate a starting point with `pandoc --print-highlight-style=pygments`). It is a build input only - unlike `specs.xml`, it is deliberately *not* a release asset. Three constraints shaped it, and they apply to any replacement:

- **No background colour.** A style that sets one makes pandoc emit `\usepackage{framed}`, and `framed.sty` is not in the CI TeX Live installation (`texlive-xetex texlive-fonts-recommended`). Of the built-in styles only `pygments`, `haddock` and `monochrome` qualify; `specs.theme` keeps `"background-color": null` for the same reason.
- **No token may rely on weight.** `header.tex` sets `\fvset{formatcom=\bfseries}` so that highlighted blocks match the weight of the plain verbatim blocks around them, which also makes every token bold and erases the per-token `bold` flags. In stock `pygments` that would merge `dsKeyword` with `dsOthers` (both `#007020`, differing only in weight), i.e. `SET`/`PRINT`/`WRITE` would become indistinguishable from `NEXTWORD`/`NEXT`. `specs.theme` gives `Other` its own colour instead.
- **Enough contrast for print.** The default palettes are tuned for screens and look pale on paper. Every colour used by a `specs.xml` attribute reaches at least **4.5:1** against white, which is the WCAG AA threshold for normal-size text. The relaxed 3:1 threshold does not apply here even though `formatcom` makes everything bold: that one starts at 14pt bold, and the guidebook sets code smaller than that. The ratio is `(L_lighter + 0.05) / (L_darker + 0.05)`, where white has `L = 1.0` and `L` is relative luminance, computed from the sRGB channels *after* linearising them:

  ```
  C' = C / 255
  C_lin = C'/12.92                  if C' <= 0.04045
        = ((C' + 0.055)/1.055)^2.4  otherwise
  L = 0.2126*R_lin + 0.7152*G_lin + 0.0722*B_lin
  ```

  Do not judge this with a plain weighted sum of the raw 8-bit channels: that is luma, not luminance, and it ranks these colours differently depending on whether the Rec. 709 or Rec. 601 coefficients are used.

A style is also free to leave a default style undefined, in which case those tokens come out plain black: `haddock` defines nothing for `dsAttribute`, `dsBuiltIn`, `dsConstant`, `dsDataType`, `dsDecVal`, `dsFloat`, `dsBaseN`, `dsFunction`, `dsOperator` or `dsVariable`, which is more than half of what `specs.xml` uses. For the 25 attributes in `specs.xml`, `specs.theme` currently yields 16 distinct renderings, `pygments` 13 and `haddock` 5.

Some attributes are close enough in colour to read as one, which is fine where they cannot appear next to each other. In increasing order of separation: `dsPreprocessor` (`#7a3e00`, Directive) and `dsOthers` (`#7a4300`, Shell Command) are effectively the same brown, and do meet on a `+SET` line; `dsBuiltIn` (`#006e00`, Input Source) and `dsChar` (`#008000`, Separator Char) never meet, since a separator character only follows `WS`/`FS`/`PAD`; `dsFunction` (`#06287e`) and `dsVariable` (`#19177c`) are both dark navy and do sit together inside expressions, but a function name is always followed by `(`.

## ALU Functions

- When adding or changing an ALU function - those are the functions defined in `aluFunctions.h` and implemented in `aluFunctions.cc` - document it in `manpage`, in `specs/docs/alu_adv.md`, and in `specs/docs/guidebook.md`.  Add unit tests as necessary in `specs/src/test/ALUUnitTest.cc`.  Add the function name to the `alufunctions` list in `specs/utils/specs.xml`.
- Always consider the exactness of the arguments to the ALU function and of the return value.

## Rolling Context Feature (Issue #106)

### Token Normalization in tokens.cc

The CONTEXT keyword requires special handling during token normalization (`normalizeTokenList` in `tokens.cc`):

1. **Initial State**: When a CONTEXT token is first parsed, its `Literal()` field is empty.

2. **Normalization Process** (lines 920-951 in tokens.cc):
   - Check if the next token exists: `i+1 < tokList->size()`
   - If yes, extract the integer offset from the next token (which can be a RANGE or a literal)
   - Store the offset string in the CONTEXT token's literal field
   - Erase the next token from the list
   - If no next token exists, the literal remains empty

3. **Compile-Time Validation** (lines 473-489 in specItems.cc):
   - When `itemGroup::Compile` processes a CONTEXT token, it must validate that the literal is not empty
   - Use `tokenVec[index].argIndex()` (not the vector index) for error messages
   - Call `std::stoi(tokenVec[index].Literal())` only after validation

### Key Insight

The `if` statement at line 923 in tokens.cc checks `tok.Literal()==""` because:
- If there is no next token (`i+1 >= tokList->size()`), the condition is false
- The literal is never set
- The error is caught at compile time in specItems.cc with a proper error message

**Example that triggers the error:**
```
specs '1-* 1 CONTEXT'
```
This produces: "CONTEXT at index X must be followed by an integer offset"

### Testing

All rolling context tests are in `specs/src/test/ProcessingTest.cc` (tests #229-#241).
Run with: `../exe/ProcessingTest`

### Verbose Output

When rolling context is used and verbose mode is enabled (`-v` flag), the buffer sizes are printed:
```
Rolling context buffer sizes: forward=<n> backward=<m>
```

This is implemented in `specs/src/test/specs.cc` after the compilation phase.
