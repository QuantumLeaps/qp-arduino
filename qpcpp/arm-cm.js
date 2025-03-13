var arm_cm =
[
    [ "Directories and Files", "arm-cm.html#arm-cm_files", null ],
    [ "Interrupts in the QP Ports to ARM Cortex-M", "arm-cm.html#arm-cm_int", [
      [ "\"Kernel-Aware\" and \"Kernel-Unaware\" Interrupts", "arm-cm.html#arm-cm_kernel-aware", null ],
      [ "Assigning Interrupt Priorities", "arm-cm.html#arm-cm_int-assign", null ],
      [ "Interrupts and the FPU (Cortex-M4F/M7)", "arm-cm.html#arm-cm_int-fpu", null ]
    ] ],
    [ "References", "arm-cm.html#arm-cm_ref", null ],
    [ "Cooperative QV Kernel", "arm-cm_qv.html", [
      [ "Synopsis of the QV Port on ARM Cortex-M", "arm-cm_qv.html#arm-cm_qv-synopsis", null ],
      [ "The qep_port.h Header File", "arm-cm_qv.html#arm-cm_qv-qep_port", null ],
      [ "The qf_port.h Header File", "arm-cm_qv.html#arm-cm_qv-qf_port", null ],
      [ "The qv_port.h Header File", "arm-cm_qv.html#arm-cm_qv_port_h", null ],
      [ "The qv_port.c Implementation File", "arm-cm_qv.html#arm-cm_qv_port_c", null ],
      [ "Writing ISRs for QV", "arm-cm_qv.html#arm-cm_qv-isr", null ],
      [ "Using the FPU in the QV Port (Cortex-M4F/M7)", "arm-cm_qv.html#arm-cm_qv-fpu", [
        [ "FPU NOT used in the ISRs", "arm-cm_qv.html#arm-cm_qv-fpu_noisr", null ],
        [ "FPU used in the ISRs", "arm-cm_qv.html#arm-cm_qv-fpu_isr", null ]
      ] ],
      [ "QV Idle Processing Customization in QV_onIdle()", "arm-cm_qv.html#arm-cm_qv-idle", null ]
    ] ],
    [ "Preemptive Non-Blocking QK Kernel", "arm-cm_qk.html", [
      [ "Synopsis of the QK Port on ARM Cortex-M", "arm-cm_qk.html#arm-cm_qk-synopsis", [
        [ "Preemption Scenarios in QK on ARM Cortex-M", "arm-cm_qk.html#arm-cm_qk-preempt", null ]
      ] ],
      [ "The qf_port.h Header File", "arm-cm_qk.html#arm-cm_qk-qf_port", null ],
      [ "QK Port Implementation for ARM Cortex-M", "arm-cm_qk.html#arm-cm_qk-qk_impl", [
        [ "QK_init() Implementation", "arm-cm_qk.html#arm-cm_qk_port-asm_init", null ],
        [ "PendSV_Handler() Implementation", "arm-cm_qk.html#arm-cm_qk_port-asm_pendsv", null ],
        [ "NMI_Handler() Implementation", "arm-cm_qk.html#arm-cm_qk_port-asm_nmi", null ]
      ] ],
      [ "Writing ISRs for QK", "arm-cm_qk.html#arm-cm_qk-isr", null ],
      [ "Using the FPU in the QK Port (Cortex-M4F/M7)", "arm-cm_qk.html#arm-cm_qk-fpu", [
        [ "FPU used in ONE thread only and not in any ISR", "arm-cm_qk.html#arm-cm_qk-fpu_1thread", null ],
        [ "FPU used in more than one thread only or the ISR", "arm-cm_qk.html#arm-cm_qk-fpu_nthreadd", null ]
      ] ],
      [ "QK Idle Processing Customization in QK_onIdle()", "arm-cm_qk.html#arm-cm_qk-idle", null ],
      [ "Testing QK Preemption Scenarios", "arm-cm_qk.html#arm-cm_qk-testing", [
        [ "Interrupt Nesting Test", "arm-cm_qk.html#arm-cm_qk-test-isr", null ],
        [ "Thread Preemption Test", "arm-cm_qk.html#arm-cm_qk-test-thread", null ],
        [ "Testing the FPU (Cortex-M4F/M7)", "arm-cm_qk.html#arm-cm_qk-test-fpu", null ],
        [ "Other Tests", "arm-cm_qk.html#arm-cm_qk-test-other", null ]
      ] ]
    ] ],
    [ "Preemptive \"Dual-Mode\" QXK Kernel", "arm-cm_qxk.html", [
      [ "Synopsis of the QXK Port on ARM Cortex-M", "arm-cm_qxk.html#arm-cm_qxk-synopsis", [
        [ "Using the VFP", "arm-cm_qxk.html#arm-com_qxk_vfp", null ]
      ] ]
    ] ]
];