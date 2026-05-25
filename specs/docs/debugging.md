# Using GDB to Debug specs

This guide explains how to use the GDB debugging toolkit for specs, which provides convenient "dump" commands for inspecting all major classes and data structures during debugging.

## Table of Contents

1. [Building with Debug Symbols](#building-with-debug-symbols)
2. [Loading the GDB Macros](#loading-the-gdb-macros)
3. [Quick Reference](#quick-reference)
4. [Detailed Examples](#detailed-examples)
5. [Tips and Tricks](#tips-and-tricks)
6. [Troubleshooting](#troubleshooting)

---

## Building with Debug Symbols

To debug specs effectively, you must build with debug symbols enabled.

### On Linux and macOS

```bash
cd specs/src
python3 setup.py -v DEBUG
make clean all
```

The `-v DEBUG` flag tells the setup script to enable debug symbols and disable optimizations, making it easier to inspect variables and step through code.

### On Windows

```cmd
msbuild specs\specs.sln /p:Configuration=Debug /p:Platform=x64
```

However, `gdb` is not normally the debugger that you use on Windows.

---

## Loading the GDB Macros

### Automatic Loading (Recommended)

When you run GDB from the `specs/src/` directory, the `.gdbinit` file is automatically loaded. So you can add the `specs.gdb` file to `.gdbinit`. Or you can specify it on the command line:

```bash
cd specs/src
gdb ../exe/specs -x gdb/specs.gdb
```

### Manual Loading

If you're running GDB from a different directory, you can manually load the macros from within GDB:

```
(gdb) source gdb/specs.gdb
```

### Verify Loading

After loading, you should see a welcome message:

```
specs GDB extension loaded successfully

========================================
specs GDB debugging macros loaded
========================================

Available commands:
dump_pstate <var>              - Dump ProcessingState
dump_sb <var>                  - Dump StringBuilder
dump_item <var>                - Dump Item (polymorphic)
dump_items <var>               - Dump itemGroup
dump_token <var>               - Dump Token
dump_alu_value <var>           - Dump ALUValue
dump_alu_counters <var>        - Dump ALUCounters
dump_alu_vec <var>             - Dump AluVec
dump_alu_function <var>        - Dump AluFunction
dump_external_func_rec <var>   - Dump ExternalFunctionRec
dump_python_func_collection <var> - Dump PythonFunctionCollection
dump_python_func_by_name <col> <name> - Dump PythonFuncRec by name
dump_python_func_rec <var>     - Dump PythonFuncRec
dump_python_func_arg <var>     - Dump PythonFuncArg
dump_exception <var>           - Dump SpecsException

Breakpoint helpers:
bp_apply                       - Break on all 9 Item subclass apply methods
bp_getstr                      - Break on InputPart::getStr
bp_compile                     - Break on itemGroup::Compile
bp_parseAluExpression          - Break on parseAluExpression, where expressions are parsed
bp_pfc_initialize              - Break on PythonFunctionCollection::Initialize, where the Python Function Collection is initialized
bp_func_setargvalue            - Break on PythonFuncRec::setArgValue, where an argument for an external function is set
bp_func_call                   - Break on PythonFuncRec::Call, where an external function is invoked

For more help, type: help dump-processing-state
```

---

## Quick Reference

### ProcessingState Commands

| Command | Purpose |
|---------|---------|
| `dump_pstate <var>` | Dump the entire ProcessingState (current record, cycle counter, separators, etc.) |
| `dump_sb <var>` | Dump the StringBuilder (output string being built, current position) |

### Item Commands

| Command | Purpose |
|---------|---------|
| `dump_item <var>` | Dump an Item (polymorphic; detects DataField, TokenItem, etc.) |
| `dump_items <var>` | Dump an itemGroup (list of all compiled spec items) |
| `dump_data_field <var>` | Dump a DataField (input source, output placement, conversion) |
| `dump_token_item <var>` | Dump a TokenItem |
| `dump_set_item <var>` | Dump a SetItem (assignment expression) |
| `dump_condition_item <var>` | Dump a ConditionItem (IF/WHILE/etc.) |
| `dump_split_item <var>` | Dump a SplitItem (SPLITW/SPLITF) |

### InputPart Commands

| Command | Purpose |
|---------|---------|
| `dump_literal_part <var>` | Dump a LiteralPart (literal string) |
| `dump_range_part <var>` | Dump a RangePart (character range) |
| `dump_word_range_part <var>` | Dump a WordRangePart (word range with separator) |
| `dump_field_range_part <var>` | Dump a FieldRangePart (field range with separator) |
| `dump_clock_part <var>` | Dump a ClockPart (time value) |
| `dump_id_part <var>` | Dump an IDPart (field identifier) |
| `dump_expr_part <var>` | Dump an ExpressionPart (ALU expression) |

### Token Commands

| Command | Purpose |
|---------|---------|
| `dump_token <var>` | Dump a Token (type, literal, original text) |
| `dump_token_range <var>` | Dump a TokenFieldRange (range specification) |

### ALU Commands

| Command | Purpose |
|---------|---------|
| `dump_alu_value <var>` | Dump an ALUValue (type, value, exactness) |
| `dump_alu_counters <var>` | Dump ALUCounters (all counter variables) |
| `dump_alu_unit <var>` | Dump an AluUnit (polymorphic; literal, counter, operator, etc.) |
| `dump_alu_vec <var>` | Dump an AluVec (vector of AluUnits) |
| `dump_alu_stats <var>` | Dump AluValueStats (statistical data) |
| `dump_freq_map <var>` | Dump a frequencyMap (frequency distribution) |

### Utility Commands

| Command | Purpose |
|---------|---------|
| `dump_reader <var>` | Dump a Reader (record count, EOF state) |
| `dump_writer <var>` | Dump a Writer (output count) |
| `dump_exception <var>` | Dump a SpecsException (file, line, message) |

### Python Interface Commands

|| Command | Purpose |
||---------|----------|
|| `dump_alu_function <var>` | Dump an AluFunction (name, arg count, input dependency) |
|| `dump_external_func_rec <var>` | Dump an ExternalFunctionRec (polymorphic base class) |
|| `dump_external_func_collection <var>` | Dump an ExternalFunctionCollection (initialization state) |
|| `dump_python_func_collection <var>` | Dump a PythonFunctionCollection (internal Python function registry) |
|| `dump_python_func_rec <var>` | Dump a PythonFuncRec (Python function record with name and args) |
|| `dump_python_func_arg <var>` | Dump a PythonFuncArg (function argument with default value) |

### Breakpoint Helpers

| Command | Purpose |
|---------|---------|
| `bp_apply` | Set breakpoint on Item::apply |
| `bp_getstr` | Set breakpoint on InputPart::getStr |
| `bp_compile` | Set breakpoint on itemGroup::Compile |

---

## Detailed Examples

### Example 1: Inspecting ProcessingState During Execution

Suppose you're debugging a spec that processes records and you want to see the current state:

```
(gdb) break Item::apply
Breakpoint 1 at 0x...

(gdb) run < input.txt
Starting program: ./specs ...
Breakpoint 1, Item::apply (this=0x..., pState=0x..., pSB=0x...) at specitems/specItems.cc:...

(gdb) dump_pstate pState
ProcessingState @ 0x7fffffffde00
  Current Record:    "hello world"
  Previous Record:   "goodbye world"
  Pad Char:          ' ' (0x20)
  Word Separator:    " "
  Field Separator:   "\t"
  Cycle Counter:     42
  Extra Reads:       0
  Record Count:      42
  Word Count:        2
  Field Count:       1
  Input Station:     -1
  Input Stream:      1
  Output Index:      1
  No Write:          false
  EOF:               false
```

This shows you exactly what the current record is, how many times we've processed records, and the current separators.

### Example 2: Inspecting a DataField

When debugging a data field specification:

```
(gdb) dump_data_field pDataField
DataField @ 0x...
  m_label: A
  m_outStart: 10
  m_maxLength: 20
  m_strip: true
  m_conversion: UCASE
  m_alignment: Left
```

This tells you that the field is labeled 'A', outputs starting at column 10, has a max length of 20 characters, strips whitespace, converts to uppercase, and is left-aligned.

### Example 3: Walking an itemGroup

To see the entire compiled specification:

```
(gdb) dump_items pItemGroup
itemGroup @ 0x...
  bNeedRunoutCycle: true
  bFoundSelectSecond: false
  Item count: 5
  Items:
    [0] @ 0x...
    [1] @ 0x...
    [2] @ 0x...
    [3] @ 0x...
    [4] @ 0x...
```

Then you can inspect individual items:

```
(gdb) dump_item pItemGroup.m_items[0]
Item @ 0x...
  m_originalIndex: 0
  Debug: {Source=Range[1:10];Dest=@10L20}
  readsLines: true
  producesOutput: true
  forcesRunoutCycle: false
  isBreak: false
```

### Example 4: Examining ALU Expressions

When debugging expression evaluation:

```
(gdb) dump_alu_value myALUValue
ALUValue @ 0x...
  m_type: Int
  m_value: "42"
  m_exact: true

(gdb) dump_alu_counters g_counters
ALUCounters @ 0x...
  Counters (map):
    m_map @ 0x...
```

### Example 5: Conditional Breakpoints with Cycle Counter

To break only on a specific record number:

```
(gdb) break Item::apply if pState.m_CycleCounter == 100
Breakpoint 1 at 0x...

(gdb) run < input.txt
...
Breakpoint 1, Item::apply (this=0x..., pState=0x..., pSB=0x...) at specitems/specItems.cc:...

(gdb) dump_pstate pState
ProcessingState @ 0x...
  Cycle Counter:     100
  ...
```

This is useful for debugging issues that only occur on specific records.

### Example 6: Debugging Python Function Integration

When debugging Python function calls and integration:

```
(gdb) break PythonIntf.cc:167
Breakpoint 1 at 0x...

(gdb) run -f myspec.txt < input.txt
...
Breakpoint 1, PyObject_CallObject (...) at PythonIntf.cc:167

(gdb) dump_python_func_collection gFunctionCollection
PythonFunctionCollection @ 0x...
  Initialized: true
  Functions (3 entries):
    my_custom_function @ 0x... (2 args)
    another_func @ 0x... (exact, 1 args)
    third_func @ 0x... (0 args)

(gdb) dump_python_func_by_name gFunctionCollection my_custom_function
PythonFuncRec @ 0x...
  Name: my_custom_function
  Func Ptr: 0x...
  doc: Computes the custom value based on input
  Arg Type: no exactness information
  Tuple: 0x...
  Args (2 items):
    [0] input_value (default: Int)
         = 0
    [1] multiplier (default: Float)
         = 1.5
```

This shows you the complete function signature, documentation, and argument defaults. The `Tuple` field shows whether arguments have been prepared for the function call. The collection dump now lists all functions with their key properties (exactness and argument count).

---

## Tips and Tricks

### 1. Using Pretty-Printers

The GDB extension includes pretty-printers for common types. When you print a variable, it's automatically formatted nicely:

```
(gdb) p myALUValue
$1 = ALUValue {type: Int, value: "42", exact: true}

(gdb) p myToken
$2 = Token {type: RANGE, literal: "", argc: 0}
```

### 2. Inspecting Shared Pointers

The extension can dereference `std::shared_ptr` automatically:

```
(gdb) p myDataField.m_InputPart
$3 = std::shared_ptr<InputPart> (use count=2, weak count=0) 0x...

(gdb) dump_input_part myDataField.m_InputPart
InputPart @ 0x...
  Debug: Range[1:10]
  readsLines: true
  forcesRunoutCycle: false
```

### 3. Setting Breakpoints on Virtual Methods

Since specs uses polymorphism extensively, you can break on virtual methods:

```
(gdb) break InputPart::getStr
Breakpoint 1 at 0x...

(gdb) run < input.txt
...
Breakpoint 1, LiteralPart::getStr (this=0x..., pState=...) at specitems/InputPart.cc:...
```

GDB will break on any derived class's implementation.

### 4. Examining the Specification Before Execution

You can set a breakpoint at the start of processing and dump the entire compiled spec:

```
(gdb) break itemGroup::process
Breakpoint 1 at 0x...

(gdb) run < input.txt
...
Breakpoint 1, itemGroup::process (this=0x..., sb=..., pState=..., rd=..., tmr=...) at specitems/specItems.cc:...

(gdb) dump_items this
itemGroup @ 0x...
  Item count: 5
  Items:
    [0] @ 0x...
    [1] @ 0x...
    ...
```

### 5. Tracking State Changes

Use conditional breakpoints to track when state changes:

```
(gdb) break ProcessingState::setString
Breakpoint 1 at 0x...

(gdb) commands
> dump_pstate this
> continue
> end

(gdb) run < input.txt
ProcessingState @ 0x...
  Current Record:    "line 1"
  Cycle Counter:     1
ProcessingState @ 0x...
  Current Record:    "line 2"
  Cycle Counter:     2
...
```

---

## Troubleshooting

### Python Extension Not Loading

**Error:** `ImportError: No module named specs_gdb`

**Solution:** Make sure you're running GDB from the `specs/src/` directory, or manually source the `.gdb` file:

```bash
cd specs/src
gdb ./specs
```

Or:

```bash
gdb ./specs -x specs/src/gdb/specs.gdb
```

### Dump Commands Not Found

**Error:** `Undefined command: "dump_pstate"`

**Solution:** The Python extension may not have loaded. Check that the `.gdb` file was sourced:

```
(gdb) source specs/src/gdb/specs.gdb
```

### Unable to Read Variables

**Error:** `Error: <unable to read string>`

**Solution:** This usually means the inferior (the running program) is in a bad state or the variable is uninitialized. Try:

1. Step to a different location in the code
2. Check that the variable is actually in scope
3. Use `info locals` to see available variables

### Calling Methods Fails

**Error:** `Error: <unable to call Debug()>`

**Solution:** Some methods may not be callable if the inferior is corrupted or in an inconsistent state. This is normal. The dump commands will still show the raw member variables.

### GDB Crashes When Calling Methods

**Solution:** If calling virtual methods causes GDB to crash, you can disable method invocation by editing `specs_gdb.py` and commenting out the `call_method_safe` calls.

---

## Building and Debugging Tips

### Debugging a Specific Spec

Create a test input file and a spec file, then run:

```bash
cd specs/src
gdb ./specs
(gdb) set args -f myspec.txt < input.txt
(gdb) break itemGroup::Compile
(gdb) run
(gdb) dump_items this
```

### Debugging Parsing

To debug specification parsing:

```bash
(gdb) break itemGroup::Compile
(gdb) run -f myspec.txt < input.txt
(gdb) dump_items this
```

### Debugging Expression Evaluation

To debug ALU expression evaluation:

```bash
(gdb) break AluFunction::evaluate
(gdb) run -f myspec.txt < input.txt
(gdb) dump_alu_unit this
```

---

## See Also

- [specs User Manual](basicspec.md)
- [ALU Reference](alu.md)
- [GDB Manual](https://sourceware.org/gdb/documentation/)
