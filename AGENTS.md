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

## ALU Functions

- When adding or changing an ALU function - those are the functions defined in `aluFunctions.h` and implemented in `aluFunctions.cc` - document it in `manpage`, in `specs/docs/alu_adv.md`, and in `specs/docs/guidebook.md`.  Add unit tests as necessary in `specs/src/test/ALUUnitTest.cc`.
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
