#!/usr/bin/env python3
"""
GDB debugging extension for specs.

Provides pretty-printers and dump commands for all major classes and structs
in the specs codebase, organized by subsystem.

Usage:
    (gdb) source specs/src/gdb/specs.gdb
    (gdb) dump-processing-state pState
    (gdb) dump-item myItem
    (gdb) dump-alu-value myALUValue
"""

import gdb
import gdb.printing
import struct
import sys

# ============================================================================
# ENUM DECODE HELPERS
# ============================================================================

CLOCK_TYPE = {
    0: "Static",
    1: "Dynamic",
    2: "Diff",
}

APPLY_RET = {
    0: "Continue",
    1: "ContinueWithDataWritten",
    2: "Write",
    3: "Read",
    4: "ReadStop",
    5: "EnterLoop",
    6: "DoneLoop",
    7: "EOF",
    8: "UNREAD",
    9: "ReDo",
    10: "Break",
    11: "SkipToNext",
    12: "SplitStart",
    13: "SplitContinue",
}

ALU_COUNTER_TYPE = {
    0: "None",
    1: "Str",
    2: "Int",
    3: "Float",
}

OUTPUT_ALIGNMENT = {
    0: "Left",
    1: "Center",
    2: "Right",
    3: "Composed",
}

ALU_UNIT_TYPE = {
    0: "Invalid",
    1: "None",
    2: "OpenParenthesis",
    3: "ClosingParenthesis",
    4: "Comma",
    5: "Identifier",
    6: "LiteralNumber",
    7: "Counter",
    8: "FieldIdentifier",
    9: "UnaryOp",
    10: "BinaryOp",
    11: "AssignmentOp",
    12: "InputRecord",
    13: "Null",
}

ALU_UNARY_OP = {
    0: "Plus",
    1: "Minus",
    2: "Not",
}

ALU_BINARY_OP = {
    0: "Add",
    1: "Sub",
    2: "Mult",
    3: "Div",
    4: "IntDiv",
    5: "RemDiv",
    6: "Appnd",
    7: "LT",
    8: "LE",
    9: "GT",
    10: "GE",
    11: "SLT",
    12: "SLTE",
    13: "SGT",
    14: "SGTE",
    15: "EQ",
    16: "SEQ",
    17: "NE",
    18: "SNE",
    19: "AND",
    20: "OR",
}

ALU_ASSN_OP = {
    0: "Let",
    1: "Add",
    2: "Sub",
    3: "Mult",
    4: "Div",
    5: "RemDiv",
    6: "IntDiv",
    7: "Appnd",
}

CONDITION_PREDICATE = {
    0: "IF",
    1: "THEN",
    2: "ELSE",
    3: "ELSEIF",
    4: "ENDIF",
    5: "WHILE",
    6: "DO",
    7: "DONE",
    8: "ASSERT",
}

RECORD_FORMAT = {
    0: "DELIMITED",
    1: "FIXED",
    2: "FIXED_DELIMITED",
}

TIME_CLASSES = {
    0: "Initializing",
    1: "Processing",
    2: "IO",
    3: "InputQueue",
    4: "OutputQueue",
    5: "Draining",
    6: "Last",
}

WRITER_TYPE = {
    0: "COUT",
    1: "CERR",
    2: "SHELL",
    3: "FILE",
}

EXTERNAL_FUNC_ERROR_HANDLING = {
    0: "Throw",
    1: "NaN",
    2: "Zero",
    3: "NullStr",
}

EXTREME_BOOL = {
    0: "False",
    1: "True",
    2: "DontCare",
}

INPUT_STATION = {
    -1: "FIRST",
    -2: "SECOND",
    0: "STDERR",
}

# Token types (X-macro generated, simplified list)
TOKEN_TYPES = {
    0: "STOP",
    1: "ALLEOF",
    2: "ANYEOF",
    3: "COUNTERS",
    4: "PRINTONLY",
    5: "EOF",
    6: "KEEP",
    7: "READ",
    8: "READSTOP",
    9: "WRITE",
    10: "NOWRITE",
    11: "ASSERT",
    12: "ABEND",
    13: "RANGELABEL",
    14: "ID",
    15: "PERIOD",
    16: "RANGE",
    17: "WORDRANGE",
    18: "FIELDSEPARATOR",
    19: "WORDSEPARATOR",
    20: "PAD",
    21: "NEXTWORD",
    22: "NEXTFIELD",
    23: "NEXT",
    24: "FIELDRANGE",
    25: "SUBSTRING",
    26: "OF",
    27: "GROUPSTART",
    28: "GROUPEND",
    29: "STRIP",
    30: "LITERAL",
    31: "CONVERSION",
    32: "LEFT",
    33: "CENTER",
    34: "RIGHT",
    35: "NUMBER",
    36: "TODCLOCK",
    37: "DTODCLOCK",
    38: "TIMEDIFF",
    39: "SET",
    40: "PRINT",
    41: "IF",
    42: "THEN",
    43: "ELSE",
    44: "ELSEIF",
    45: "ENDIF",
    46: "CONTINUE",
    47: "WHILE",
    48: "DO",
    49: "DONE",
    50: "UNREAD",
    51: "REDO",
    52: "BREAK",
    53: "SELECT",
    54: "FIRST",
    55: "SECOND",
    56: "OUTSTREAM",
    57: "STDERR",
    58: "REQUIRES",
    59: "SKIPUNTIL",
    60: "SKIPWHILE",
    61: "SPLITW",
    62: "SPLITF",
    63: "DUMMY",
}

STRING_CONVERSIONS = {
    0: "identity",
    1: "ROT13",
    2: "C2B",
    3: "C2X",
    4: "B2C",
    5: "X2CH",
    6: "D2X",
    7: "X2D",
    8: "LCASE",
    9: "UCASE",
    10: "BSWAP",
    11: "ti2f",
    12: "tf2i",
    13: "s2tf",
    14: "tf2s",
    15: "mcs2tf",
    16: "tf2mcs",
    17: "NONE",
}

# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

def deref_shared_ptr(val):
    """Dereference a std::shared_ptr to get the pointee."""
    try:
        # libstdc++ layout: shared_ptr has _M_ptr member
        ptr_val = val["_M_ptr"]
        if ptr_val == 0:
            return None
        return ptr_val.dereference()
    except:
        try:
            # Alternative: try to dereference directly
            return val.dereference()
        except:
            return None

def std_string_to_str(val):
    """Extract a Python string from a std::string."""
    try:
        # Try to get the string value directly
        return val.string()
    except:
        try:
            # Fallback: access _M_dataplus._M_p
            return val["_M_dataplus"]["_M_p"].string()
        except:
            return "<unable to read string>"

def std_vector_size(val):
    """Get the size of a std::vector."""
    try:
        return int(val["_M_impl"]["_M_finish"] - val["_M_impl"]["_M_start"])
    except:
        return 0

def std_map_size(val):
    """Get the size of a std::map."""
    try:
        return int(val["_M_t"]["_M_impl"]["_M_node_count"])
    except:
        return 0

def std_map_items(val):
    """
    Iterate over key-value pairs in a std::map.
    Uses GDB's default visualizer (libstdc++ StdMapPrinter).
    Returns list of (key, value) tuples.

    Note: The libstdc++ StdMapPrinter yields *alternating* children:
      [0] -> first key,  [1] -> first value,
      [2] -> second key, [3] -> second value, ...
    This function pairs them up into (key, value) tuples.
    """
    try:
        items = []
        pp = gdb.default_visualizer(val)
        if pp and hasattr(pp, 'children'):
            children = list(pp.children())
            # Pair up alternating key/value entries
            for i in range(0, len(children) - 1, 2):
                key = children[i][1]        # the gdb.Value for the key
                value = children[i + 1][1]  # the gdb.Value for the value
                items.append((key, value))
        return items
    except:
        return []

def std_vector_int_items(val):
    """
    Extract all elements from a std::vector<int> as a Python list.
    Returns an empty list on failure.
    """
    try:
        size = std_vector_size(val)
        start = val["_M_impl"]["_M_start"]
        return [int(start[i]) for i in range(size)]
    except:
        return []

