# specs GDB convenience macros and initialization
# This file sources the Python GDB extension and provides shorthand commands

# Source the Python extension
python
import sys, os
# Try to find specs_gdb.py in common locations
search_paths = [
    os.path.join(os.getcwd(), 'gdb'),                    # Current dir + gdb/
    os.path.join(os.getcwd(), 'specs', 'src', 'gdb'),    # specs/src/gdb from project root
    os.path.join(os.getcwd(), '..', 'gdb'),              # Parent dir + gdb/
    'gdb',                                                 # Just gdb/ in current dir
]

gdb_script_dir = None
for path in search_paths:
    if os.path.isfile(os.path.join(path, 'specs_gdb.py')):
        gdb_script_dir = path
        break

if gdb_script_dir:
    sys.path.insert(0, gdb_script_dir)
    import specs_gdb
else:
    print("Warning: Could not find specs_gdb.py in expected locations")
    print("Searched: " + ", ".join(search_paths))
end

# ============================================================================
# CONVENIENCE ALIASES FOR DUMP COMMANDS
# ============================================================================

# InputPart hierarchy
define dump_literal_part
  dump-literal-part $arg0
end

define dump_range_part
  dump-range-part $arg0
end

define dump_word_range_part
  dump-word-range-part $arg0
end

define dump_field_range_part
  dump-field-range-part $arg0
end

define dump_clock_part
  dump-clock-part $arg0
end

define dump_id_part
  dump-id-part $arg0
end

define dump_expr_part
  dump-expression-part $arg0
end

# Item hierarchy
define dump_item
  dump-item $arg0
end

define dump_data_field
  dump-data-field $arg0
end

define dump_token_item
  dump-token-item $arg0
end

define dump_set_item
  dump-set-item $arg0
end

define dump_skip_item
  dump-skip-item $arg0
end

define dump_condition_item
  dump-condition-item $arg0
end

define dump_break_item
  dump-break-item $arg0
end

define dump_context_item
  dump-context-item $arg0
end

define dump_select_item
  dump-select-item $arg0
end

define dump_split_item
  dump-split-item $arg0
end

# itemGroup
define dump_items
  dump-item-group $arg0
end

# Token system
define dump_token
  dump-token $arg0
end

define dump_token_range
  dump-token-range $arg0
end

# Processing
define dump_pstate
  dump-processing-state $arg0
end

define dump_sb
  dump-string-builder $arg0
end

define dump_reader
  dump-reader $arg0
end

define dump_writer
  dump-writer $arg0
end

# ALU
define dump_alu_value
  dump-alu-value $arg0
end

define dump_alu_counters
  dump-alu-counters $arg0
end

define dump_alu_unit
  dump-alu-unit $arg0
end

define dump_alu_vec
  dump-alu-vec $arg0
end

define dump_alu_stats
  dump-alu-value-stats $arg0
end

define dump_freq_map
  dump-frequency-map $arg0
end

# Python interface
define dump_alu_function
  dump-alu-function $arg0
end

define dump_external_func_rec
  dump-external-function-rec $arg0
end

define dump_external_func_collection
  dump-external-function-collection $arg0
end

define dump_python_func_collection
  dump-python-function-collection $arg0
end

define dump_python_func_rec
  dump-python-func-rec $arg0
end

define dump_python_func_arg
  dump-python-func-arg $arg0
end

# Utilities
define dump_exception
  dump-exception $arg0
end

# ============================================================================
# USEFUL BREAKPOINT HELPERS
# ============================================================================

define bp_apply
  break Item::apply
end
document bp_apply
  Set a breakpoint on Item::apply to debug item application.
end

define bp_getstr
  break InputPart::getStr
end
document bp_getstr
  Set a breakpoint on InputPart::getStr to debug input part string extraction.
end

define bp_compile
  break itemGroup::Compile
end
document bp_compile
  Set a breakpoint on itemGroup::Compile to debug specification compilation.
end

define bp_parseAluExpression
  break parseAluExpression
end
document bp_parseAluExpression
  Set a breakpoint on parseAluExpression to debug parsing of mathematical expressions.
end

define bp_context
  break ContextItem::apply
end
document bp_context
  Set a breakpoint on ContextItem::apply to debug rolling context operations.
end

define bp_pfc_initialize
  break PythonFunctionCollection::Initialize
end
document bp_pfc_initialize
  Set a breakpoint on PythonFunctionCollection::Initialize to debug the initialization of the Python Function Collection.
end

define bp_func_setargvalue
  break PythonFuncRec::setArgValue
end
document bp_func_setargvalue
  Set a breakpoint on PythonFuncRec::setArgValue to debug setting external function arguments.
end

define bp_func_call
  break PythonFuncRec::Call
end
document bp_func_call
  Set a breakpoint on PythonFuncRec::Call to debug calling external functions.
end

# ============================================================================
# USEFUL GDB SETTINGS FOR SPECS DEBUGGING
# ============================================================================

set print pretty on
set print array on
set print array-indexes on
set print object on
set print static-members on

# ============================================================================
# WELCOME MESSAGE
# ============================================================================

echo \n
echo ========================================\n
echo specs GDB debugging macros loaded\n
echo ========================================\n
echo \n
echo Available commands:\n
echo   dump_pstate <var>              - Dump ProcessingState\n
echo   dump_sb <var>                  - Dump StringBuilder\n
echo   dump_item <var>                - Dump Item (polymorphic)\n
echo   dump_items <var>               - Dump itemGroup\n
echo   dump_token <var>               - Dump Token\n
echo   dump_alu_value <var>           - Dump ALUValue\n
echo   dump_alu_counters <var>        - Dump ALUCounters\n
echo   dump_alu_vec <var>             - Dump AluVec\n
echo   dump_alu_function <var>        - Dump AluFunction\n
echo   dump_external_func_rec <var>   - Dump ExternalFunctionRec\n
echo   dump_python_func_rec <var>     - Dump PythonFuncRec\n
echo   dump_python_func_arg <var>     - Dump PythonFuncArg\n
echo   dump_exception <var>           - Dump SpecsException\n
echo \n
echo Breakpoint helpers:\n
echo   bp_apply                       - Break on Item::apply\n
echo   bp_getstr                      - Break on InputPart::getStr\n
echo   bp_compile                     - Break on itemGroup::Compile\n
echo   bp_parseAluExpression          - Break on parseAluExpression, where expressions are parsed\n
echo   bp_pfc_initialize              - Break on PythonFunctionCollection::Initialize, where the Python Function Collection is initialized\n
echo   bp_func_setargvalue            - Break on PythonFuncRec::setArgValue, where an argument for an external function is set\n
echo   bp_func_call                   - Break on PythonFuncRec::Call, where an external function is invoked\n
echo \n
echo For more help, type: help dump-processing-state\n
echo \n
