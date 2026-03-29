# Agents Notes

- When adding or removing tests in `specs/src/test/ProcessingTest.cc`, also update `count_processing_tests` in `specs/tests/valgrind_unit_tests.py`.
- When adding or removing tests in `specs/src/test/ALUUnitTest.cc`, also update `count_ALU_tests` in `specs/tests/valgrind_unit_tests.py`.
- When adding or removing tests in `specs/src/test/TokenTest.cc`, also update `count_token_tests` in `specs/tests/valgrind_unit_tests.py`.
- Keep any new `ProcessingTest.cc` regression cases appended at the end when possible, to avoid unnecessary renumbering.
- `MYASSERT(cond)` and `MYASSERT_WITH_MSG(cond, msg)` (defined in `specs/src/utils/ErrorReporting.h`) are **always-on runtime checks** that throw `SpecsException` via `MYTHROW`. They are *not* compiled out by `NDEBUG`. Do not add redundant `if`/`MYTHROW` guards that duplicate what a `MYASSERT` already covers.
- When building the project, always use the command-line `make clean all`. Do not skip the `clean` target, and do not use the `-j` argument.
