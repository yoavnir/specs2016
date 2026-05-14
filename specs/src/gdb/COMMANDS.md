# GDB Dump Commands Reference

## InputPart Hierarchy Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `dump-input-part` | — | Dump an InputPart (polymorphic) |
| `dump-literal-part` | — | Dump a LiteralPart |
| `dump-range-part` | — | Dump a RangePart |
| `dump-word-range-part` | — | Dump a WordRangePart |
| `dump-field-range-part` | — | Dump a FieldRangePart |
| `dump-clock-part` | — | Dump a ClockPart |
| `dump-id-part` | — | Dump an IDPart |
| `dump-expression-part` | `dump_expr_part` | Dump an ExpressionPart |

## Item Hierarchy Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `dump-item` | `dump_item` | Dump an Item (polymorphic) |
| `dump-data-field` | `dump_data_field` | Dump a DataField |
| `dump-token-item` | `dump_token_item` | Dump a TokenItem |
| `dump-set-item` | `dump_set_item` | Dump a SetItem |
| `dump-skip-item` | `dump_skip_item` | Dump a SkipItem |
| `dump-condition-item` | `dump_condition_item` | Dump a ConditionItem |
| `dump-break-item` | `dump_break_item` | Dump a BreakItem |
| `dump-select-item` | `dump_select_item` | Dump a SelectItem |
| `dump-split-item` | `dump_split_item` | Dump a SplitItem |

## itemGroup Command

| Command | Alias | Description |
|---------|-------|-------------|
| `dump-item-group` | `dump_items` | Dump an itemGroup |

## Token System Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `dump-token` | `dump_token` | Dump a Token |
| `dump-token-range` | `dump_token_range` | Dump a TokenFieldRange |

## Processing Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `dump-processing-state` | `dump_pstate` | Dump a ProcessingState |
| `dump-string-builder` | `dump_sb` | Dump a StringBuilder |
| `dump-reader` | `dump_reader` | Dump a Reader |
| `dump-writer` | `dump_writer` | Dump a Writer |

## ALU Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `dump-alu-value` | `dump_alu_value` | Dump an ALUValue |
| `dump-alu-counters` | `dump_alu_counters` | Dump ALUCounters |
| `dump-alu-unit` | `dump_alu_unit` | Dump an AluUnit (polymorphic) |
| `dump-alu-vec` | `dump_alu_vec` | Dump an AluVec |
| `dump-alu-value-stats` | `dump_alu_stats` | Dump AluValueStats |
| `dump-frequency-map` | `dump_freq_map` | Dump a frequencyMap |


## Python Interface Commands

|| Command | Alias | Description |
||---------|-------|-------------|
|| `dump-alu-function` | `dump_alu_function` | Dump an AluFunction (name, arg count, input dependency) |
|| `dump-external-function-rec` | `dump_external_func_rec` | Dump an ExternalFunctionRec (calls virtual methods GetArgCount/GetFuncPtr) |
|| `dump-external-function-collection` | `dump_external_func_collection` | Dump an ExternalFunctionCollection (initialization state) |
|| `dump-python-function-collection` | `dump_python_func_collection` | Dump a PythonFunctionCollection (registry state and function count) |
|| `dump-python-func-rec` | `dump_python_func_rec` | Dump a PythonFuncRec (name, pointer, doc, and expanded argument list) |
|| `dump-python-func-arg` | `dump_python_func_arg` | Dump a PythonFuncArg (name, default type, and default value) |

## Utility Commands

|| Command | Alias | Description |
||---------|-------|-------------|
|| `dump-exception` | `dump_exception` | Dump a SpecsException |
|| `dump-all` | — | Dump all relevant debugging info |

## Breakpoint Helpers

|| Command | Description |
||---------|-------------|
|| `bp_apply` | Set breakpoint on Item::apply |
|| `bp_getstr` | Set breakpoint on InputPart::getStr |
|| `bp_compile` | Set breakpoint on itemGroup::Compile |
|| `bp_parseAluExpression` | Set breakpoint on parseAluExpression, where expressions are parsed |
|| `bp_pfc_initialize` | Set breakpoint on PythonFunctionCollection::Initialize, where the Python Function Collection is initialized |
|| `bp_func_setargvalue` | Set breakpoint on PythonFuncRec::setArgValue, where an argument for an external function is set |
|| `bp_func_call` | Set breakpoint on PythonFuncRec::Call, where an external function is invoked |

## Usage Examples

### Dump ProcessingState
```gdb
(gdb) dump_pstate pState
ProcessingState @ 0x7fffffffde00
  Current Record:    "hello world"
  Previous Record:   "goodbye world"
  Pad Char:          ' ' (0x20)
  Word Separator:    " "
  Field Separator:   "\t"
  Cycle Counter:     42
  ...
```

### Dump Item
```gdb
(gdb) dump_item myItem
Item @ 0x...
  m_originalIndex: 0
  Debug: {Source=Range[1:10];Dest=@10L20}
  readsLines: true
  producesOutput: true
  ...
```

### Dump ALUValue
```gdb
(gdb) dump_alu_value myValue
ALUValue @ 0x...
  m_type: Int
  m_value: "42"
  m_exact: true
```

### Set Breakpoint on Item::apply
```gdb
(gdb) bp_apply
Breakpoint 1 at 0x...
(gdb) run
Breakpoint 1, Item::apply (this=0x..., pState=0x..., pSB=0x...) at specitems/specItems.cc:...
(gdb) dump_pstate pState
```

## Getting Help

To get help on any command:
```gdb
(gdb) help dump-processing-state
(gdb) help dump-item
(gdb) help dump_pstate
```

## Notes

- Commands with aliases can be called either way: `dump-processing-state` or `dump_pstate`
- Polymorphic commands (marked with "(polymorphic)") automatically detect the actual derived type
- All dump commands are safe to call even if the inferior is in a bad state
- Virtual method calls (like `Debug()`) are guarded with error handling