def std_stack_items(val):
    """
    Extract all elements from a std::stack as a Python list (bottom to top).
    std::stack wraps a deque in its 'c' member.  Uses GDB's pretty-printer
    for the underlying deque to iterate.
    """
    try:
        deque = val["c"]
        pp = gdb.default_visualizer(deque)
        if pp and hasattr(pp, 'children'):
            return [child[1] for child in pp.children()]
        return []
    except:
        return []

def identify_dynamic_type(val):
    """
    Identify the actual derived type of a polymorphic object by reading the vtable.
    Returns the demangled type name.
    """
    try:
        # Get the vtable pointer (first member of any polymorphic object)
        vtable_ptr = val.address.cast(gdb.lookup_type("void").pointer().pointer()).dereference()
        # Get the type info from the vtable (usually at offset -1)
        typeinfo = vtable_ptr.cast(gdb.lookup_type("void").pointer().pointer())[-1]
        # Try to get the name from RTTI
        return gdb.execute(f"info symbol {typeinfo}", to_string=True).split()[0]
    except:
        return "Unknown"

def call_method_safe(val, method_name, *args):
    """
    Safely call a method on a value in the inferior.
    Returns the result as a string, or None if the call fails.
    """
    try:
        arg_str = ", ".join(str(a) for a in args)
        expr = f"(({val.type.name}*){val.address}).{method_name}({arg_str})"
        result = gdb.parse_and_eval(expr)
        return result
    except:
        return None

def pyobj_repr(pyobj_ptr):
    """
    Given a GDB value that is a PyObject*, return a human-readable string
    representation of the Python object.  Handles int, float, str, bool,
    None, and tuple.  Falls back to showing the type name for anything else.
    """
    try:
        if int(pyobj_ptr) == 0:
            return "NULL"

        # Read ob_type->tp_name to discover the Python type
        ob_type = pyobj_ptr["ob_type"]
        tp_name = ob_type["tp_name"].string()

        if tp_name == "NoneType":
            return "None"

        if tp_name == "bool":
            # True/False are singletons; compare the pointer value to _Py_TrueStruct
            try:
                py_true = gdb.parse_and_eval("(PyObject*)&_Py_TrueStruct")
                if int(pyobj_ptr) == int(py_true):
                    return "True"
                return "False"
            except:
                return "<bool>"

        if tp_name == "float":
            try:
                float_type = gdb.lookup_type("PyFloatObject").pointer()
                fval = pyobj_ptr.cast(float_type)["ob_fval"]
                return str(float(fval))
            except:
                return "<float>"

        if tp_name == "int":
            try:
                long_type = gdb.lookup_type("PyLongObject").pointer()
                long_obj = pyobj_ptr.cast(long_type)
                lv_tag = int(long_obj["long_value"]["lv_tag"])
                # _PyLong_NON_SIZE_BITS = 3, _PyLong_SIGN_MASK = 3
                is_compact = lv_tag < (2 << 3)
                if is_compact:
                    sign = 1 - (lv_tag & 3)
                    digit = int(long_obj["long_value"]["ob_digit"][0])
                    return str(sign * digit)
                else:
                    return "<int (multi-digit)>"
            except:
                return "<int>"

        if tp_name == "str":
            try:
                ascii_type = gdb.lookup_type("PyASCIIObject").pointer()
                ascii_obj = pyobj_ptr.cast(ascii_type)
                length = int(ascii_obj["length"])
                kind = int(ascii_obj["state"]["kind"])
                is_ascii = int(ascii_obj["state"]["ascii"])
                is_compact = int(ascii_obj["state"]["compact"])
                if is_ascii and is_compact and kind == 1:
                    # Data follows immediately after the PyASCIIObject struct
                    data_ptr = (ascii_obj + 1).cast(gdb.lookup_type("char").pointer())
                    s = data_ptr.string(length=min(length, 80))
                    if length > 80:
                        return f'"{s}..."'
                    return f'"{s}"'
                elif is_compact and kind == 1:
                    # Latin-1, data after PyCompactUnicodeObject
                    compact_type = gdb.lookup_type("PyCompactUnicodeObject").pointer()
                    compact_obj = pyobj_ptr.cast(compact_type)
                    data_ptr = (compact_obj + 1).cast(gdb.lookup_type("char").pointer())
                    s = data_ptr.string(length=min(length, 80))
                    if length > 80:
                        return f'"{s}..."'
                    return f'"{s}"'
                else:
                    return f'<str (kind={kind}, len={length})>'
            except Exception as e:
                return f"<str: {e}>"

        if tp_name == "tuple":
            try:
                tuple_type = gdb.lookup_type("PyTupleObject").pointer()
                tup = pyobj_ptr.cast(tuple_type)
                # ob_size is in the PyVarObject header
                var_type = gdb.lookup_type("PyVarObject").pointer()
                size = int(pyobj_ptr.cast(var_type)["ob_size"])
                elems = []
                for i in range(min(size, 10)):
                    elem = tup["ob_item"][i]
                    elems.append(pyobj_repr(elem))
                inner = ", ".join(elems)
                if size > 10:
                    inner += ", ..."
                return f"({inner})"
            except Exception as e:
                return f"<tuple: {e}>"

        # Fallback: just show the type name
        return f"<{tp_name} object at {pyobj_ptr}>"
    except:
        return f"<PyObject at {pyobj_ptr}>"

def dump_pytuple(pyobj_ptr, arg_dict, indent="  "):
    """
    Given a GDB value that is a PyObject* pointing to a Python tuple, print
    its contents.  The tuple holds the argument values for a Python function
    call (populated by setArgValue, freed by ResetArgs).
    """
    if not isinstance(arg_dict,dict):
        arg_dict = dict()
    try:
        if int(pyobj_ptr) == 0:
            print(f"{indent}Tuple: nullptr (no call arguments prepared)")
            return

        var_type = gdb.lookup_type("PyVarObject").pointer()
        size = int(pyobj_ptr.cast(var_type)["ob_size"])
        tuple_type = gdb.lookup_type("PyTupleObject").pointer()
        tup = pyobj_ptr.cast(tuple_type)

        print(f"{indent}Tuple ({size} call args):")
        for i in range(size):
            elem = tup["ob_item"][i]
            if i in arg_dict.keys():
                print(f"{indent}  [{i}] {arg_dict[i]} = {pyobj_repr(elem)}")
            else:
                print(f"{indent}  [{i}] {pyobj_repr(elem)}")
    except Exception as e:
        print(f"{indent}Tuple: {pyobj_ptr} (cannot inspect: {e})")

# ============================================================================
# PRETTY-PRINTERS
# ============================================================================

class ALUValuePrinter:
    """Pretty-printer for ALUValue."""
    
    def __init__(self, val):
        self.val = val
    
    def to_string(self):
        type_val = int(self.val["m_type"])
        type_str = ALU_COUNTER_TYPE.get(type_val, f"Unknown({type_val})")
        value_str = std_string_to_str(self.val["m_value"])
        exact = bool(self.val["m_exact"])
        
        return f"ALUValue {{type: {type_str}, value: \"{value_str}\", exact: {exact}}}"

class TokenPrinter:
    """Pretty-printer for Token."""
    
    def __init__(self, val):
        self.val = val
    
    def to_string(self):
        type_val = int(self.val["m_type"])
        type_str = TOKEN_TYPES.get(type_val, f"Unknown({type_val})")
        literal = std_string_to_str(self.val["m_literal"])
        argc = int(self.val["m_argc"])
        
        return f"Token {{type: {type_str}, literal: \"{literal}\", argc: {argc}}}"

class ProcessingStatePrinter:
    """Abbreviated pretty-printer for ProcessingState."""
    
    def __init__(self, val):
        self.val = val
    
    def to_string(self):
        try:
            # Current record
            ps = deref_shared_ptr(self.val["m_ps"])
            if ps:
                record_str = std_string_to_str(ps)
            else:
                record_str = "<nullptr>"
            
            cycle = int(self.val["m_CycleCounter"])
            
            return f"ProcessingState {{record: \"{record_str[:30]}...\", cycle: {cycle}}}"
        except:
            return "ProcessingState {<error reading>}"

