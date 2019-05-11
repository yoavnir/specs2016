# specs2016 Documentation
The *specs2016* project is a re-writing of the *specs* pipeline stage from CMS, only changed quite a bit.

*specs* is a command line utility for parsing and re-arranging text input. It allows re-alignment of fields, some format conversion, plus quite a bit of computation and re-formatting multiple lines into single lines or vice versa. Input comes from standard input or files, and output flows to standard output or files.

This version is liberally based on the [**CMS Pipelines User's Guide and Reference**](https://publib.boulder.ibm.com/epubs/pdf/hcsj0c30.pdf), especially chapters 16, 24, and 20.

Basic Functionality
===================
* [Basic Specifications](basicspec.md)
  * [Spec Units](basicspec.md#spec-units)
  * [Data Fields](basicspec.md#data-fields)
  * [Alignment](basicspec.md#alignment)
  * [Conversions](basicspec.md#conversions)
  * [Words vs Fields](basicspec.md#words-vs-fields)
  * [Examples](basicspec.md#examples)
  * [Other Common Spec Units](basicspec.md#other-common-spec-units)
* [Arithmetic-Logical Unit (ALU)](alu.md)
  * [Elements of the ALU](alu.md#elements-of-the-alu)
    * [String and Numerical Literals](alu.md#string-and-numerical-literals)
    * [Field Identifiers](alu.md#field-identifiers)
    * [Counters](alu.md#counters)
    * [Configured Literals](alu.md#configured-literals)
    * [Operators](alu.md#operators)
    * [Functions](alu.md#functions)
  * [SET](alu.md#set)
    * [Assignments as Expressions](alu.md#assignments-as-expressions)
* [Structured Specification](struct.md)
  * [Condition](struct.md#conditions)
  * [Conditional Execution](struct.md#conditional-execution)
  * [Loops](struct.md#loops)
  * [Run-In and Run-Out](struct.md#run-in-and-run-out)
  * [Control Breaks](struct.md#control-breaks)
* [Input and Output Streams and Records](streams.md)
  * [>1 Output Record in Each Iteration](streams.md#1-output-record-in-each-iteration)
  * [>1 Input Record in Each Iteration](streams.md#1-input-record-in-each-iteration)
  * [Pushing Back The Last Record](streams.md#pushing-back-the-last-record)
  * [Process a Record in Two Phases](streams.md#process-a-record-in-two-phases)

Advanced Topics
===============
* [Command-Line Switches](cliswitch.md)
* [Advanced ALU](alu_adv.md)
  * [Table of Operands](alu_adv.md#table-of-operands)
  * [Table of Assignment Operators](alu_adv.md#table-of-assignment-operators)
  * [Table of Numerical Functions](alu_adv.md#table-of-numerical-functions)
  * [Table of String Functions](alu_adv.md#table-of-string-functions)
  * [Table of Record Access Functions](alu_adv.md#table-of-record-access-functions)
  * [Table of Special Functions](alu_adv.md#table-of-special-functions)
* [Examples](examples.md)
  * [Estimation of PI](examples.md#pi-estimate)


The old, one-page draft of documentation can be read [here](onepage.md).