/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "QP/C++", "index.html", [
    [ "About QP/C++™", "index.html", [
      [ "What's new?", "index.html#ab_new", null ],
      [ "What is it?", "index.html#ab_about", null ],
      [ "What does it do?", "index.html#ab_goals", null ],
      [ "What's special about it?", "index.html#ab_special", [
        [ "Object Orientation", "index.html#oop", null ],
        [ "Lightweight", "index.html#lightweight", null ],
        [ "Hierarchical State Machines", "index.html#hsms", null ],
        [ "Built-in Kernels", "index.html#kernels", null ],
        [ "Interoperability", "index.html#inter", null ],
        [ "Popularity & Maturity", "index.html#popular", null ],
        [ "Book", "index.html#psicc2", null ]
      ] ],
      [ "How is it licensed?", "index.html#licensing", [
        [ "Open Source Projects", "index.html#open-source", null ],
        [ "Closed Source Projects", "index.html#closed-source", null ]
      ] ],
      [ "How to get help?", "index.html#support", null ],
      [ "Contact Information", "index.html#contact", null ]
    ] ],
    [ "Getting Started", "gs.html", [
      [ "Downloading & Installing QP/C++™", "gs.html#gs_obtain", null ],
      [ "Getting Started with QP/C++ App Note", "gs.html#gs_an", null ]
    ] ],
    [ "Structure and Features", "struct.html", [
      [ "Directories and Files", "struct.html#files", null ],
      [ "Components of QP/C++", "struct.html#comp", [
        [ "QEP Hierarchical Event Processor", "struct.html#comp_qep", null ],
        [ "QF Active-Object Framework", "struct.html#comp_qf", null ],
        [ "QV Cooperative Kernel", "struct.html#comp_qv", null ],
        [ "QK Preemptive Non-Blocking Kernel", "struct.html#comp_qk", null ],
        [ "QXK Preemptive Blocking Kernel", "struct.html#comp_qxk", null ],
        [ "QS Software Tracing System", "struct.html#comp_qs", null ]
      ] ],
      [ "Object-Orientation", "struct.html#qpcpp_oo", null ],
      [ "Classes in QP/C++", "struct.html#classes", null ],
      [ "Hierarchical State Machines", "struct.html#hsm", null ],
      [ "Design by Contract", "struct.html#DbC", null ],
      [ "Coding Standard", "struct.html#coding", null ],
      [ "MISRA Compliance", "struct.html#misra", null ],
      [ "PC-Lint Support", "struct.html#pc-lint", null ]
    ] ],
    [ "Coding State Machines in QP/C++", "sm.html", [
      [ "State Machine Declaration", "sm.html#sm_decl", [
        [ "The Q_STATE_DECL() Macro", "sm.html#sm_state_decl", null ]
      ] ],
      [ "State Machine Definition", "sm.html#sm_def", [
        [ "The Q_STATE_DEF() Macro", "sm.html#sm_state_def", null ],
        [ "Top-Most Initial Pseudostate", "sm.html#sm_def_init", null ],
        [ "State-Handler Member Functions", "sm.html#sm_def_state", null ]
      ] ],
      [ "State-Handler Call Overhead", "sm.html#sm_call", null ]
    ] ],
    [ "API Reference", "api.html", [
      [ "QEP (Hierarchical State Machines)", "api.html#api_qep", [
        [ "Hierarchical State Machines", "api.html#api_qep_hsm", null ]
      ] ],
      [ "QF (Active Object Framework)", "api.html#api_qf", [
        [ "Active Objects", "api.html#api_qf_act", null ],
        [ "Publish-Subscribe", "api.html#api_qf_ps", null ],
        [ "Dynamic Events", "api.html#api_qf_evt", null ],
        [ "Time Events", "api.html#api_qf_time", null ],
        [ "Event Queues (raw thread-safe)", "api.html#api_qf_queue", null ],
        [ "Memory Pools", "api.html#api_qf_mem", null ]
      ] ],
      [ "QS (\"Quantum Spy\" Software Tracing)", "api.html#api_qs", [
        [ "QS Initialization and Control", "api.html#api_qs_ini", null ],
        [ "QS Receive-Channel (QS-RX)", "api.html#api_qs_rx", null ],
        [ "QS Filters", "api.html#api_qs_filter", null ],
        [ "QS Dictionaries", "api.html#api_qs_dict", null ],
        [ "QS Application-Specific Records", "api.html#api_qs_user", null ]
      ] ],
      [ "QV (Cooperative Kernel)", "api.html#api_qv", [
        [ "Kernel Initialization and Control", "api.html#api_qv_init", null ]
      ] ],
      [ "QK (Preemptive Run-to-Completion Kernel)", "api.html#api_qk", [
        [ "Kernel Initialization and Control", "api.html#api_qk_ctrl", null ],
        [ "Interrupt Management", "api.html#api_qk_isr", null ]
      ] ],
      [ "QXK (Preemptive Dual-Mode Run-to-Completion/Blocking RTOS Kernel)", "api.html#api_qxk", [
        [ "Kernel Initialization and Control", "api.html#api_qxk_ctrl", null ],
        [ "Interrupt Management", "api.html#api_qxk_isr", null ],
        [ "Extended Thread Management", "api.html#api_qxk_xthr", null ],
        [ "Semaphores", "api.html#api_qxk_sema", null ],
        [ "Mutexes", "api.html#api_qxk_mutex", null ],
        [ "Message Queues", "api.html#api_qxk_queue", null ],
        [ "Memory Pools", "api.html#api_qxk_mem", null ],
        [ "Thread-Local Storage", "api.html#api_qxk_tls", null ]
      ] ]
    ] ],
    [ "Examples", "exa.html", "exa" ],
    [ "Ports", "ports.html", "ports" ],
    [ "Revision History", "history.html", [
      [ "Version 6.9.3, 2021-04-12", "history.html#qpcpp_6_9_3", null ],
      [ "Version 6.9.2, 2021-01-18", "history.html#qpcpp_6_9_2", null ],
      [ "Version 6.9.1, 2020-09-28", "history.html#qpcpp_6_9_1", null ],
      [ "Version 6.9.0, 2020-08-21", "history.html#qpcpp_6_9_0", null ],
      [ "Version 6.8.2, 2020-07-17", "history.html#qpcpp_6_8_2", null ],
      [ "Version 6.8.1, 2020-04-04", "history.html#qpcpp_6_8_1", null ],
      [ "Version 6.8.0, 2020-03-21", "history.html#qpcpp_6_8_0", null ],
      [ "Version 6.7.0, 2019-12-30", "history.html#qpcpp_6_7_0", null ],
      [ "Version 6.6.0, 2019-10-31", "history.html#qpcpp_6_6_0", null ],
      [ "Version 6.5.1, 2019-05-24", "history.html#qpcpp_6_5_1", null ],
      [ "Version 6.5.0, 2019-03-30", "history.html#qpcpp_6_5_0", null ],
      [ "Version 6.4.0, 2019-02-10", "history.html#qpcpp_6_4_0", null ],
      [ "Version 6.3.8, 2018-12-31", "history.html#qpcpp_6_3_8", null ],
      [ "Version 6.3.7, 2018-11-20", "history.html#qpcpp_6_3_7", null ],
      [ "Version 6.3.6, 2018-10-20", "history.html#qpcpp_6_3_6", null ],
      [ "Version 6.3.4, 2018-08-10", "history.html#qpcpp_6_3_4", null ],
      [ "Version 6.3.3a, 2018-07-16", "history.html#qpcpp_6_3_3a", null ],
      [ "Version 6.3.3, 2018-06-22", "history.html#qpcpp_6_3_3", null ],
      [ "Version 6.3.2, 2018-06-20", "history.html#qpcpp_6_3_2", null ],
      [ "Version 6.3.1, 2018-05-24", "history.html#qpcpp_6_3_1", null ],
      [ "Version 6.3.0, 2018-05-10", "history.html#qpcpp_6_3_0", null ],
      [ "Version 6.2.0, 2018-03-16", "history.html#qpcpp_6_2_2", null ],
      [ "Version 6.1.1, 2018-02-18", "history.html#qpcpp_6_1_1", null ],
      [ "Version 6.1.0, 2018-02-04", "history.html#qpcpp_6_1_0", null ],
      [ "Version 6.0.4, 2018-01-10", "history.html#qpcpp_6_0_4", null ],
      [ "Version 6.0.3, 2017-12-12", "history.html#qpcpp_6_0_3", null ],
      [ "Version 6.0.1, 2017-12-10", "history.html#qpcpp_6_0_1", null ],
      [ "Version 6.0.0, 2017-10-13", "history.html#qpcpp_6_0_0", null ],
      [ "Version 5.9.9, 2017-09-29", "history.html#qpcpp_5_9_9", null ],
      [ "Version 5.9.8, 2017-09-15", "history.html#qpcpp_5_9_8", null ],
      [ "Version 5.9.7, 2017-08-18", "history.html#qpcpp_5_9_7", null ],
      [ "Version 5.9.6, 2017-08-04", "history.html#qpcpp_5_9_6", null ],
      [ "Version 5.9.5, 2017-07-20", "history.html#qpcpp_5_9_5", null ],
      [ "Version 5.9.4, 2017-07-07", "history.html#qpcpp_5_9_4", null ],
      [ "Version 5.9.3, 2017-06-19", "history.html#qpcpp_5_9_3", null ],
      [ "Version 5.9.2, 2017-06-05", "history.html#qpcpp_5_9_2", null ],
      [ "Version 5.9.1, 2017-05-26", "history.html#qpcpp_5_9_1", null ],
      [ "Version 5.9.0, 2017-05-19", "history.html#qpcpp_5_9_0", null ],
      [ "Version 5.8.2, 2017-02-08", "history.html#qpcpp_5_8_2", null ],
      [ "Version 5.8.1, 2016-12-16", "history.html#qpcpp_5_8_1", null ],
      [ "Version 5.8.0, 2016-11-30", "history.html#qpcpp_5_8_0", null ],
      [ "Version 5.7.4, 2016-11-04", "history.html#qpcpp_5_7_4", null ],
      [ "Version 5.7.3, 2016-10-07", "history.html#qpcpp_5_7_3", null ],
      [ "Version 5.7.2, 2016-09-30", "history.html#qpcpp_5_7_2", null ],
      [ "Version 5.7.0, 2016-08-31", "history.html#qpcpp_5_7_0", null ],
      [ "Version 5.6.5, 2016-06-06", "history.html#qpcpp_5_6_5", null ],
      [ "Version 5.6.4, 2016-04-25", "history.html#qpcpp_5_6_4", null ],
      [ "Version 5.6.3, 2016-04-12", "history.html#qpcpp_5_6_3", null ],
      [ "Version 5.6.2, 2016-03-31", "history.html#qpcpp_5_6_2", null ],
      [ "Version 5.6.1, 2016-01-01", "history.html#qpcpp_5_6_1", null ],
      [ "Version 5.5.1, 2015-10-05", "history.html#qpcpp_5_5_1", null ],
      [ "Version 5.5.0, 2015-09-28", "history.html#qpcpp_5_5_0", null ],
      [ "Version 5.4.2, 2015-06-06", "history.html#qpcpp_5_4_2", null ],
      [ "Version 5.4.1, 2015-05-21", "history.html#qpcpp_5_4_1", null ],
      [ "Version 5.4.0, 2015-05-14", "history.html#qpcpp_5_4_0", null ],
      [ "Version 5.3.1, 2014-09-20", "history.html#qpcpp_5_3_1", null ],
      [ "Version 5.3.0, 2014-04-14", "history.html#qpcpp_5_3_0", null ],
      [ "Version 5.2.1, 2014-01-06", "history.html#qpcpp_5_2_1", null ],
      [ "Version 5.2.0, 2013-12-28", "history.html#qpcpp_5_2_0", null ],
      [ "Version 5.1.1, 2013-10-10", "history.html#qpcpp_5_1_1", null ],
      [ "Version 5.1.0, 2013-10-02", "history.html#qpcpp_5_1_0", null ],
      [ "Version 4.5.04, 2013-02-10", "history.html#qpcpp_4_5_04", null ],
      [ "Version 4.5.03, 2012-11-29", "history.html#qpcpp_4_5_03", null ],
      [ "Version 4.5.02, 2012-08-15", "history.html#qpcpp_4_5_02", null ],
      [ "Version 4.5.01, 2012-06-14", "history.html#qpcpp_4_5_01", null ],
      [ "Version 4.5.00, 2012-05-26", "history.html#qpcpp_4_5_00", null ],
      [ "Version 4.4.00, 2012-05-02", "history.html#qpcpp_4_4_00", null ],
      [ "Version 4.3.00, 2011-11-03", "history.html#qpcpp_4_3_00", null ],
      [ "Version 4.2.04, 2011-09-24", "history.html#qpcpp_4_2_04", null ],
      [ "Version 4.2.02, 2011-09-08", "history.html#qpcpp_4_2_02", null ],
      [ "Version 4.2.01, 2011-08-13", "history.html#qpcpp_4_2_01", null ],
      [ "Version 4.2.00, 2011-07-15", "history.html#qpcpp_4_2_00", null ],
      [ "Version 4.1.07, 2011-02-28", "history.html#qpcpp_4_1_07", null ],
      [ "Version 4.1.06, 2011-01-07", "history.html#qpcpp_4_1_06", null ],
      [ "Version 4.1.05, 2010-11-01", "history.html#qpcpp_4_1_05", null ],
      [ "Version 4.1.04, 2010-05-16", "history.html#qpcpp_4_1_04", null ],
      [ "Version 4.1.03, 2010-01-21", "history.html#qpcpp_4_1_03", null ],
      [ "Version 4.1.02, 2010-01-14", "history.html#qpcpp_4_1_02", null ],
      [ "Version 4.1.01, 2009-11-05", "history.html#qpcpp_4_1_01", null ],
      [ "Version 4.1.00, 2009-10-09", "history.html#qpcpp_4_1_00", null ],
      [ "Version 4.0.04 (Product) Apr 09, 2009", "history.html#qpcpp_4_0_04", null ],
      [ "Version 4.0.03, 2008-12-27", "history.html#qpcpp_4_0_03", null ],
      [ "Version 4.0.02, 2008-11-15", "history.html#qpcpp_4_0_02", null ],
      [ "Version 4.0.01, 2008-06-09", "history.html#qpcpp_4_0_01", null ],
      [ "Version 4.0.00, 2008-04-07", "history.html#qpcpp_4_0_00", null ]
    ] ],
    [ "Code Metrics", "metrics.html", null ],
    [ "Using Online Help", "help.html", [
      [ "Reading Online Help Sequentially", "help.html#help_seq", null ],
      [ "Quickly Locating a Topic of Interest", "help.html#help_random", null ],
      [ "Using the Tree View", "help.html#help_tree_view", null ]
    ] ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", "namespacemembers_dup" ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"16bit_2cpu_8lnt.html",
"class_q_p_1_1_q_hsm.html#a1bd057458f70978840c6f18b891ea07f",
"class_q_p_1_1_q_x_mutex.html#ab7e2800c013f23f154ebea7b9ff8a0f2",
"history.html#qpcpp_6_0_1",
"namespace_q_p.html#aae826e32e83161f065979b1519417ba2a10a83ac14ca016e2596bdc4d06299772",
"qk_8hpp.html#ab912e96f95e2fea16da71a582fd25141",
"qs_8hpp.html#ad3a766c33b9c3111cdf3c2a09ce17ea0a73581bdfde2881461493ce9dfc072073",
"qxk__pkg_8hpp_source.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';