class SpecsExceptionPrinter:
    """Pretty-printer for SpecsException."""
    
    def __init__(self, val):
        self.val = val
    
    def to_string(self):
        fn = self.val["fn"]
        msg = std_string_to_str(self.val["msg"])
        ln = int(self.val["ln"])
        is_abend = bool(self.val["bIsAbend"])
        
        abend_str = " [ABEND]" if is_abend else ""
        return f"SpecsException {{{fn}:{ln}: {msg}{abend_str}}}"

def build_pretty_printer():
    """Build and register all pretty-printers."""
    pp = gdb.printing.RegexPrettyPrinter("specs")
    pp.add_printer("ALUValue", "^ALUValue$", ALUValuePrinter)
    pp.add_printer("Token", "^Token$", TokenPrinter)
    pp.add_printer("ProcessingState", "^ProcessingState$", ProcessingStatePrinter)
    pp.add_printer("SpecsException", "^SpecsException$", SpecsExceptionPrinter)
    return pp

# ============================================================================
# DUMP COMMANDS - InputPart Hierarchy
# ============================================================================

class DumpInputPart(gdb.Command):
    """Dump an InputPart (polymorphic)."""
    
    def __init__(self):
        super(DumpInputPart, self).__init__("dump-input-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            self._dump_input_part(val, 0)
        except Exception as e:
            print(f"Error: {e}")
    
    def _dump_input_part(self, val, indent=0):
        prefix = "  " * indent
        
        # Try to call Debug() to get the type-specific string
        try:
            debug_result = call_method_safe(val, "Debug")
            if debug_result:
                debug_str = str(debug_result)
            else:
                debug_str = "<unable to call Debug()>"
        except:
            debug_str = "<unable to call Debug()>"
        
        print(f"{prefix}InputPart @ {val.address}")
        print(f"{prefix}  Debug: {debug_str}")
        
        # Try to call virtual methods
        try:
            reads = call_method_safe(val, "readsLines")
            print(f"{prefix}  readsLines: {bool(reads)}")
        except:
            pass
        
        try:
            forces = call_method_safe(val, "forcesRunoutCycle")
            print(f"{prefix}  forcesRunoutCycle: {bool(forces)}")
        except:
            pass

class DumpLiteralPart(gdb.Command):
    """Dump a LiteralPart."""
    
    def __init__(self):
        super(DumpLiteralPart, self).__init__("dump-literal-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            m_str = std_string_to_str(val["m_Str"])
            print(f"LiteralPart @ {val.address}")
            print(f"  String: \"{m_str}\"")
        except Exception as e:
            print(f"Error: {e}")

class DumpRangePart(gdb.Command):
    """Dump a RangePart."""
    
    def __init__(self):
        super(DumpRangePart, self).__init__("dump-range-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            from_val = int(val["_from"])
            to_val = int(val["_to"])
            print(f"RangePart @ {val.address}")
            print(f"  From: {from_val}")
            print(f"  To: {to_val}")
            print(f"  readsLines: true")
        except Exception as e:
            print(f"Error: {e}")

class DumpWordRangePart(gdb.Command):
    """Dump a WordRangePart."""
    
    def __init__(self):
        super(DumpWordRangePart, self).__init__("dump-word-range-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            from_val = int(val["_from"])
            to_val = int(val["_to"])
            sep = std_string_to_str(val["m_WordSep"])
            print(f"WordRangePart @ {val.address}")
            print(f"  From: {from_val}")
            print(f"  To: {to_val}")
            print(f"  Word Separator: \"{sep}\"")
        except Exception as e:
            print(f"Error: {e}")

class DumpFieldRangePart(gdb.Command):
    """Dump a FieldRangePart."""
    
    def __init__(self):
        super(DumpFieldRangePart, self).__init__("dump-field-range-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            from_val = int(val["_from"])
            to_val = int(val["_to"])
            sep = std_string_to_str(val["m_FieldSep"])
            print(f"FieldRangePart @ {val.address}")
            print(f"  From: {from_val}")
            print(f"  To: {to_val}")
            print(f"  Field Separator: \"{sep}\"")
        except Exception as e:
            print(f"Error: {e}")

class DumpClockPart(gdb.Command):
    """Dump a ClockPart."""
    
    def __init__(self):
        super(DumpClockPart, self).__init__("dump-clock-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            type_val = int(val["m_Type"])
            type_str = CLOCK_TYPE.get(type_val, f"Unknown({type_val})")
            clock = int(val["m_StaticClock"])
            print(f"ClockPart @ {val.address}")
            print(f"  Type: {type_str}")
            print(f"  Static Clock: {clock}")
        except Exception as e:
            print(f"Error: {e}")

class DumpIDPart(gdb.Command):
    """Dump an IDPart."""
    
    def __init__(self):
        super(DumpIDPart, self).__init__("dump-id-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            fid = std_string_to_str(val["m_fieldIdentifier"])
            print(f"IDPart @ {val.address}")
            print(f"  Field Identifier: \"{fid}\"")
        except Exception as e:
            print(f"Error: {e}")

class DumpExpressionPart(gdb.Command):
    """Dump an ExpressionPart."""
    
    def __init__(self):
        super(DumpExpressionPart, self).__init__("dump-expression-part", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            raw_expr = std_string_to_str(val["m_rawExpression"])
            is_assn = bool(val["m_isAssignment"])
            print(f"ExpressionPart @ {val.address}")
            print(f"  Expression: \"{raw_expr}\"")
            print(f"  Is Assignment: {is_assn}")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - Item Hierarchy
# ============================================================================

class DumpItem(gdb.Command):
    """Dump an Item (polymorphic)."""
    
    def __init__(self):
        super(DumpItem, self).__init__("dump-item", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            orig_idx = int(val["m_originalIndex"])
            print(f"Item @ {val.address}")
            print(f"  Original Index: {orig_idx}")
            
            # Try to call virtual methods
            try:
                debug_result = call_method_safe(val, "Debug")
                if debug_result:
                    print(f"  Debug: {debug_result}")
            except:
                pass
            
            try:
                reads = call_method_safe(val, "readsLines")
                print(f"  readsLines: {bool(reads)}")
            except:
                pass
            
            try:
                produces = call_method_safe(val, "producesOutput")
                print(f"  producesOutput: {bool(produces)}")
            except:
                pass
            
            try:
                forces = call_method_safe(val, "forcesRunoutCycle")
                print(f"  forcesRunoutCycle: {bool(forces)}")
            except:
                pass
            
            try:
                is_break = call_method_safe(val, "isBreak")
                print(f"  isBreak: {bool(is_break)}")
            except:
                pass
            
            print("----- end of 'Item' dump")
        except Exception as e:
            print(f"Error: {e}")

class DumpDataField(gdb.Command):
    """Dump a DataField."""
    
    def __init__(self):
        super(DumpDataField, self).__init__("dump-data-field", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"DataField:")
            val = gdb.parse_and_eval(arg)
            label = chr(int(val["m_label"])) if int(val["m_label"]) > 0 else "none"
            out_start = int(val["m_outStart"])
            max_len = int(val["m_maxLength"])
            strip = bool(val["m_strip"])
            conv = int(val["m_conversion"])
            align = int(val["m_alignment"])
            
            conv_str = STRING_CONVERSIONS.get(conv, f"Unknown({conv})")
            align_str = OUTPUT_ALIGNMENT.get(align, f"Unknown({align})")
            
            print(f"  Label: {label}")
            print(f"  Output Start: {out_start}")
            print(f"  Max Length: {max_len}")
            print(f"  Strip: {strip}")
            print(f"  Conversion: {conv_str}")
            print(f"  Alignment: {align_str}")
        except Exception as e:
            print(f"Error: {e}")

class DumpTokenItem(gdb.Command):
    """Dump a TokenItem."""
    
    def __init__(self):
        super(DumpTokenItem, self).__init__("dump-token-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"TokenItem:")
            val = gdb.parse_and_eval(arg)
            token = deref_shared_ptr(val["mp_Token"])
            if token:
                type_val = int(token["m_type"])
                type_str = TOKEN_TYPES.get(type_val, f"Unknown({type_val})")
                print(f"  Token type: {type_str}")
            else:
                print(f"  mp_Token: <nullptr>")
        except Exception as e:
            print(f"Error: {e}")

class DumpSetItem(gdb.Command):
    """Dump a SetItem."""
    
    def __init__(self):
        super(DumpSetItem, self).__init__("dump-set-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"SetItem:")
            val = gdb.parse_and_eval(arg)
            raw_expr = std_string_to_str(val["m_rawExpression"])
            key = int(val["m_key"])
            print(f"  Expression: \"{raw_expr}\"")
            print(f"  Key: {key}")
        except Exception as e:
            print(f"Error: {e}")

class DumpSkipItem(gdb.Command):
    """Dump a SkipItem."""
    
    def __init__(self):
        super(DumpSkipItem, self).__init__("dump-skip-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"SkipItem:")
            val = gdb.parse_and_eval(arg)
            raw_expr = std_string_to_str(val["m_rawExpression"])
            is_until = bool(val["m_bIsUntil"])
            satisfied = bool(val["m_bSatisfied"])
            skip_type = "SKIPUNTIL" if is_until else "SKIPWHILE"
            print(f"  Type: {skip_type}")
            print(f"  Expression: \"{raw_expr}\"")
            print(f"  Satisfied: {satisfied}")
        except Exception as e:
            print(f"Error: {e}")

class DumpConditionItem(gdb.Command):
    """Dump a ConditionItem."""
    
    def __init__(self):
        super(DumpConditionItem, self).__init__("dump-condition-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"ConditionItem:")
            val = gdb.parse_and_eval(arg)
            pred = int(val["m_pred"])
            pred_str = CONDITION_PREDICATE.get(pred, f"Unknown({pred})")
            raw_expr = std_string_to_str(val["m_rawExpression"])
            is_assn = bool(val["m_isAssignment"])
            print(f"  Predicate: {pred_str}")
            print(f"  Expression: \"{raw_expr}\"")
            print(f"  Is Assignment: {is_assn}")
        except Exception as e:
            print(f"Error: {e}")

class DumpBreakItem(gdb.Command):
    """Dump a BreakItem."""
    
    def __init__(self):
        super(DumpBreakItem, self).__init__("dump-break-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"BreakItem:")
            val = gdb.parse_and_eval(arg)
            ident = chr(int(val["m_identifier"]))
            print(f"  Identifier: {ident}")
        except Exception as e:
            print(f"Error: {e}")

class DumpContextItem(gdb.Command):
    """Dump a ContextItem."""
    
    def __init__(self):
        super(DumpContextItem, self).__init__("dump-context-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"ContextItem:")
            val = gdb.parse_and_eval(arg)
            offset = int(val["m_offset"])
            print(f"  Offset: {offset}")
        except Exception as e:
            print(f"Error: {e}")

class DumpSelectItem(gdb.Command):
    """Dump a SelectItem."""
    
    def __init__(self):
        super(DumpSelectItem, self).__init__("dump-select-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"SelectItem:")
            val = gdb.parse_and_eval(arg)
            stream = int(val["m_stream"])
            b_output = bool(val["bOutput"])
            print(f"  Stream: {stream}")
            print(f"  Output: {b_output}")
        except Exception as e:
            print(f"Error: {e}")

class DumpSplitItem(gdb.Command):
    """Dump a SplitItem."""
    
    def __init__(self):
        super(DumpSplitItem, self).__init__("dump-split-item", gdb.COMMAND_DATA)
        self.dump_item = None
    
    def invoke(self, arg, from_tty):
        try:
            # First, call DumpItem to print base class fields
            if self.dump_item is None:
                self.dump_item = DumpItem()
            self.dump_item.invoke(arg, from_tty)
            
            # Then print derived class fields
            print(f"SplitItem:")
            val = gdb.parse_and_eval(arg)
            is_field = bool(val["m_isField"])
            sep = std_string_to_str(val["m_separator"])
            splitting = bool(val["m_splitting"])
            current_piece = int(val["m_currentPiece"])
            split_type = "SPLITF" if is_field else "SPLITW"
            print(f"  Type: {split_type}")
            print(f"  Separator: \"{sep}\"")
            print(f"  Splitting: {splitting}")
            print(f"  Current Piece: {current_piece}")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - itemGroup
# ============================================================================

def _dump_item_detail(ptr_val, index, indent="    "):
    """
    Given an Item* (as a gdb.Value integer pointer), print a detailed
    one-line summary including the dynamic type name and Debug() output.
    """
    if int(ptr_val) == 0:
        print(f"{indent}[{index}] <nullptr>")
        return

    # Determine the dynamic type name via RTTI
    type_name = "Item"
    try:
        item_obj = ptr_val.dereference()
        dyn_type = item_obj.dynamic_type
        type_name = dyn_type.name
    except:
        pass

    # Call the virtual Debug() method for a type-specific description
    debug_str = None
    try:
        result = gdb.parse_and_eval(
            f'((Item*)({int(ptr_val)}))->Debug()')
        debug_str = std_string_to_str(result)
    except:
        pass

    if debug_str:
        print(f"{indent}[{index}] ({type_name}) {debug_str}")
    else:
        print(f"{indent}[{index}] ({type_name}) @ {ptr_val}")


class DumpItemGroup(gdb.Command):
    """Dump an itemGroup."""
    
    def __init__(self):
        super(DumpItemGroup, self).__init__("dump-item-group", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            need_runout = bool(val["bNeedRunoutCycle"])
            need_runout_from_start = bool(val["bNeedRunoutCycleFromStart"])
            found_second = bool(val["bFoundSelectSecond"])
            
            # Get m_items vector
            items_vec = val["m_items"]
            item_count = std_vector_size(items_vec)
            
            print(f"itemGroup @ {val.address}")
            print(f"  Need Runout Cycle: {need_runout}")
            print(f"     From start:     {need_runout_from_start}")
            print(f"  Found Select Second: {found_second}")
            print(f"  Item count: {item_count}")
            print(f"  Items:")
            
            max_display = 50
            for i in range(min(item_count, max_display)):
                try:
                    shared_ptr = items_vec["_M_impl"]["_M_start"][i]
                    ptr = shared_ptr["_M_ptr"]
                    _dump_item_detail(ptr, i)
                except Exception:
                    print(f"    [{i}] <error reading item>")
            
            if item_count > max_display:
                print(f"    ... and {item_count - max_display} more items")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - Token System
# ============================================================================

class DumpToken(gdb.Command):
    """Dump a Token."""
    
    def __init__(self):
        super(DumpToken, self).__init__("dump-token", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            type_val = int(val["m_type"])
            type_str = TOKEN_TYPES.get(type_val, f"Unknown({type_val})")
            literal = std_string_to_str(val["m_literal"])
            argc = int(val["m_argc"])
            orig = std_string_to_str(val["m_orig"])
            
            print(f"Token @ {val.address}")
            print(f"  Type: {type_str}")
            print(f"  Literal: \"{literal}\"")
            print(f"  Arg Count: {argc}")
            print(f"  Original: \"{orig}\"")
        except Exception as e:
            print(f"Error: {e}")

class DumpTokenRange(gdb.Command):
    """Dump a TokenFieldRange."""
    
    def __init__(self):
        super(DumpTokenRange, self).__init__("dump-token-range", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            b_done = bool(val["bDone"])
            
            # Try to access Simple range members
            try:
                first = int(val["m_first"])
                last = int(val["m_last"])
                print(f"TokenFieldRangeSimple @ {val.address}")
                print(f"  First: {first}")
                print(f"  Last: {last}")
                print(f"  Done: {b_done}")
            except:
                print(f"TokenFieldRange @ {val.address}")
                print(f"  Done: {b_done}")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - Processing
# ============================================================================

class DumpProcessingState(gdb.Command):
    """Dump a ProcessingState."""
    
    def __init__(self):
        super(DumpProcessingState, self).__init__("dump-processing-state", gdb.COMMAND_DATA)
    
    @staticmethod
    def _fmt_record(shared_str):
        """Format a PSpecString (shared_ptr<string>) for display."""
        obj = deref_shared_ptr(shared_str)
        if obj is None:
            return "<nullptr>"
        s = std_string_to_str(obj)
        if len(s) > 60:
            return f"\"{s[:60]}...\" (len={len(s)})"
        return f"\"{s}\""
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            print(f"ProcessingState @ {val.address}")
            
            # --- Records ---
            print(f"  Current Record:    {self._fmt_record(val['m_ps'])}   (CONTEXT-dependent)")
            print(f"  Previous Record:   {self._fmt_record(val['m_prevPs'])}")
            print(f"  Input Record:      {self._fmt_record(val['m_inputRecord'])}   (CONTEXT-independent)")
            context_offset = int(val["m_contextOffset"])
            print(f"  Context Offset:    {context_offset}")
            
            # --- Separators & Padding ---
            try:
                pad = chr(int(val["m_pad"]))
                print(f"  Pad Char:          '{pad}' (0x{ord(pad):02x})")
            except Exception as e:
                print(f"  Pad Char:          (error: {e})")
            
            try:
                ws_local = bool(val["m_wordSeparatorLocal"])
                word_sep = std_string_to_str(val["m_wordSeparator"])
                local_tag = " (local)" if ws_local else ""
                print(f"  Word Separator:    \"{word_sep}\"{local_tag}")
            except Exception as e:
                print(f"  Word Separator:    (error: {e})")
            
            try:
                field_sep = std_string_to_str(val["m_fieldSeparator"])
                print(f"  Field Separator:   \"{field_sep}\"")
            except Exception as e:
                print(f"  Field Separator:   (error: {e})")
            
            # --- Counters ---
            try:
                cycle = int(val["m_CycleCounter"])
                extra_reads = int(val["m_ExtraReads"])
                print(f"  Cycle Counter:     {cycle}")
                print(f"  Extra Reads:       {extra_reads}")
                print(f"  Record Count:      {cycle + extra_reads}")
            except Exception as e:
                print(f"  Counters:          (error: {e})")
            
            # --- Word / Field Caches ---
            try:
                word_count = int(val["m_wordCount"])
                field_count = int(val["m_fieldCount"])
                print(f"  Word Count:        {word_count}")
                print(f"  Field Count:       {field_count}")
            except Exception as e:
                print(f"  Word/Field Count:  (error: {e})")
            
            try:
                ws = std_vector_int_items(val["m_wordStart"])
                we = std_vector_int_items(val["m_wordEnd"])
                n = len(ws)
                print(f"  Word Positions ({n} cached):")
                if n == 0:
                    print(f"    (none)")
                else:
                    limit = min(n, 20)
                    for i in range(limit):
                        end_val = we[i] if i < len(we) else "?"
                        print(f"    [{i}] {ws[i]}-{end_val}")
                    if n > 20:
                        print(f"    ... and {n - 20} more")
            except Exception as e:
                print(f"  Word Positions:    (error: {e})")
            
            try:
                fs = std_vector_int_items(val["m_fieldStart"])
                fe = std_vector_int_items(val["m_fieldEnd"])
                n = len(fs)
                print(f"  Field Positions ({n} cached):")
                if n == 0:
                    print(f"    (none)")
                else:
                    limit = min(n, 20)
                    for i in range(limit):
                        end_val = fe[i] if i < len(fe) else "?"
                        print(f"    [{i}] {fs[i]}-{end_val}")
                    if n > 20:
                        print(f"    ... and {n - 20} more")
            except Exception as e:
                print(f"  Field Positions:   (error: {e})")
            
            # --- Field Identifiers ---
            try:
                fi = val["m_fieldIdentifiers"]
                fi_size = std_map_size(fi)
                print(f"  Field Identifiers ({fi_size} entries):")
                if fi_size == 0:
                    print(f"    (none)")
                else:
                    items = std_map_items(fi)
                    for key, value in items:
                        try:
                            k = chr(int(key))
                            s = deref_shared_ptr(value)
                            v = std_string_to_str(s) if s else "<nullptr>"
                            if len(v) > 40:
                                v = v[:40] + "..."
                            print(f"    '{k}' = \"{v}\"")
                        except:
                            pass
            except Exception as e:
                print(f"  Field Identifiers: (error: {e})")
            
            # --- FI Statistics ---
            try:
                fis = val["m_fiStatistics"]
                fis_size = std_map_size(fis)
                print(f"  FI Statistics ({fis_size} entries):")
                if fis_size == 0:
                    print(f"    (none)")
                else:
                    items = std_map_items(fis)
                    for key, value in items:
                        try:
                            k = chr(int(key))
                            stats = deref_shared_ptr(value)
                            if stats:
                                total = int(stats["m_totalCount"])
                                int_count = int(stats["m_intCount"])
                                float_count = int(stats["m_floatCount"])
                                print(f"    '{k}': {total} values ({int_count} int, {float_count} float)")
                            else:
                                print(f"    '{k}': <nullptr>")
                        except:
                            pass
            except Exception as e:
                print(f"  FI Statistics:     (error: {e})")
            
            # --- Break Values ---
            try:
                bv = val["m_breakValues"]
                bv_size = std_map_size(bv)
                print(f"  Break Values ({bv_size} entries):")
                if bv_size == 0:
                    print(f"    (none)")
                else:
                    items = std_map_items(bv)
                    for key, value in items:
                        try:
                            k = chr(int(key))
                            s = deref_shared_ptr(value)
                            v = std_string_to_str(s) if s else "<nullptr>"
                            if len(v) > 40:
                                v = v[:40] + "..."
                            print(f"    '{k}' = \"{v}\"")
                        except:
                            pass
            except Exception as e:
                print(f"  Break Values:      (error: {e})")
            
            try:
                bl = int(val["m_breakLevel"])
                if bl == 0:
                    print(f"  Break Level:       (none)")
                else:
                    print(f"  Break Level:       '{chr(bl)}' (0x{bl:02x})")
            except Exception as e:
                print(f"  Break Level:       (error: {e})")
            
            # --- Frequency Maps ---
            try:
                fm = val["m_freqMaps"]
                fm_size = std_map_size(fm)
                print(f"  Frequency Maps ({fm_size} entries):")
                if fm_size == 0:
                    print(f"    (none)")
                else:
                    items = std_map_items(fm)
                    for key, value in items:
                        try:
                            k = chr(int(key))
                            fmap = deref_shared_ptr(value)
                            if fmap:
                                nelem = int(fmap["map"]["_M_element_count"])
                                counter = int(fmap["counter"])
                                print(f"    '{k}': {nelem} unique elements, {counter} total")
                            else:
                                print(f"    '{k}': <nullptr>")
                        except:
                            pass
            except Exception as e:
                print(f"  Frequency Maps:    (error: {e})")
            
            # --- Conditions Stack ---
            try:
                cond_items = std_stack_items(val["m_Conditions"])
                depth = len(cond_items)
                print(f"  Conditions ({depth} deep):")
                if depth == 0:
                    print(f"    (empty)")
                else:
                    for i in range(depth - 1, -1, -1):
                        label = "top -> " if i == depth - 1 else "       "
                        v = int(cond_items[i])
                        name = EXTREME_BOOL.get(v, f"Unknown({v})")
                        print(f"    {label}{name}")
            except Exception as e:
                print(f"  Conditions:        (error: {e})")
            
            # --- Loops Stack ---
            try:
                loop_items = std_stack_items(val["m_Loops"])
                depth = len(loop_items)
                print(f"  Loops ({depth} deep):")
                if depth == 0:
                    print(f"    (empty)")
                else:
                    for i in range(depth - 1, -1, -1):
                        label = "top -> " if i == depth - 1 else "       "
                        v = int(loop_items[i])
                        print(f"    {label}token #{v}")
            except Exception as e:
                print(f"  Loops:             (error: {e})")
            
            # --- I/O State ---
            try:
                input_station = int(val["m_inputStation"])
                station_name = INPUT_STATION.get(input_station, f"Stream({input_station})")
                print(f"  Input Station:     {station_name}")
            except Exception as e:
                print(f"  Input Station:     (error: {e})")
            
            try:
                input_stream = int(val["m_inputStream"])
                print(f"  Input Stream:      {input_stream}")
            except Exception as e:
                print(f"  Input Stream:      (error: {e})")
            
            try:
                stream_changed = bool(val["m_inputStreamChanged"])
                print(f"  Stream Changed:    {stream_changed}")
            except Exception as e:
                print(f"  Stream Changed:    (error: {e})")
            
            try:
                writers = val["m_Writers"]
                print(f"  Writers:           {writers}")
            except Exception as e:
                print(f"  Writers:           (error: {e})")
            
            try:
                output_idx = int(val["m_outputIndex"])
                print(f"  Output Index:      {output_idx}")
            except Exception as e:
                print(f"  Output Index:      (error: {e})")
            
            try:
                no_write = bool(val["m_bNoWrite"])
                print(f"  No Write:          {no_write}")
            except Exception as e:
                print(f"  No Write:          (error: {e})")
            
            try:
                eof = bool(val["m_bEOF"])
                print(f"  EOF:               {eof}")
            except Exception as e:
                print(f"  EOF:               (error: {e})")
        except Exception as e:
            print(f"Error: {e}")

class DumpStringBuilder(gdb.Command):
    """Dump a StringBuilder."""
    
    def __init__(self):
        super(DumpStringBuilder, self).__init__("dump-string-builder", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            # Current string
            mp_str = deref_shared_ptr(val["mp_str"])
            if mp_str:
                str_content = std_string_to_str(mp_str)
            else:
                str_content = "<nullptr>"
            
            pos = int(val["m_pos"])
            pad = chr(int(val["m_pad"]))
            
            print(f"StringBuilder @ {val.address}")
            print(f"  Current String: \"{str_content[:50]}{'...' if len(str_content) > 50 else ''}\"")
            print(f"  Position:       {pos}")
            print(f"  Pad Char:       '{pad}' (0x{ord(pad):02x})")
        except Exception as e:
            print(f"Error: {e}")

class DumpReader(gdb.Command):
    """Dump a Reader."""
    
    def __init__(self):
        super(DumpReader, self).__init__("dump-reader", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            count_read = int(val["m_countRead"])
            count_used = int(val["m_countUsed"])
            b_abort = bool(val["m_bAbort"])
            b_ran_dry = bool(val["m_bRanDry"])
            
            print(f"Reader @ {val.address}")
            print(f"  Count Read:  {count_read}")
            print(f"  Count Used:  {count_used}")
            print(f"  Abort:       {b_abort}")
            print(f"  Ran Dry:     {b_ran_dry}")
        except Exception as e:
            print(f"Error: {e}")

class DumpWriter(gdb.Command):
    """Dump a Writer."""
    
    def __init__(self):
        super(DumpWriter, self).__init__("dump-writer", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            count_gen = int(val["m_countGenerated"])
            count_written = int(val["m_countWritten"])
            ended = bool(val["m_ended"])
            
            print(f"Writer @ {val.address}")
            print(f"  Count Generated: {count_gen}")
            print(f"  Count Written:   {count_written}")
            print(f"  Ended:           {ended}")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - ALU
# ============================================================================

class DumpALUValue(gdb.Command):
    """Dump an ALUValue."""
    
    def __init__(self):
        super(DumpALUValue, self).__init__("dump-alu-value", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            type_val = int(val["m_type"])
            type_str = ALU_COUNTER_TYPE.get(type_val, f"Unknown({type_val})")
            value_str = std_string_to_str(val["m_value"])
            exact = bool(val["m_exact"])
            
            print(f"ALUValue @ {val.address}")
            print(f"  Type:  {type_str}")
            print(f"  Value: \"{value_str}\"")
            print(f"  Exact: {exact}")
        except Exception as e:
            print(f"Error: {e}")

class DumpALUCounters(gdb.Command):
    """Dump ALUCounters."""
    
    def __init__(self):
        super(DumpALUCounters, self).__init__("dump-alu-counters", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            m_map = val["m_map"]
            size = std_map_size(m_map)
            print(f"ALUCounters @ {val.address}")
            print(f"  Counters ({size} entries):")
            if size == 0:
                print(f"    (empty)")
            else:
                items = std_map_items(m_map)
                for key, value in items:
                    try:
                        k = int(key)
                        type_val = int(value["m_type"])
                        type_str = ALU_COUNTER_TYPE.get(type_val, f"Unknown({type_val})").lower()
                        val_str = std_string_to_str(value["m_value"])
                        exact = bool(value["m_exact"])
                        exact_str = " (exact)" if exact else ""
                        if type_val == 0:  # counterType__None
                            print(f"    #{k}: (none)")
                        else:
                            print(f"    #{k}: ({type_str}) {val_str}{exact_str}")
                    except Exception as item_e:
                        print(f"    (error: {item_e})")
        except Exception as e:
            print(f"Error: {e}")

class DumpAluUnit(gdb.Command):
    """Dump an AluUnit (polymorphic)."""
    
    def __init__(self):
        super(DumpAluUnit, self).__init__("dump-alu-unit", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            # Try to call virtual methods
            try:
                identify = call_method_safe(val, "_identify")
                if identify:
                    print(f"AluUnit @ {val.address}")
                    print(f"  _identify: {identify}")
            except:
                print(f"AluUnit @ {val.address}")
        except Exception as e:
            print(f"Error: {e}")

class DumpAluVec(gdb.Command):
    """Dump an AluVec (vector<PUnit>)."""
    
    def __init__(self):
        super(DumpAluVec, self).__init__("dump-alu-vec", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            size = std_vector_size(val)
            
            print(f"AluVec @ {val.address}")
            print(f"  Size: {size}")
            print(f"  Units:")
            
            for i in range(min(size, 10)):
                try:
                    unit = val["_M_impl"]["_M_start"][i]
                    print(f"    [{i}] @ {unit.address}")
                except:
                    pass
            
            if size > 10:
                print(f"    ... and {size - 10} more units")
        except Exception as e:
            print(f"Error: {e}")

class DumpAluValueStats(gdb.Command):
    """Dump AluValueStats."""
    
    def __init__(self):
        super(DumpAluValueStats, self).__init__("dump-alu-value-stats", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            int_count = int(val["m_intCount"])
            float_count = int(val["m_floatCount"])
            total_count = int(val["m_totalCount"])
            
            print(f"AluValueStats @ {val.address}")
            print(f"  Int Count:   {int_count}")
            print(f"  Float Count: {float_count}")
            print(f"  Total Count: {total_count}")
        except Exception as e:
            print(f"Error: {e}")

class DumpFrequencyMap(gdb.Command):
    """Dump a frequencyMap."""
    
    def __init__(self):
        super(DumpFrequencyMap, self).__init__("dump-frequency-map", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            counter = int(val["counter"])
            
            print(f"frequencyMap @ {val.address}")
            print(f"  counter (samples): {counter}")
            print(f"  map @ {val['map'].address}")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - Utilities
# ============================================================================

class DumpException(gdb.Command):
    """Dump a SpecsException."""
    
    def __init__(self):
        super(DumpException, self).__init__("dump-exception", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            fn = str(val["fn"])
            msg = std_string_to_str(val["msg"])
            ln = int(val["ln"])
            is_abend = bool(val["bIsAbend"])
            
            abend_str = " [ABEND]" if is_abend else ""
            print(f"SpecsException @ {val.address}")
            print(f"  File:    {fn}")
            print(f"  Line:    {ln}")
            print(f"  Message: {msg}{abend_str}")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# DUMP COMMANDS - Python Interface
# ============================================================================

class DumpAluFunction(gdb.Command):
    """Dump an AluFunction."""
    
    def __init__(self):
        super(DumpAluFunction, self).__init__("dump-alu-function", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            func_name = std_string_to_str(val["m_FuncName"])
            arg_count = int(val["m_ArgCount"])
            relies_on_input = bool(val["m_reliesOnInput"])
            
            print(f"AluFunction @ {val.address}")
            print(f"  Function Name: {func_name}")
            print(f"  Arg Count: {arg_count}")
            print(f"  Relies On Input: {relies_on_input}")
        except Exception as e:
            print(f"Error: {e}")

class DumpExternalFunctionRec(gdb.Command):
    """Dump an ExternalFunctionRec (polymorphic) - calls virtual methods."""
    
    def __init__(self):
        super(DumpExternalFunctionRec, self).__init__("dump-external-function-rec", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            print(f"ExternalFunctionRec @ {val.address}")
            
            # Try to call virtual methods to get information
            try:
                arg_count = call_method_safe(val, "GetArgCount")
                print(f"  GetArgCount(): {arg_count}")
            except Exception as e:
                print(f"  GetArgCount(): (error: {e})")
            
            try:
                func_ptr = call_method_safe(val, "GetFuncPtr")
                print(f"  GetFuncPtr(): {func_ptr}")
            except Exception as e:
                print(f"  GetFuncPtr(): (error: {e})")
            
            # Try to detect actual derived type
            try:
                actual_type = identify_dynamic_type(val)
                if actual_type and actual_type != "ExternalFunctionRec":
                    print(f"  Actual type: {actual_type}")
            except:
                pass
        except Exception as e:
            print(f"Error: {e}")

class DumpExternalFunctionCollection(gdb.Command):
    """Dump an ExternalFunctionCollection."""
    
    def __init__(self):
        super(DumpExternalFunctionCollection, self).__init__("dump-external-function-collection", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            # Try to call virtual methods
            try:
                is_init = call_method_safe(val, "IsInitialized")
                print(f"ExternalFunctionCollection @ {val.address}")
                print(f"  IsInitialized: {bool(is_init)}")
            except:
                try:
                    count = call_method_safe(val, "CountFunctions")
                    print(f"ExternalFunctionCollection @ {val.address}")
                    print(f"  CountFunctions: {count}")
                except:
                    print(f"ExternalFunctionCollection @ {val.address}")
        except Exception as e:
            print(f"Error: {e}")

class DumpPythonFunctionCollection(gdb.Command):
    """Dump a PythonFunctionCollection (internal class from PythonIntf.cc)."""
    
    def __init__(self):
        super(DumpPythonFunctionCollection, self).__init__("dump-python-function-collection", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            print(f"PythonFunctionCollection @ {val.address}")
            
            # Access m_Initialized
            try:
                m_initialized = bool(val["m_Initialized"])
                print(f"  Initialized: {m_initialized}")
            except Exception as e:
                print(f"  Initialized: (error: {e})")
            
            # Try to access m_Functions map and iterate
            try:
                m_functions = val["m_Functions"]
                map_size = std_map_size(m_functions)
                print(f"  Functions ({map_size} entries):")
                
                if map_size == 0:
                    print(f"    (empty)")
                else:
                    # Iterate the map using std_map_items
                    items = std_map_items(m_functions)
                    if items:
                        for key, value in items:
                            try:
                                # value is shared_ptr<PythonFuncRec>
                                func_rec = deref_shared_ptr(value)
                                if func_rec:
                                    func_name = std_string_to_str(func_rec["m_name"]).ljust(20)
                                    func_ptr = func_rec["m_pFuncPtr"]
                                    arg_type_exact = bool(func_rec["m_argTypeExact"])
                                    arg_count = std_vector_size(func_rec["m_args"])
                                    
                                    exact_str = ", exact arguments" if arg_type_exact else ""
                                    print(f"    {func_name} @ {func_ptr} ({arg_count} args{exact_str})")
                            except:
                                pass
                    else:
                        print(f"    (iteration failed)")
            except Exception as e:
                print(f"  Functions: (error: {e})")
        except Exception as e:
            print(f"Error: {e}")

class DumpPythonFuncByName(gdb.Command):
    """Dump a PythonFuncRec by looking it up in a PythonFunctionCollection by name."""
    
    def __init__(self):
        super(DumpPythonFuncByName, self).__init__("dump-python-func-by-name", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            # Parse arguments: collection_expr function_name
            args = arg.split(None, 1)
            if len(args) < 2:
                print("Usage: dump-python-func-by-name <collection> <function_name>")
                return
            
            collection_expr = args[0]
            func_name_arg = args[1]
            
            # Remove quotes if present
            if func_name_arg.startswith('"') and func_name_arg.endswith('"'):
                func_name_arg = func_name_arg[1:-1]
            elif func_name_arg.startswith("'") and func_name_arg.endswith("'"):
                func_name_arg = func_name_arg[1:-1]
            
            # Evaluate the collection
            collection = gdb.parse_and_eval(collection_expr)
            
            # Access the m_Functions map and find the function by name
            m_functions = collection["m_Functions"]
            items = std_map_items(m_functions)
            
            found = False
            if items:
                for key, value in items:
                    try:
                        key_str = std_string_to_str(key)
                        if key_str == func_name_arg:
                            func_rec = deref_shared_ptr(value)
                            if func_rec:
                                self._dump_python_func_rec(func_rec)
                                found = True
                                break
                    except:
                        pass
            
            if not found:
                print(f"Function '{func_name_arg}' not found in collection")
        except Exception as e:
            print(f"Error: {e}")
    
    def _dump_python_func_rec(self, val):
        """Dump a PythonFuncRec (reuses logic from DumpPythonFuncRec)."""
        try:
            print(f"PythonFuncRec @ {val.address}")
            
            # Access members
            try:
                m_name = std_string_to_str(val["m_name"])
                print(f"  Name: {m_name}")
            except Exception as e:
                print(f"  Name: (error: {e})")
            
            try:
                m_pFuncPtr = val["m_pFuncPtr"]
                print(f"  Func Ptr: {m_pFuncPtr}")
            except Exception as e:
                print(f"  Func Ptr: (error: {e})")
            
            # Show m_doc
            try:
                m_doc = std_string_to_str(val["m_doc"])
                if m_doc:
                    # Format multi-line docs nicely
                    if "\n" in m_doc:
                        print(f"  doc:")
                        for line in m_doc.split("\n"):
                            print(f"    {line}")
                    else:
                        print(f"  doc: {m_doc}")
                else:
                    print(f"  doc: (empty)")
            except Exception as e:
                print(f"  doc: (error: {e})")
            
            # Show m_argTypeExact
            try:
                m_argTypeExact = bool(val["m_argTypeExact"])
                print("  Arg Type: {}".format("exact" if m_argTypeExact else "no exactness information"))
            except Exception as e:
                print(f"  Arg Type: (error: {e})")
            
            # Expand m_args vector
            argDict = dict()
            try:
                m_args = val["m_args"]
                arg_size = std_vector_size(m_args)
                print(f"  Args ({arg_size} items):")
                
                # Try to iterate and dump each argument
                args_start = m_args["_M_impl"]["_M_start"]
                for i in range(arg_size):
                    try:
                        arg_elem = args_start[i]
                        arg_name = std_string_to_str(arg_elem["m_name"])
                        arg_default = int(arg_elem["m_default"])
                        arg_default_str = ALU_COUNTER_TYPE.get(arg_default, f"Unknown({arg_default})")
                        
                        print(f"    [{i}] {arg_name} (default: {arg_default_str})")
                        argDict[i] = arg_name
                        
                        # Show default value if present
                        if arg_default == 1:  # counterType__Str
                            try:
                                defStr = std_string_to_str(arg_elem["m_defStr"])
                                print(f"         = \"{defStr}\"")
                            except:
                                pass
                        elif arg_default == 2:  # counterType__Int
                            try:
                                defInt = int(arg_elem["m_defInt"])
                                print(f"         = {defInt}")
                            except:
                                pass
                        elif arg_default == 3:  # counterType__Float
                            try:
                                defFloat = float(arg_elem["m_defFloat"])
                                print(f"         = {defFloat}")
                            except:
                                pass
                    except Exception as arg_e:
                        print(f"    [{i}] (error: {arg_e})")
            except Exception as e:
                print(f"  m_args: (error: {e})")

            # Show m_pTuple (argument values for the current Python function call)
            try:
                m_pTuple = val["m_pTuple"]
                dump_pytuple(m_pTuple, argDict)
            except Exception as e:
                print(f"  Tuple: (error: {e})")
            

        except Exception as e:
            print(f"Error: {e}")

class DumpPythonFuncRec(gdb.Command):
    """Dump a PythonFuncRec (internal class from PythonIntf.cc)."""
    
    def __init__(self):
        super(DumpPythonFuncRec, self).__init__("dump-python-func-rec", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            # Auto-cast: if val is a shared_ptr<ExternalFunctionRec>, extract
            # _M_ptr and cast to PythonFuncRec* (the derived type).
            val_type = val.type.strip_typedefs()
            type_name = str(val_type)
            if "shared_ptr" in type_name and "ExternalFunctionRec" in type_name:
                ptr = val["_M_ptr"]
                if ptr == 0:
                    print("PythonFuncRec: nullptr")
                    return
                python_func_type = gdb.lookup_type("PythonFuncRec").pointer()
                val = ptr.cast(python_func_type).dereference()
            
            print(f"PythonFuncRec @ {val.address}")
            
            # Access members
            try:
                m_name = std_string_to_str(val["m_name"])
                print(f"  Name: {m_name}")
            except Exception as e:
                print(f"  Name: (error: {e})")
            
            try:
                m_pFuncPtr = val["m_pFuncPtr"]
                print(f"  Func Ptr: {m_pFuncPtr}")
            except Exception as e:
                print(f"  Func Ptr: (error: {e})")
            
            # Show m_doc
            try:
                m_doc = std_string_to_str(val["m_doc"])
                if m_doc:
                    # Format multi-line docs nicely
                    if "\n" in m_doc:
                        print(f"  doc:")
                        for line in m_doc.split("\n"):
                            print(f"    {line}")
                    else:
                        print(f"  doc: {m_doc}")
                else:
                    print(f"  doc: (empty)")
            except Exception as e:
                print(f"  doc: (error: {e})")
            
            # Show m_argTypeExact
            try:
                m_argTypeExact = bool(val["m_argTypeExact"])
                print("  Arg Type: {}".format("exact" if m_argTypeExact else "no exactness information"))
            except Exception as e:
                print(f"  Arg Type: (error: {e})")
            
            # Expand m_args vector
            argDict = dict()
            try:
                m_args = val["m_args"]
                arg_size = std_vector_size(m_args)
                print(f"  Args ({arg_size} items):")
                
                # Try to iterate and dump each argument
                args_start = m_args["_M_impl"]["_M_start"]
                for i in range(arg_size):
                    try:
                        arg_elem = args_start[i]
                        arg_name = std_string_to_str(arg_elem["m_name"])
                        arg_default = int(arg_elem["m_default"])
                        arg_default_str = ALU_COUNTER_TYPE.get(arg_default, f"Unknown({arg_default})")
                        
                        print(f"    [{i}] {arg_name} (default: {arg_default_str})")
                        argDict[i] = arg_name
                        
                        # Show default value if present
                        if arg_default == 1:  # counterType__Str
                            try:
                                defStr = std_string_to_str(arg_elem["m_defStr"])
                                print(f"         = \"{defStr}\"")
                            except:
                                pass
                        elif arg_default == 2:  # counterType__Int
                            try:
                                defInt = int(arg_elem["m_defInt"])
                                print(f"         = {defInt}")
                            except:
                                pass
                        elif arg_default == 3:  # counterType__Float
                            try:
                                defFloat = float(arg_elem["m_defFloat"])
                                print(f"         = {defFloat}")
                            except:
                                pass
                    except Exception as arg_e:
                        print(f"    [{i}] (error: {arg_e})")
            except Exception as e:
                print(f"  Args: (error: {e})")
            
            # Show m_pTuple (argument values for the current Python function call)
            try:
                m_pTuple = val["m_pTuple"]
                dump_pytuple(m_pTuple, argDict)
            except Exception as e:
                print(f"  Tuple: (error: {e})")

        except Exception as e:
            print(f"Error: {e}")

class DumpPythonFuncArg(gdb.Command):
    """Dump a PythonFuncArg (internal class from PythonIntf.cc)."""
    
    def __init__(self):
        super(DumpPythonFuncArg, self).__init__("dump-python-func-arg", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        try:
            val = gdb.parse_and_eval(arg)
            
            # Access members
            try:
                m_name = std_string_to_str(val["m_name"])
                m_default = int(val["m_default"])
                m_default_str = ALU_COUNTER_TYPE.get(m_default, f"Unknown({m_default})")
                
                print(f"PythonFuncArg @ {val.address}")
                print(f"  Name: {m_name}")
                print(f"  Default Type: {m_default_str}")
                
                # Try to get default value
                if m_default == 1:  # counterType__Str
                    try:
                        m_defStr = std_string_to_str(val["m_defStr"])
                        print(f"  Default String: \"{m_defStr}\"")
                    except:
                        pass
                elif m_default == 2:  # counterType__Int
                    try:
                        m_defInt = int(val["m_defInt"])
                        print(f"  Default Int: {m_defInt}")
                    except:
                        pass
                elif m_default == 3:  # counterType__Float
                    try:
                        m_defFloat = float(val["m_defFloat"])
                        print(f"  Default Float: {m_defFloat}")
                    except:
                        pass
            except Exception as inner_e:
                print(f"PythonFuncArg @ {val.address}")
                print(f"  (Error reading members: {inner_e})")
        except Exception as e:
            print(f"Error: {e}")

# ============================================================================
# CONVENIENCE COMMAND
# ============================================================================

class DumpAll(gdb.Command):
    """Dump all relevant debugging info (ProcessingState + StringBuilder + current Item)."""
    
    def __init__(self):
        super(DumpAll, self).__init__("dump-all", gdb.COMMAND_DATA)
    
    def invoke(self, arg, from_tty):
        print("=== Full Debug Dump ===")
        print("\nNote: Provide arguments like: dump-all pState sb item")
        print("This is a convenience command for dumping multiple objects at once.")

# ============================================================================
# REGISTRATION
# ============================================================================

def register_commands():
    """Register all dump commands."""
    # InputPart commands
    DumpInputPart()
    DumpLiteralPart()
    DumpRangePart()
    DumpWordRangePart()
    DumpFieldRangePart()
    DumpClockPart()
    DumpIDPart()
    DumpExpressionPart()
    
    # Item commands
    DumpItem()
    DumpDataField()
    DumpTokenItem()
    DumpSetItem()
    DumpSkipItem()
    DumpConditionItem()
    DumpBreakItem()
    DumpContextItem()
    DumpSelectItem()
    DumpSplitItem()
    
    # itemGroup command
    DumpItemGroup()
    
    # Token commands
    DumpToken()
    DumpTokenRange()
    
    # Processing commands
    DumpProcessingState()
    DumpStringBuilder()
    DumpReader()
    DumpWriter()
    
    # ALU commands
    DumpALUValue()
    DumpALUCounters()
    DumpAluUnit()
    DumpAluVec()
    DumpAluValueStats()
    DumpFrequencyMap()
    
    # Python interface commands
    DumpAluFunction()
    DumpExternalFunctionRec()
    DumpExternalFunctionCollection()
    DumpPythonFunctionCollection()
    DumpPythonFuncByName()
    DumpPythonFuncRec()
    DumpPythonFuncArg()
    
    # Utility commands
    DumpException()
    DumpAll()

# Register pretty-printers (only if an object file is loaded)
try:
    objfile = gdb.objfile.current_objfile()
    if objfile:
        gdb.printing.register_pretty_printer(objfile, build_pretty_printer())
except:
    # No object file loaded yet; pretty-printers will be registered when one is loaded
    pass

# Register all commands
register_commands()

print("specs GDB extension loaded successfully")
